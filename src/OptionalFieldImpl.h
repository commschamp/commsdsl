#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "bbmp/Endian.h"
#include "bbmp/OptionalField.h"
#include "FieldImpl.h"

namespace bbmp
{

class OptionalFieldImpl : public FieldImpl
{
    using Base = FieldImpl;
public:
    using Mode = OptionalField::Mode;

    OptionalFieldImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    OptionalFieldImpl(const OptionalFieldImpl& other);

    Mode defaultMode() const
    {
        return m_state.m_mode;
    }

    bool hasField() const
    {
        return (m_state.m_extField != nullptr) || static_cast<bool>(m_field);
    }

    Field field() const
    {
        if (m_state.m_extField != nullptr) {
            return Field(m_state.m_extField);
        }

        return Field(m_field.get());
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
    bool updateMode();
    bool updateField();
    bool checkFieldFromRef();
    bool checkFieldAsChild();
    const FieldImpl* getField() const;

    struct State
    {
        Mode m_mode = Mode::Tentative;
        const FieldImpl* m_extField = nullptr;
        // TODO: conditions
    };

    State m_state;
    FieldImplPtr m_field;
};

} // namespace bbmp
