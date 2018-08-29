#pragma once

#include <cstdint>
#include <utility>
#include <vector>

#include "commsdsl/Endian.h"
#include "commsdsl/OptionalField.h"
#include "FieldImpl.h"
#include "OptCondImpl.h"

namespace commsdsl
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

    bool externalModeCtrl() const
    {
        return m_state.m_externalModeCtrl;
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

    OptCond wrappedCondition() const
    {
        return OptCond(m_cond.get());
    }

    OptCondImplPtr& cond()
    {
        return m_cond;
    }

    const OptCondImplPtr& cond() const
    {
        return m_cond;
    }

protected:
    virtual Kind kindImpl() const override;
    virtual Ptr cloneImpl() const override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual const XmlWrap::NamesList& extraPossiblePropsNamesImpl() const override;
    virtual const XmlWrap::NamesList& extraChildrenNamesImpl() const override;
    virtual bool reuseImpl(const FieldImpl& other) override;
    virtual bool parseImpl() override;
    virtual bool verifySiblingsImpl(const FieldsList& fields) const override;
    virtual std::size_t minLengthImpl() const override;
    virtual std::size_t maxLengthImpl() const override;

private:
    bool updateMode();
    bool updateExternalModeCtrl();
    bool updateField();
    bool updateSingleCondition();
    bool updateMultiCondition();
    bool checkFieldFromRef();
    bool checkFieldAsChild();
    const FieldImpl* getField() const;

    struct State
    {
        Mode m_mode = Mode::Tentative;
        const FieldImpl* m_extField = nullptr;
        bool m_externalModeCtrl = false;
    };

    State m_state;
    FieldImplPtr m_field;
    OptCondImplPtr m_cond;
};

} // namespace commsdsl
