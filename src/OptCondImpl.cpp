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

bool OptCondExprImpl::parse(const std::string& expr)
{
    if (expr.empty()) {
        return false;
    }

    assert(!hasUpdatedValue());
    return
        checkComparison(expr, "!=") &&
        checkComparison(expr, ">=") &&
        checkComparison(expr, "<=") &&
        checkComparison(expr, "=") &&
        checkComparison(expr, ">") &&
        checkComparison(expr, "<") &&
        checkBool(expr) &&
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

bool OptCondExprImpl::verifyImpl(const OptCondImpl::FieldsList& fields)
{
    if (m_left.empty()) {
        return verifyBitCheck(fields);
    }

    // TODO:
    assert(!"NYI");
    return false;
}

bool OptCondExprImpl::hasUpdatedValue()
{
    return (!m_left.empty()) ||
           (!m_right.empty()) ||
           (!m_op.empty());
}

bool OptCondExprImpl::checkComparison(const std::string& expr, const std::string& op)
{
    if (hasUpdatedValue()) {
        return true;
    }

    std::size_t opPos = 0U;
    while (true) {
        opPos = expr.find(op, opPos);
        if (opPos == 0U) {
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
        return false;
    }

    auto rightBegPos = expr.find_first_not_of(' ', opPos + op.size());
    if (rightBegPos == std::string::npos) {
        return false;
    }

    m_left.assign(expr.begin(), expr.begin() + leftEndPos + 1U);
    m_op = op;
    m_right.assign(expr.begin() + rightBegPos, expr.end());

    if (m_left.empty() || m_right.empty()) {
        return false;
    }

    if (m_left[0] != Deref) {
        return false;
    }

    return true;
}

bool OptCondExprImpl::checkBool(const std::string& expr)
{
    if (hasUpdatedValue()) {
        return true;
    }

    assert(!expr.empty());
    if (expr[0] == Deref) {
        m_right = expr;
        return true;
    }

    if (expr[0] != '!') {
        return false;
    }

    auto valPos = expr.find_first_not_of(' ', 1);
    if (valPos == std::string::npos) {
        return false;
    }

    if (expr[valPos] != Deref) {
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
        [this, &name, &remPos](const auto& f)
        {
            auto& members = f.members();
            return this->findField(members, name, remPos);
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

bool OptCondExprImpl::verifyBitCheck(const OptCondImpl::FieldsList& fields)
{
    assert(!m_right.empty());
    assert(m_right[0] == Deref);

    std::size_t remPos = 1;
    auto field = findField(fields, m_right, remPos);
    if ((field == nullptr) ||
        (field->kind() != FieldImpl::Kind::Set)) {
        return false;
    }


    auto setField = static_cast<const SetFieldImpl*>(field);
    std::string bitName(m_right, remPos);
    auto iter = setField->bits().find(bitName);
    if (iter == setField->bits().end()) {
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

bool OptCondListImpl::verifyImpl(const OptCondImpl::FieldsList& fields)
{
    return std::all_of(
        m_conds.begin(), m_conds.end(),
        [&fields](auto& c)
        {
            return c->verify(fields);
        });
}



} // namespace bbmp
