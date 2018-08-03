#pragma once

#include "LayerImpl.h"
#include "InterfaceImpl.h"

namespace commsdsl
{

class ValueLayerImpl : public LayerImpl
{
    using Base = LayerImpl;
public:
    using Interfaces = std::vector<const InterfaceImpl*>;
    using InterfacesList = ValueLayer::Interfaces;
    ValueLayerImpl(::xmlNodePtr node, ProtocolImpl& protocol);

    InterfacesList interfacesList() const;

    const std::string& fieldName() const
    {
        return *m_fieldName;
    }

    std::size_t fieldIdx() const;

    bool pseudo() const
    {
        return m_pseudo;
    }


protected:
    virtual Kind kindImpl() const override;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override;
    virtual bool parseImpl() override;
    virtual bool verifyImpl(const LayersList& layers) override;

private:
    bool updateInterfaces();
    bool updateFieldName();
    bool updatePseudo();

    Interfaces m_interfaces;
    const std::string* m_fieldName = nullptr;
    bool m_pseudo = false;
};

} // namespace commsdsl
