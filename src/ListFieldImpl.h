#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "bbmp/Endian.h"
#include "FieldImpl.h"

namespace bbmp
{

class ListFieldImpl : public FieldImpl
{
    using Base = FieldImpl;
public:

    ListFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    ListFieldImpl(const ListFieldImpl& other);

    unsigned count() const
    {
        return m_state.m_count;
    }

    bool hasElementField() const
    {
        return (m_state.m_extElementField != nullptr) ||
               static_cast<bool>(m_elementField);
    }

    Field elementField() const
    {
        if (m_state.m_extElementField != nullptr) {
            return Field(m_state.m_extElementField);
        }

        return Field(m_elementField.get());
    }


    bool hasCountPrefixField() const
    {
        return (m_state.m_extCountPrefixField != nullptr) ||
               static_cast<bool>(m_countPrefixField);
    }

    Field countPrefixField() const
    {
        if (m_state.m_extCountPrefixField != nullptr) {
            return Field(m_state.m_extCountPrefixField);
        }

        return Field(m_countPrefixField.get());
    }

    bool hasLengthPrefixField() const
    {
        return (m_state.m_extLengthPrefixField != nullptr) ||
               static_cast<bool>(m_lengthPrefixField);
    }

    Field lengthPrefixField() const
    {
        if (m_state.m_extLengthPrefixField != nullptr) {
            return Field(m_state.m_extCountPrefixField);
        }

        return Field(m_lengthPrefixField.get());
    }

    bool hasElemLengthPrefixField() const
    {
        return (m_state.m_extElemLengthPrefixField != nullptr) ||
               static_cast<bool>(m_elemLengthPrefixField);
    }

    Field elemLengthPrefixField() const
    {
        if (m_state.m_extElemLengthPrefixField != nullptr) {
            return Field(m_state.m_extCountPrefixField);
        }

        return Field(m_elemLengthPrefixField.get());
    }

    bool elemFixedLength() const
    {
        return m_state.m_elemFixedLength;
    }


protected:
    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual const XmlWrap::NamesList& extraPossiblePropsNamesImpl() const;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override;
    virtual bool reuseImpl(const FieldImpl& other) override;
    virtual bool parseImpl() override;
    virtual std::size_t minLengthImpl() const override;
    virtual std::size_t maxLengthImpl() const override;

private:
    void cloneFields(const ListFieldImpl& other);
    bool updateElement();
    bool updateCount();
    bool updateCountPrefix();
    bool updateLengthPrefix();
    bool updateElemLengthPrefix();
    bool updateElemFixedLength();
    bool checkElementFromRef();
    bool checkElementAsChild();
    bool checkPrefixFromRef(
        const std::string& type,
        const FieldImpl*& extField,
        FieldImplPtr& locField);
    bool checkPrefixAsChild(
        const std::string& type,
        const FieldImpl*& extField,
        FieldImplPtr& locField);
    const FieldImpl* getCountPrefixField() const;
    const FieldImpl* getLengthPrefixField() const;

    struct State
    {
        std::size_t m_count = 0U;
        const FieldImpl* m_extElementField = nullptr;
        const FieldImpl* m_extCountPrefixField = nullptr;
        const FieldImpl* m_extLengthPrefixField = nullptr;
        const FieldImpl* m_extElemLengthPrefixField = nullptr;
        bool m_elemFixedLength = false;
    };

    State m_state;
    FieldImplPtr m_elementField;
    FieldImplPtr m_countPrefixField;
    FieldImplPtr m_lengthPrefixField;
    FieldImplPtr m_elemLengthPrefixField;
};

} // namespace bbmp
