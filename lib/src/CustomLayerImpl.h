#pragma once

#include "LayerImpl.h"

namespace commsdsl
{

class CustomLayerImpl : public LayerImpl
{
    using Base = LayerImpl;
public:
    CustomLayerImpl(::xmlNodePtr node, ProtocolImpl& protocol);

    bool isIdReplacement() const
    {
        return m_idReplacement;
    }

protected:
    virtual Kind kindImpl() const override final;
    virtual bool parseImpl() override final;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override final;

private:
    bool m_idReplacement = false;
};

} // namespace commsdsl
