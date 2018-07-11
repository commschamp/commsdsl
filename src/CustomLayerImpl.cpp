#include "CustomLayerImpl.h"

#include "ProtocolImpl.h"
#include "common.h"

namespace commsdsl
{

CustomLayerImpl::CustomLayerImpl(::xmlNodePtr node, ProtocolImpl& protocol)
  : Base(node, protocol)
{
}

LayerImpl::Kind CustomLayerImpl::kindImpl() const
{
    return Kind::Custom;
}

bool CustomLayerImpl::parseImpl()
{
    if (!validateSinglePropInstance(common::idReplacementStr())) {
        return false;
    }

    auto iter = props().find(common::idReplacementStr());
    if (iter == props().end()) {
        return true;
    }

    bool ok = false;
    m_idReplacement = common::strToBool(iter->second, &ok);
    if (!ok) {
        reportUnexpectedPropertyValue(common::idReplacementStr(), iter->second);
        return false;
    }
    return true;
}

const XmlWrap::NamesList& CustomLayerImpl::extraPropsNamesImpl() const
{
    static const XmlWrap::NamesList List = {
        common::idReplacementStr()
    };

    return List;
}

} // namespace commsdsl
