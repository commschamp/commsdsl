#include "RefFieldImpl.h"

#include <cassert>
#include "common.h"
#include "ProtocolImpl.h"

namespace bbmp
{

RefFieldImpl::RefFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}

RefFieldImpl::RefFieldImpl(const RefFieldImpl& other)
  : Base(other),
    m_field(other.m_field->clone())
{
}

FieldImpl::Ptr RefFieldImpl::cloneImpl() const
{
    return Ptr(new RefFieldImpl(*this));
}

const XmlWrap::NamesList& RefFieldImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::origStr()
    };

    return List;
}

const XmlWrap::NamesList& RefFieldImpl::extraChildrenNamesImpl() const
{
    return m_extraChildrenNames;
}

bool RefFieldImpl::parseImpl()
{
    if (!validateSinglePropInstance(common::origStr(), true)) {
        return false;
    }

    auto propsIter = props().find(common::origStr());
    assert (propsIter != props().end());

    auto* field = protocol().findField(propsIter->second);
    if (field == nullptr) {
        reportUnexpectedPropertyValue(common::origStr(), propsIter->second);
        return false;
    }

    if (!m_field) {
        m_field = field->clone();
    }

    assert(m_field->getNode() != getNode());

    m_field->setNode(getNode());
    if (!m_field->parse()) {
        return false;
    }

    m_extraChildrenNames.clear();
    auto& propsNames = m_field->extraPropsNames();
    auto& possibleNames = m_field->extraPossiblePropsNames();
    auto& extraChildren = m_field->extraChildrenNames();

    m_extraChildrenNames.insert(m_extraChildrenNames.end(), propsNames.begin(), propsNames.end());
    m_extraChildrenNames.insert(m_extraChildrenNames.end(), possibleNames.begin(), possibleNames.end());
    m_extraChildrenNames.insert(m_extraChildrenNames.end(), extraChildren.begin(), extraChildren.end());

    return true;
}

std::size_t RefFieldImpl::lengthImpl() const
{
    return m_field->length();
}

std::size_t RefFieldImpl::bitLengthImpl() const
{
    return m_field->bitLength();
}

} // namespace bbmp
