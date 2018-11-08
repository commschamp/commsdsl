#pragma once

#include "LayerImpl.h"

namespace commsdsl
{

class ChecksumLayerImpl : public LayerImpl
{
    using Base = LayerImpl;
public:
    using Alg = ChecksumLayer::Alg;
    ChecksumLayerImpl(::xmlNodePtr node, ProtocolImpl& protocol);

    Alg alg() const
    {
        return m_alg;
    }

    const std::string& algName() const
    {
        return *m_algName;
    }

    const std::string& from() const
    {
        return *m_from;
    }

    const std::string& until() const
    {
        return *m_until;
    }

    bool verifyBeforeRead() const
    {
        return m_verifyBeforeRead;
    }

protected:
    virtual Kind kindImpl() const override final;
    virtual const XmlWrap::NamesList& extraPropsNamesImpl() const override final;
    virtual bool parseImpl() override final;
    virtual bool verifyImpl(const LayersList& layers) override final;

private:
    bool updateAlg();
    bool updateFrom();
    bool updateUntil();
    bool updateVerifyBeforeRead();

    Alg m_alg = Alg::NumOfValues;
    const std::string* m_algName = nullptr;
    const std::string* m_from = nullptr;
    const std::string* m_until = nullptr;
    bool m_verifyBeforeRead = false;
};

} // namespace commsdsl
