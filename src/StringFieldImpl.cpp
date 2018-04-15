#include "StringFieldImpl.h"

#include <type_traits>
#include <algorithm>
#include <iterator>
#include <limits>
#include <cassert>

#include "common.h"
#include "ProtocolImpl.h"
#include "IntFieldImpl.h"
#include "util.h"

namespace bbmp
{

namespace
{

} // namespace

StringFieldImpl::StringFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}


FieldImpl::Kind StringFieldImpl::kindImpl() const
{
    return Kind::String;
}

StringFieldImpl::StringFieldImpl(const StringFieldImpl& other)
  : Base(other),
    m_state(other.m_state)
  {
      m_prefixField = other.m_prefixField->clone();
  }

FieldImpl::Ptr StringFieldImpl::cloneImpl() const
{
    return Ptr(new StringFieldImpl(*this));
}

const XmlWrap::NamesList& StringFieldImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::lengthStr(),
        common::encodingStr(),
        common::zeroTermSuffixStr()
    };

    return List;
}

const XmlWrap::NamesList&StringFieldImpl::extraPossiblePropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::lengthPrefixStr(),
    };

    return List;
}

const XmlWrap::NamesList& StringFieldImpl::extraChildrenNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::lengthPrefixStr()
    };

    return List;
}

bool StringFieldImpl::reuseImpl(const FieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const StringFieldImpl&>(other);
    m_state = castedOther.m_state;
    m_prefixField = castedOther.m_prefixField->clone();
    return true;
}

bool StringFieldImpl::parseImpl()
{
    return
        updateEncoding() &&
        updateLength() &&
        updatePrefix() &&
        updateZeroTerm() &&
        updateDefaultValue();
}

std::size_t StringFieldImpl::minLengthImpl() const
{
    if (m_state.m_length != 0U) {
        return m_state.m_length;
    }

    if (m_state.m_haxZeroSuffix) {
        return 1U;
    }

    if (m_prefixField) {
        return m_prefixField->minLength();
    }

    return 0U;
}

std::size_t StringFieldImpl::maxLengthImpl() const
{
    if (m_state.m_length != 0U) {
        return m_state.m_length;
    }

    if (m_prefixField) {
        assert(m_prefixField->kind() == Field::Kind::Int);
        auto& castedPrefix = static_cast<IntFieldImpl&>(*m_prefixField);
        return castedPrefix.maxLength() + castedPrefix.maxValue();
    }

    return std::numeric_limits<std::size_t>::max();
}

bool StringFieldImpl::updateDefaultValue()
{
    if (!validateSinglePropInstance(common::defaultValueStr())) {
        return false;
    }

    auto iter = props().find(common::defaultValueStr());
    if (iter != props().end()) {
        m_state.m_defaultValue = iter->second;
    }

    if (m_state.m_length < m_state.m_defaultValue.size()) {
        logWarning() << XmlWrap::logPrefix(getNode()) <<
            "The default value is too long for proper serialisation";
    }

    return true;
}

bool StringFieldImpl::updateEncoding()
{
    if (!validateSinglePropInstance(common::encodingStr())) {
        return false;
    }

    auto iter = props().find(common::encodingStr());
    if (iter != props().end()) {
        m_state.m_encoding = iter->second;
    }

    return true;
}

bool StringFieldImpl::updateLength()
{
    if (!validateSinglePropInstance(common::lengthStr())) {
        return false;
    }

    auto iter = props().find(common::lengthStr());
    if (iter == props().end()) {
        return true;
    }

    bool ok = false;
    auto newVal = common::strToUnsigned(iter->second, &ok);
    if ((!ok) || (newVal == 0U)) {
        reportUnexpectedPropertyValue(common::lengthStr(), iter->second);
        return false;
    }

    if (m_state.m_length == newVal) {
        return true;
    }

    if (m_prefixField) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot force fixed length after reusing string with length prefix.";
        return false;
    }

    if (m_state.m_haxZeroSuffix) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot force fixed length after reusing string with zero suffix.";
        return false;
    }

    m_state.m_length = newVal;
    return true;
}

bool StringFieldImpl::updatePrefix()
{
    if ((!checkPrefixFromRef()) ||
        (!checkPrefixAsChild())) {
        return false;
    }

    if (!m_prefixField) {
        return true;
    }

    if (m_state.m_length != 0U) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Length prefix field is not applicable to fixed length strings.";
        return false;
    }

    if (m_state.m_haxZeroSuffix) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Length prefix field is not applicable to zero terminated strings.";
        return false;
    }

    return true;
}

bool StringFieldImpl::updateZeroTerm()
{
    if (!validateSinglePropInstance(common::zeroTermSuffixStr())) {
        return false;
    }

    auto iter = props().find(common::zeroTermSuffixStr());
    if (iter == props().end()) {
        return true;
    }

    bool ok = false;
    auto newVal = common::strToBool(iter->second, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::zeroTermSuffixStr(), iter->second);
        return false;
    }

    if (newVal == m_state.m_haxZeroSuffix) {
        return true;
    }

    if (!newVal) {
        m_state.m_haxZeroSuffix = newVal;
        return true;
    }

    if (m_state.m_length != 0U) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot apply zero suffix to fixed length strings.";
        return false;
    }

    if (m_prefixField) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot apply zero suffix to strings having length prefix.";
        return false;
    }

    m_state.m_haxZeroSuffix = newVal;
    return true;
}

bool StringFieldImpl::checkPrefixFromRef()
{
    if (!validateSinglePropInstance(common::lengthPrefixStr())) {
        return false;
    }

    auto iter = props().find(common::lengthPrefixStr());
    if (iter == props().end()) {
        return true;
    }

    auto* field = protocol().findField(iter->second);
    if (field == nullptr) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot find field referenced by \"" << common::lengthPrefixStr() <<
            "\" property (" << iter->second << ").";
        return false;
    }

    if (field->kind() != Kind::Int) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "The field referenced by \"" << common::lengthPrefixStr() <<
            "\" property (" << iter->second << ") must be of type \"" << common::intStr() << "\".";
        return false;
    }

    m_prefixField = field->clone();
    assert(m_prefixField);
    return true;
}

bool StringFieldImpl::checkPrefixAsChild()
{
    auto children = XmlWrap::getChildren(getNode(), common::lengthPrefixStr());
    if (children.empty()) {
        return true;
    }

    if (1U < children.size()) {
        logError() << "There must be only one occurance of \"" << common::lengthPrefixStr() << "\".";
        return false;
    }

    auto child = children.front();
    auto prefixFields = XmlWrap::getChildren(child);
    if (1U < prefixFields.size()) {
        logError() << XmlWrap::logPrefix(child) <<
            "The \"" << common::lengthPrefixStr() << "\" element is expected to define only "
            "single field";
        return false;
    }

    auto iter = props().find(common::lengthPrefixStr());
    bool hasInProps = iter != props().end();
    if (prefixFields.empty()) {
        assert(hasInProps);
        return true;
    }

    if (hasInProps) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "The \"" << common::lengthPrefixStr() << "\" element is expected to define only "
            "single field";
        return false;
    }

    auto fieldNode = prefixFields.front();
    assert(fieldNode->name != nullptr);
    std::string fieldKind(reinterpret_cast<const char*>(fieldNode->name));
    if (fieldKind != common::intStr()) {
        logError() << XmlWrap::logPrefix(fieldNode) <<
            "The field defined by \"" << common::lengthPrefixStr() <<
            "\" element must be of type \"" << common::intStr() << "\".";
        return false;
    }

    auto field = FieldImpl::create(fieldKind, fieldNode, protocol());
    if (!field) {
        assert(!"Should not happen");
        return false;
    }

    field->setParent(this);
    if (!field->parse()) {
        return false;
    }

    m_prefixField = std::move(field);
    return true;
}


} // namespace bbmp
