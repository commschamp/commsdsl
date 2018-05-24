#pragma once

#include <memory>
#include <map>
#include <string>
#include <cstdint>

#include "XmlWrap.h"
#include "Logger.h"
#include "Object.h"
#include "commsdsl/Frame.h"
#include "commsdsl/Protocol.h"
#include "LayerImpl.h"

namespace commsdsl
{

class ProtocolImpl;
class FrameImpl : public Object
{
    using Base = Object;
public:
    using Ptr = std::unique_ptr<FrameImpl>;
    using PropsMap = XmlWrap::PropsMap;
    using LayersList = Frame::LayersList;
    using ContentsList = XmlWrap::ContentsList;

    FrameImpl(::xmlNodePtr node, ProtocolImpl& protocol);
    FrameImpl(const FrameImpl&) = delete;
    FrameImpl(FrameImpl&&) = default;
    ~FrameImpl() = default;

    ::xmlNodePtr getNode() const
    {
        return m_node;
    }

    bool parse();

    const PropsMap& props() const
    {
        return m_props;
    }

    const std::string& name() const;
    const std::string& description() const;

    LayersList layersList() const;

    std::string externalRef() const;

    const PropsMap& extraAttributes() const
    {
        return m_extraAttrs;
    }

    const ContentsList& extraChildren() const
    {
        return m_extraChildren;
    }

protected:

    virtual ObjKind objKindImpl() const override final;

private:
    LogWrapper logError() const;
    LogWrapper logWarning() const;
    LogWrapper logInfo() const;

    static const XmlWrap::NamesList& commonProps();
    static XmlWrap::NamesList allNames();

    bool validateSinglePropInstance(const std::string& str, bool mustHave = false);
    bool validateAndUpdateStringPropValue(const std::string& str, const std::string*& valuePtr, bool mustHave = false);
    void reportUnexpectedPropertyValue(const std::string& propName, const std::string& propValue);
    bool updateName();
    bool updateDescription();
    bool updateLayers();
    void cloneLayersFrom(const FrameImpl& other);
    bool updateExtraAttrs();
    bool updateExtraChildren();

    ::xmlNodePtr m_node = nullptr;
    ProtocolImpl& m_protocol;
    PropsMap m_props;
    PropsMap m_extraAttrs;
    ContentsList m_extraChildren;

    const std::string* m_name = nullptr;
    const std::string* m_description = nullptr;
    std::vector<LayerImplPtr> m_layers;
};

using FrameImplPtr = FrameImpl::Ptr;

} // namespace commsdsl
