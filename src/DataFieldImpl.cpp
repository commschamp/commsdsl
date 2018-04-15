#include "DataFieldImpl.h"

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

DataFieldImpl::DataFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}


FieldImpl::Kind DataFieldImpl::kindImpl() const
{
    return Kind::Data;
}

DataFieldImpl::DataFieldImpl(const DataFieldImpl& other)
  : Base(other),
    m_state(other.m_state)
{
    if (other.m_prefixField) {
        assert(other.m_state.m_extPrefixField == nullptr);
        m_prefixField = other.m_prefixField->clone();
    }
}

FieldImpl::Ptr DataFieldImpl::cloneImpl() const
{
    return Ptr(new DataFieldImpl(*this));
}

const XmlWrap::NamesList& DataFieldImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::lengthStr(),
    };

    return List;
}

const XmlWrap::NamesList&DataFieldImpl::extraPossiblePropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::lengthPrefixStr(),
    };

    return List;
}

const XmlWrap::NamesList& DataFieldImpl::extraChildrenNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::lengthPrefixStr()
    };

    return List;
}

bool DataFieldImpl::reuseImpl(const FieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const DataFieldImpl&>(other);
    m_state = castedOther.m_state;
    if (castedOther.m_prefixField) {
        assert(m_state.m_extPrefixField == nullptr);
        m_prefixField = castedOther.m_prefixField->clone();
    }
    else {
        assert(!m_prefixField);
    }
    return true;
}

bool DataFieldImpl::parseImpl()
{
    return
        updateLength() &&
        updatePrefix() &&
        updateDefaultValue();
}

std::size_t DataFieldImpl::minLengthImpl() const
{
    if (m_state.m_length != 0U) {
        return m_state.m_length;
    }

    if (hasPrefixField()) {
        return getPrefixField()->minLength();
    }

    return 0U;
}

std::size_t DataFieldImpl::maxLengthImpl() const
{
    if (m_state.m_length != 0U) {
        return m_state.m_length;
    }

    if (hasPrefixField()) {
        auto* prefixField = getPrefixField();
        assert(prefixField->kind() == Field::Kind::Int);
        auto& castedPrefix = static_cast<const IntFieldImpl&>(*prefixField);
        return castedPrefix.maxLength() + castedPrefix.maxValue();
    }

    return std::numeric_limits<std::size_t>::max();
}

bool DataFieldImpl::updateDefaultValue()
{
    if (!validateSinglePropInstance(common::defaultValueStr())) {
        return false;
    }

    auto iter = props().find(common::defaultValueStr());
    if (iter != props().end()) {
        m_state.m_defaultValue = iter->second;
    }

    m_state.m_defaultValue.erase(
        std::remove(m_state.m_defaultValue.begin(), m_state.m_defaultValue.end(), ' '),
        m_state.m_defaultValue.end());

    if ((m_state.m_defaultValue.size() %2) != 0) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Property \"" << common::defaultValueStr() << "\" of element \"" << name() <<
            "\" has unexpected value (" << iter->second << "), expected to "
            "be hex values string with even number of non-white characters.";
        return false;

    }


    auto validChars =
        std::all_of(
            m_state.m_defaultValue.begin(), m_state.m_defaultValue.end(),
            [](char ch)
            {
                auto c = static_cast<char>(std::tolower(ch));
                return (('0' <= c) && (c <='9')) ||
                       (('a' <= c) && (c <= 'f'));
            });

    if (!validChars) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Property \"" << common::defaultValueStr() << "\" of element \"" << name() <<
                      "\" has unexpected value (" << iter->second << "), expected to "
                      "be hex values string.";
        return false;
    }

    if ((m_state.m_length != 0U) &&
        (m_state.m_length < (m_state.m_defaultValue.size() / 2U))) {
        logWarning() << XmlWrap::logPrefix(getNode()) <<
            "The default value (" << m_state.m_defaultValue << ") is too long "
            "for proper serialisation.";
    }

    return true;
}

bool DataFieldImpl::updateLength()
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

    if (hasPrefixField()) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot force fixed length after reusing data sequence with length prefix.";
        return false;
    }

    m_state.m_length = newVal;
    return true;
}

bool DataFieldImpl::updatePrefix()
{
    if ((!checkPrefixFromRef()) ||
        (!checkPrefixAsChild())) {
        return false;
    }

    if (!hasPrefixField()) {
        return true;
    }

    if (m_state.m_length != 0U) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Length prefix field is not applicable to fixed length data sequences.";
        return false;
    }

    return true;
}

bool DataFieldImpl::checkPrefixFromRef()
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

    m_prefixField.reset();
    m_state.m_extPrefixField = field;
    assert(hasPrefixField());
    return true;
}

bool DataFieldImpl::checkPrefixAsChild()
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

    m_state.m_extPrefixField = nullptr;
    m_prefixField = std::move(field);
    return true;
}

const FieldImpl* DataFieldImpl::getPrefixField() const
{
    if (m_state.m_extPrefixField != nullptr) {
        assert(!m_prefixField);
        return m_state.m_extPrefixField;
    }

    assert(m_prefixField);
    return m_prefixField.get();
}


} // namespace bbmp
