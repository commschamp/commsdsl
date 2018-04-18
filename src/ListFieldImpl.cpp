#include "ListFieldImpl.h"

#include <type_traits>
#include <algorithm>
#include <iterator>
#include <limits>
#include <cassert>

#include "common.h"
#include "ProtocolImpl.h"
#include "IntFieldImpl.h"

namespace bbmp
{

namespace
{

} // namespace

ListFieldImpl::ListFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}


FieldImpl::Kind ListFieldImpl::kindImpl() const
{
    return Kind::List;
}

ListFieldImpl::ListFieldImpl(const ListFieldImpl& other)
  : Base(other),
    m_state(other.m_state)
{
    cloneFields(other);
}

FieldImpl::Ptr ListFieldImpl::cloneImpl() const
{
    return Ptr(new ListFieldImpl(*this));
}

const XmlWrap::NamesList& ListFieldImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::countStr(),
        common::elemFixedLengthStr()
    };

    return List;
}

const XmlWrap::NamesList&ListFieldImpl::extraPossiblePropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::elementStr(),
        common::countPrefixStr(),
        common::lengthPrefixStr(),
        common::elemLengthPrefixStr(),
    };

    return List;
}

const XmlWrap::NamesList& ListFieldImpl::extraChildrenNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::elementStr(),
        common::countPrefixStr(),
        common::lengthPrefixStr(),
        common::elemLengthPrefixStr(),
    };

    return List;
}

bool ListFieldImpl::reuseImpl(const FieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const ListFieldImpl&>(other);
    m_state = castedOther.m_state;
    cloneFields(castedOther);
    return true;
}

bool ListFieldImpl::parseImpl()
{
    return
        updateElement() &&
        updateCount() &&
        updateCountPrefix() &&
        updateLengthPrefix() &&
        updateElemLengthPrefix() &&
        updateElemFixedLength();
}

std::size_t ListFieldImpl::minLengthImpl() const
{
    std::size_t extraLen = 0U;
    if (hasElemLengthPrefixField()) {
        extraLen += elemLengthPrefixField().minLength();
    }

    if (m_state.m_count != 0U) {
        assert(elementField().valid());
        auto elemMinLength = elementField().minLength();

        if (!m_state.m_elemFixedLength) {
            extraLen *= m_state.m_count;
        }

        return (m_state.m_count * elemMinLength) + extraLen;
    }

    if (hasCountPrefixField()) {
        return countPrefixField().minLength();
    }

    if (hasLengthPrefixField()) {
        return lengthPrefixField().minLength();
    }

    return 0U;
}

std::size_t ListFieldImpl::maxLengthImpl() const
{
    static const std::size_t MaxLen = std::numeric_limits<std::size_t>::max();
    std::size_t extraLen = 0U;
    if (hasElemLengthPrefixField()) {
        extraLen += elemLengthPrefixField().maxLength();
    }

    assert(elementField().valid());
    auto elemMaxLength = elementField().maxLength();
    if (m_state.m_count != 0U) {

        if (!m_state.m_elemFixedLength) {
            extraLen *= m_state.m_count;
        }

        return (m_state.m_count * elemMaxLength) + extraLen;
    }

    if (hasCountPrefixField()) {
        auto* prefixField = getCountPrefixField();
        assert(prefixField->kind() == Field::Kind::Int);
        auto& castedPrefix = static_cast<const IntFieldImpl&>(*prefixField);
        std::size_t count = static_cast<std::size_t>(castedPrefix.maxValue());
        std::size_t result = std::min(MaxLen / count, elemMaxLength) * count;

        if (m_state.m_elemFixedLength) {
            return result + extraLen;
        }

        extraLen *= std::min(MaxLen / count, extraLen) * count;

        if ((MaxLen - extraLen) <= result) {
            return MaxLen;
        }

        result += extraLen;

        if ((MaxLen - castedPrefix.maxLength()) <= result) {
            return MaxLen;
        }

        result += castedPrefix.maxLength();
        return result;
    }

    if (hasLengthPrefixField()) {
        auto* prefixField = getLengthPrefixField();
        assert(prefixField->kind() == Field::Kind::Int);
        auto& castedPrefix = static_cast<const IntFieldImpl&>(*prefixField);
        std::size_t result = static_cast<std::size_t>(castedPrefix.maxValue());
        if ((MaxLen - castedPrefix.maxLength()) <= result) {
            return MaxLen;
        }
        return result + castedPrefix.maxLength();
    }

    return MaxLen;
}

void ListFieldImpl::cloneFields(const ListFieldImpl& other)
{
    if (other.m_elementField) {
        assert(other.m_state.m_extElementField == nullptr);
        m_elementField = other.m_elementField->clone();
    }

    if (other.m_countPrefixField) {
        assert(other.m_state.m_extCountPrefixField == nullptr);
        m_countPrefixField = other.m_countPrefixField->clone();
    }

    if (other.m_lengthPrefixField) {
        assert(other.m_state.m_extLengthPrefixField == nullptr);
        m_lengthPrefixField = other.m_lengthPrefixField->clone();
    }

    if (other.m_elemLengthPrefixField) {
        assert(other.m_state.m_extElemLengthPrefixField == nullptr);
        m_elemLengthPrefixField = other.m_elemLengthPrefixField->clone();
    }
}

bool ListFieldImpl::updateElement()
{
    if ((!checkElementFromRef()) ||
        (!checkElementAsChild())) {
        return false;
    }

    if (!hasElementField()) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "List element hasn't been provided.";
        return false;
    }

    return true;
}


bool ListFieldImpl::updateCount()
{
    if (!validateSinglePropInstance(common::countStr())) {
        return false;
    }

    auto iter = props().find(common::countStr());
    if (iter == props().end()) {
        return true;
    }

    bool ok = false;
    auto newVal = common::strToUnsigned(iter->second, &ok);
    if ((!ok) || (newVal == 0U)) {
        reportUnexpectedPropertyValue(common::countStr(), iter->second);
        return false;
    }

    if ((hasCountPrefixField()) || (hasLengthPrefixField())) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot force fixed length after reusing data sequence with count or length prefixes.";
        return false;
    }

    m_state.m_count = newVal;
    return true;
}

bool ListFieldImpl::updateCountPrefix()
{
    if ((!checkPrefixFromRef(common::countPrefixStr(), m_state.m_extCountPrefixField, m_countPrefixField)) ||
        (!checkPrefixAsChild(common::countPrefixStr(), m_state.m_extCountPrefixField, m_countPrefixField))) {
        return false;
    }

    if (!hasCountPrefixField()) {
        return true;
    }

    if (m_state.m_count != 0U) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Element count prefix is not applicable to fixed length data sequences.";
        return false;
    }

    if (hasLengthPrefixField()) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Element count and serialisation length prefixes cannot be used together.";
        return false;
    }

    return true;
}

bool ListFieldImpl::updateLengthPrefix()
{
    if ((!checkPrefixFromRef(common::lengthPrefixStr(), m_state.m_extLengthPrefixField, m_lengthPrefixField)) ||
        (!checkPrefixAsChild(common::lengthPrefixStr(), m_state.m_extLengthPrefixField, m_lengthPrefixField))) {
        return false;
    }

    if (!hasLengthPrefixField()) {
        return true;
    }

    if (m_state.m_count != 0U) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Serialisation length prefix is not applicable to fixed length data sequences.";
        return false;
    }

    if (hasCountPrefixField()) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Element count and serialisation length prefixes cannot be used together.";
        return false;
    }

    return true;
}

bool ListFieldImpl::updateElemLengthPrefix()
{
    if ((!checkPrefixFromRef(common::elemLengthPrefixStr(), m_state.m_extElemLengthPrefixField, m_elemLengthPrefixField)) ||
        (!checkPrefixAsChild(common::elemLengthPrefixStr(), m_state.m_extElemLengthPrefixField, m_elemLengthPrefixField))) {
        return false;
    }

    return true;
}

bool ListFieldImpl::updateElemFixedLength()
{
    if (!validateSinglePropInstance(common::elemFixedLengthStr())) {
        return false;
    }

    do {
        auto iter = props().find(common::elemFixedLengthStr());
        if (iter == props().end()) {
            break;
        }

        bool ok = false;
        m_state.m_elemFixedLength = common::strToBool(iter->second, &ok);
        if (!ok) {
            reportUnexpectedPropertyValue(common::elemFixedLengthStr(), iter->second);
            return false;
        }
    } while (false);

    if (!m_state.m_elemFixedLength) {
        return true;
    }

    assert(hasElementField());
    auto elem = elementField();
    if (elem.minLength() != elem.maxLength()) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot have \"" << common::elemFixedLengthStr() << "\" property being set to "
            "true when list element has variable length.";
        return false;
    }
    return true;
}

bool ListFieldImpl::checkElementFromRef()
{
    if (!validateSinglePropInstance(common::elementStr())) {
        return false;
    }

    auto iter = props().find(common::elementStr());
    if (iter == props().end()) {
        return true;
    }

    auto* field = protocol().findField(iter->second);
    if (field == nullptr) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot find field referenced by \"" << common::elementStr() <<
            "\" property (" << iter->second << ").";
        return false;
    }

    m_elementField.reset();
    m_state.m_extElementField = field;
    return true;
}

bool ListFieldImpl::checkElementAsChild()
{
    auto children = XmlWrap::getChildren(getNode(), common::elementStr());
    if (children.empty()) {
        return true;
    }

    if (1U < children.size()) {
        logError() << "There must be only one occurance of \"" << common::elementStr() << "\".";
        return false;
    }

    auto child = children.front();
    auto elementFields = XmlWrap::getChildren(child);
    if (1U < elementFields.size()) {
        logError() << XmlWrap::logPrefix(child) <<
            "The \"" << common::elementStr() << "\" element is expected to define only "
            "single field";
        return false;
    }

    auto iter = props().find(common::elementStr());
    bool hasInProps = iter != props().end();
    if (elementFields.empty()) {
        assert(hasInProps);
        return true;
    }

    if (hasInProps) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "The \"" << common::elementStr() << "\" element is expected to define only "
            "single field";
        return false;
    }

    auto fieldNode = elementFields.front();
    assert(fieldNode->name != nullptr);
    std::string fieldKind(reinterpret_cast<const char*>(fieldNode->name));
    auto field = FieldImpl::create(fieldKind, fieldNode, protocol());
    if (!field) {
        logError() << XmlWrap::logPrefix(fieldNode) <<
            "Unknown field type \"" << fieldKind << "\".";
        return false;
    }

    field->setParent(this);
    if (!field->parse()) {
        return false;
    }

    m_state.m_extElementField = nullptr;
    m_elementField = std::move(field);
    return true;
}

bool ListFieldImpl::checkPrefixFromRef(
    const std::string& type,
    const FieldImpl*& extField,
    FieldImplPtr& locField)
{
    if (!validateSinglePropInstance(type)) {
        return false;
    }

    auto iter = props().find(type);
    if (iter == props().end()) {
        return true;
    }

    auto* field = protocol().findField(iter->second);
    if (field == nullptr) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "Cannot find field referenced by \"" << type <<
            "\" property (" << iter->second << ").";
        return false;
    }

    if (field->kind() != Kind::Int) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "The field referenced by \"" << type <<
            "\" property (" << iter->second << ") must be of type \"" << common::intStr() << "\".";
        return false;
    }

    locField.reset();
    extField = field;
    return true;
}

bool ListFieldImpl::checkPrefixAsChild(
    const std::string& type,
    const FieldImpl*& extField,
    FieldImplPtr& locField)
{
    auto children = XmlWrap::getChildren(getNode(), type);
    if (children.empty()) {
        return true;
    }

    if (1U < children.size()) {
        logError() << "There must be only one occurance of \"" << type << "\".";
        return false;
    }

    auto child = children.front();
    auto prefixFields = XmlWrap::getChildren(child);
    if (1U < prefixFields.size()) {
        logError() << XmlWrap::logPrefix(child) <<
            "The \"" << type << "\" element is expected to define only "
            "single field";
        return false;
    }

    auto iter = props().find(type);
    bool hasInProps = iter != props().end();
    if (prefixFields.empty()) {
        assert(hasInProps);
        return true;
    }

    if (hasInProps) {
        logError() << XmlWrap::logPrefix(getNode()) <<
            "The \"" << type << "\" element is expected to define only "
            "single field";
        return false;
    }

    auto fieldNode = prefixFields.front();
    assert(fieldNode->name != nullptr);
    std::string fieldKind(reinterpret_cast<const char*>(fieldNode->name));
    if (fieldKind != common::intStr()) {
        logError() << XmlWrap::logPrefix(fieldNode) <<
            "The field defined by \"" << type <<
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

    extField = nullptr;
    locField = std::move(field);
    return true;
}

const FieldImpl* ListFieldImpl::getCountPrefixField() const
{
    if (m_state.m_extCountPrefixField != nullptr) {
        assert(!m_countPrefixField);
        return m_state.m_extCountPrefixField;
    }

    assert(m_countPrefixField);
    return m_countPrefixField.get();
}

const FieldImpl*ListFieldImpl::getLengthPrefixField() const
{
    if (m_state.m_extLengthPrefixField != nullptr) {
        assert(!m_lengthPrefixField);
        return m_state.m_extLengthPrefixField;
    }

    assert(m_lengthPrefixField);
    return m_lengthPrefixField.get();
}


} // namespace bbmp
