#include "OptionalFieldImpl.h"

#include <type_traits>
#include <algorithm>
#include <iterator>
#include <limits>
#include <cassert>
#include <map>

#include "common.h"
#include "ProtocolImpl.h"

namespace bbmp
{

namespace
{

} // namespace

OptionalFieldImpl::OptionalFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}


FieldImpl::Kind OptionalFieldImpl::kindImpl() const
{
    return Kind::Optional;
}

OptionalFieldImpl::OptionalFieldImpl(const OptionalFieldImpl& other)
  : Base(other),
    m_state(other.m_state)
{
    if (other.m_field) {
        assert(other.m_state.m_extField == nullptr);
        m_field = other.m_field->clone();
    }

    if (other.m_cond) {
        m_cond = other.m_cond->clone();
    }
}

FieldImpl::Ptr OptionalFieldImpl::cloneImpl() const
{
    return Ptr(new OptionalFieldImpl(*this));
}

const XmlWrap::NamesList& OptionalFieldImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::defaultModeStr(),
        common::condStr()
    };

    return List;
}

const XmlWrap::NamesList&OptionalFieldImpl::extraPossiblePropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::fieldStr(),
    };

    return List;
}

const XmlWrap::NamesList& OptionalFieldImpl::extraChildrenNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::fieldStr(),
        common::orStr(),
        common::andStr()
    };

    return List;
}

bool OptionalFieldImpl::reuseImpl(const FieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const OptionalFieldImpl&>(other);
    m_state = castedOther.m_state;
    if (castedOther.m_field) {
        assert(m_state.m_extField == nullptr);
        m_field = castedOther.m_field->clone();
    }
    else {
        assert(!m_field);
    }
    return true;
}

bool OptionalFieldImpl::parseImpl()
{
    return
        updateMode() &&
        updateField() &&
        updateSingleCondition();
}

std::size_t OptionalFieldImpl::minLengthImpl() const
{
    return 0U;
}

std::size_t OptionalFieldImpl::maxLengthImpl() const
{
    assert(hasField());
    return getField()->maxLength();
}

bool OptionalFieldImpl::updateMode()
{
    if (!validateSinglePropInstance(common::defaultModeStr())) {
        return false;
    }

    auto iter = props().find(common::defaultModeStr());
    if (iter == props().end()) {
        return true;
    }

    static const std::map<std::string, Mode> Map = {
        std::make_pair("tent", Mode::Tentative),
        std::make_pair("tentative", Mode::Tentative),
        std::make_pair("t", Mode::Tentative),
        std::make_pair("miss", Mode::Missing),
        std::make_pair("missing", Mode::Missing),
        std::make_pair("m", Mode::Missing),
        std::make_pair("exists", Mode::Exists),
        std::make_pair("exist", Mode::Exists),
        std::make_pair("e", Mode::Exists),
    };

    auto modeStr = common::toLowerCopy(iter->second);
    auto mapIter = Map.find(modeStr);
    if (mapIter == Map.end()) {
        reportUnexpectedPropertyValue(common::defaultModeStr(), iter->second);
        return false;
    }

    m_state.m_mode = mapIter->second;
    return true;
}

bool OptionalFieldImpl::updateField()
{
    if ((!checkFieldFromRef()) ||
        (!checkFieldAsChild())) {
        return false;
    }

    if (!hasField()) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Field itself hasn't been provided.";
        return false;
    }

    return true;
}

bool OptionalFieldImpl::updateSingleCondition()
{
    if (!validateSinglePropInstance(common::condStr())) {
        return false;
    }

    auto iter = props().find(common::condStr());
    if (iter == props().end()) {
        return true;
    }

    if (m_cond) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Overriding non-empty condition(s) is not allowed";
        return false;
    }

    if ((!isBundleMember()) && (!isMessageMember())) {
        logWarning() << XmlWrap::logPrefix(getNode()) <<
            "Condition for existing mode are applicable only to members of \"" <<
            common::bundleStr() << "\" and \"" << common::messageStr() << "\".";
    }

    std::unique_ptr<OptCondExprImpl> cond(new OptCondExprImpl);
    if (!cond->parse(iter->second, getNode(), protocol().logger())) {
        return false;
    }

    m_cond = std::move(cond);
    return true;
}

bool OptionalFieldImpl::checkFieldFromRef()
{
    if (!validateSinglePropInstance(common::fieldStr())) {
        return false;
    }

    auto iter = props().find(common::fieldStr());
    if (iter == props().end()) {
        return true;
    }

    auto* field = protocol().findField(iter->second);
    if (field == nullptr) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot find field referenced by \"" << common::fieldStr() <<
            "\" property (" << iter->second << ").";
        return false;
    }

    m_field.reset();
    m_state.m_extField = field;
    assert(hasField());
    return true;
}

bool OptionalFieldImpl::checkFieldAsChild()
{
    auto children = XmlWrap::getChildren(getNode(), common::fieldStr());
    if (children.empty()) {
        return true;
    }

    if (1U < children.size()) {
        logError() << "There must be only one occurance of \"" << common::fieldStr() << "\".";
        return false;
    }

    auto child = children.front();
    auto fields = XmlWrap::getChildren(child);
    if (1U < fields.size()) {
        logError() << XmlWrap::logPrefix(child) <<
            "The \"" << common::fieldStr() << "\" element is expected to define only "
            "single field";
        return false;
    }

    auto iter = props().find(common::fieldStr());
    bool hasInProps = iter != props().end();
    if (fields.empty()) {
        assert(hasInProps);
        return true;
    }

    if (hasInProps) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "The \"" << common::fieldStr() << "\" element is expected to define only "
            "single field";
        return false;
    }

    auto fieldNode = fields.front();
    std::string fieldKind(reinterpret_cast<const char*>(fieldNode->name));
    auto field = FieldImpl::create(fieldKind, fieldNode, protocol());
    if (!field) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Unknown field type \"" << fieldKind;
        return false;
    }

    field->setParent(this);
    if (!field->parse()) {
        return false;
    }

    m_state.m_extField = nullptr;
    m_field = std::move(field);
    assert(m_field->externalRef().empty());
    return true;
}

const FieldImpl* OptionalFieldImpl::getField() const
{
    if (m_state.m_extField != nullptr) {
        assert(!m_field);
        return m_state.m_extField;
    }

    assert(m_field);
    return m_field.get();
}


} // namespace bbmp
