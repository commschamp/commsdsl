#include "OptCondImpl.h"

#include <cassert>
#include <algorithm>

#include "BundleFieldImpl.h"
#include "BitfieldFieldImpl.h"
#include "SetFieldImpl.h"

//#include <iostream>

namespace bbmp
{

namespace
{

const char Esc = '\\';
const char Deref = '$';

} // namespace

bool OptCondExprImpl::parse(const std::string& expr, ::xmlNodePtr node, Logger& logger)
{
    if (expr.empty()) {
        logError(logger) << XmlWrap::logPrefix(node) <<
            "Invalid \"" << common::condStr() << "\" expression";
        return false;
    }

    assert(!hasUpdatedValue());
    return
        checkComparison(expr, "!=", node, logger) &&
        checkComparison(expr, ">=", node, logger) &&
        checkComparison(expr, "<=", node, logger) &&
        checkComparison(expr, "=", node, logger) &&
        checkComparison(expr, ">", node, logger) &&
        checkComparison(expr, "<", node, logger) &&
        checkBool(expr, node, logger) &&
        hasUpdatedValue();
}

OptCondImpl::Kind OptCondExprImpl::kindImpl() const
{
    return Kind::Expr;
}

OptCondImpl::Ptr OptCondExprImpl::cloneImpl() const
{
    return Ptr(new OptCondExprImpl(*this));
}

bool OptCondExprImpl::verifyImpl(const OptCondImpl::FieldsList& fields, ::xmlNodePtr node, Logger& logger) const
{
    if (m_left.empty()) {
        return verifyBitCheck(fields, node, logger);
    }

    return verifyComparison(fields, node, logger);
}

bool OptCondExprImpl::hasUpdatedValue()
{
    return (!m_left.empty()) ||
           (!m_right.empty()) ||
           (!m_op.empty());
}

bool OptCondExprImpl::checkComparison(const std::string& expr, const std::string& op, ::xmlNodePtr node, Logger& logger)
{
    if (hasUpdatedValue()) {
        return true;
    }

    auto reportInvalidExrFunc =
        [node, &logger]()
        {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "Invalid \"" << common::condStr() << "\" expression";
        };

    std::size_t opPos = 0U;
    while (true) {
        opPos = expr.find(op, opPos);
        if (opPos == 0U) {
            reportInvalidExrFunc();
            return false;
        }

        if (opPos == std::string::npos) {
            return true;
        }

        if (expr[opPos - 1] != Esc) {
            break;
        }

        ++opPos;
    }

    auto leftEndPos = expr.find_last_not_of(' ', opPos - 1);
    if (leftEndPos == std::string::npos) {
        reportInvalidExrFunc();
        return false;
    }

    auto rightBegPos = expr.find_first_not_of(' ', opPos + op.size());
    if (rightBegPos == std::string::npos) {
        reportInvalidExrFunc();
        return false;
    }

    m_left.assign(expr.begin(), expr.begin() + leftEndPos + 1U);
    m_op = op;
    m_right.assign(expr.begin() + rightBegPos, expr.end());

    if (m_left.empty() || m_right.empty()) {
        reportInvalidExrFunc();
        return false;
    }

    if (m_left[0] != Deref) {
        logError(logger) << XmlWrap::logPrefix(node) <<
            "Invalid \"" << common::condStr() << "\" expression, left side of "
            "comparison operator must dereference other field.";
        return false;
    }

    return true;
}

bool OptCondExprImpl::checkBool(const std::string& expr, ::xmlNodePtr node, Logger& logger)
{
    if (hasUpdatedValue()) {
        return true;
    }

    auto reportInvalidExrFunc =
        [node, &logger]()
        {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "Invalid \"" << common::condStr() << "\" expression";
        };

    assert(!expr.empty());
    if (expr[0] == Deref) {
        m_right = expr;
        return true;
    }

    if (expr[0] != '!') {
        reportInvalidExrFunc();
        return false;
    }

    auto valPos = expr.find_first_not_of(' ', 1);
    if (valPos == std::string::npos) {
        reportInvalidExrFunc();
        return false;
    }

    if (expr[valPos] != Deref) {
        logError(logger) << XmlWrap::logPrefix(node) <<
            "Invalid \"" << common::condStr() << "\" expression, "
            "the check must dereference other field.";
        return false;
    }

    m_op = "!";
    m_right.assign(expr.begin() + valPos, expr.end());
    return true;
}

FieldImpl* OptCondExprImpl::findField(
    const FieldsList& fields,
    const std::string& name,
    std::size_t& remPos)
{
    auto dotPos = name.find_first_of('.', remPos);
    std::string fieldName(name, remPos, dotPos - remPos);
    if (fieldName.empty()) {
        return nullptr;
    }

    auto iter =
        std::find_if(
            fields.begin(), fields.end(),
            [&fieldName](auto& f)
            {
                return f->name() == fieldName;
            });

    if (iter == fields.end()) {
        return nullptr;
    }

    if (dotPos == std::string::npos) {
        remPos = dotPos;
        return iter->get();
    }

    remPos = dotPos + 1;
    auto redirectFunc =
        [&name, &remPos](const auto& f)
        {
            auto& members = f.members();
            return findField(members, name, remPos);
        };

    auto fieldKind = (*iter)->kind();
    if (fieldKind == FieldImpl::Kind::Bundle) {
        return redirectFunc(static_cast<const BundleFieldImpl&>(**iter));
    }

    if (fieldKind == FieldImpl::Kind::Bitfield) {
        return redirectFunc(static_cast<const BundleFieldImpl&>(**iter));
    }

    return iter->get();
}

bool OptCondExprImpl::verifyBitCheck(const OptCondImpl::FieldsList& fields, ::xmlNodePtr node, Logger& logger) const
{
    assert(!m_right.empty());
    assert(m_right[0] == Deref);

    std::size_t remPos = 1;
    auto field = findField(fields, m_right, remPos);
    if ((field == nullptr) ||
        (field->kind() != FieldImpl::Kind::Set)) {
        logError(logger) << XmlWrap::logPrefix(node) <<
            "The \"" << m_right << "\" string is expected to dereference existing bit in existing \"" <<
            common::setStr() << "\" field";
        return false;
    }


    auto setField = static_cast<const SetFieldImpl*>(field);
    std::string bitName(m_right, remPos);
    auto iter = setField->bits().find(bitName);
    if (iter == setField->bits().end()) {
        logError(logger) << XmlWrap::logPrefix(node) <<
            "The \"" << m_right << "\" string is expected to dereference existing bit in existing \"" <<
            common::setStr() << "\" field";
        return false;
    }
    return true;
}

bool OptCondExprImpl::verifyComparison(const OptCondImpl::FieldsList& fields, ::xmlNodePtr node, Logger& logger) const
{
    assert(!m_left.empty());
    assert(!m_right.empty());
    assert(m_left[0] == Deref);

    std::size_t remPos = 1;
    auto field = findField(fields, m_left, remPos);
    if (field == nullptr) {
        logError(logger) << XmlWrap::logPrefix(node) <<
            "The \"" << m_left << "\" string is expected to dereference existing field in the containing \"" <<
            common::bundleStr() << "\" or \"" << common::messageStr() << "\"";
        return false;
    }

    if (m_right[0] == Deref) {
        std::size_t rightRemPos = 1U;
        auto rightField = findField(fields, m_right, rightRemPos);
        if (rightField == nullptr) {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "The \"" << m_right << "\" string is expected to dereference existing field in the containing \"" <<
                common::bundleStr() << "\" or \"" << common::messageStr() << "\"";
            return false;
        }

        if (!field->isComparableToField(*rightField)) {
            logError(logger) << XmlWrap::logPrefix(node) <<
                "Two dereferenced fields \"" << m_left << "\" and \"" << m_right << "\" cannot be compared.";
            return false;
        }

        return true;
    }

    if (!field->isComparableToValue(m_right)) {
        logError(logger) << XmlWrap::logPrefix(node) <<
            "The dereferenced fields \"" << m_left << "\" cannot be compared to value \"" << m_right << "\".";
        return false;
    }
    return true;
}

OptCondListImpl::OptCondListImpl(const OptCondListImpl& other)
  : Base(other),
    m_type(other.m_type)
{
    m_conds.reserve(other.m_conds.size());
    for (auto& c : other.m_conds) {
        m_conds.push_back(c->clone());
    }
}

OptCondImpl::Kind OptCondListImpl::kindImpl() const
{
    return Kind::List;
}

OptCondImpl::Ptr OptCondListImpl::cloneImpl() const
{
    return Ptr(new OptCondListImpl(*this));
}

bool OptCondListImpl::verifyImpl(const OptCondImpl::FieldsList& fields, ::xmlNodePtr node, Logger& logger) const
{
    return std::all_of(
        m_conds.begin(), m_conds.end(),
        [&fields, node, &logger](auto& c)
        {
            return c->verify(fields, node, logger);
        });
}



} // namespace bbmp
