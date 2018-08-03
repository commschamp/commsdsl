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
    virtual Kind kindImpl() const override;
    virtual bool parseImpl() override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;

private:
    bool m_idReplacement = false;
};

} // namespace commsdsl
