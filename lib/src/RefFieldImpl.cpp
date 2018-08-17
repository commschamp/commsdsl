#include "RefFieldImpl.h"

#include <cassert>
#include "common.h"
#include "ProtocolImpl.h"

namespace commsdsl
{

RefFieldImpl::RefFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}

RefFieldImpl::RefFieldImpl(const RefFieldImpl&) = default;

FieldImpl::Kind RefFieldImpl::kindImpl() const
{
    return Kind::Ref;
}

FieldImpl::Ptr RefFieldImpl::cloneImpl() const
{
    return Ptr(new RefFieldImpl(*this));
}

const XmlWrap::NamesList& RefFieldImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::fieldStr()
    };

    return List;
}

bool RefFieldImpl::reuseImpl(const FieldImpl& other)
{
    assert(other.kind() == kind());
    auto& castedOther = static_cast<const RefFieldImpl&>(other);
    m_field = castedOther.m_field;
    assert(m_field != nullptr);
    return true;
}

bool RefFieldImpl::parseImpl()
{
    bool mustHave = m_field == nullptr;
    if (!validateSinglePropInstance(common::fieldStr(), mustHave)) {
        return false;
    }

    auto propsIter = props().find(common::fieldStr());
    if (propsIter == props().end()) {
        assert(m_field != nullptr);
        return true;
    }

    m_field = protocol().findField(propsIter->second);
    if (m_field == nullptr) {
        reportUnexpectedPropertyValue(common::fieldStr(), propsIter->second);
        return false;
    }

    if (displayName().empty() && (!m_field->displayName().empty())) {
        setDisplayName(m_field->displayName());
    }

    return true;
}

std::size_t RefFieldImpl::minLengthImpl() const
{
    assert(m_field != nullptr);
    return m_field->minLength();
}

std::size_t RefFieldImpl::maxLengthImpl() const
{
    assert(m_field != nullptr);
    return m_field->maxLength();
}

bool RefFieldImpl::isComparableToValueImpl(const std::string& val) const
{
    assert(m_field != nullptr);
    return m_field->isComparableToValue(val);
}

bool RefFieldImpl::isComparableToFieldImpl(const FieldImpl& field) const
{
    assert(m_field != nullptr);
    return m_field->isComparableToField(field);
}

} // namespace commsdsl
