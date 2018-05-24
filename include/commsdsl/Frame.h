#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "CommsdslApi.h"
#include "Layer.h"

namespace commsdsl
{

class FrameImpl;
class COMMSDSL_API Frame
{
public:
    using LayersList = std::vector<Layer>;
    using AttributesMap = Schema::AttributesMap;
    using ElementsList = Schema::ElementsList;

    explicit Frame(const FrameImpl* impl);
    Frame(const Frame& other);
    ~Frame();

    bool valid() const;
    const std::string& name() const;
    const std::string& description() const;
    LayersList layers() const;
    std::string externalRef() const;

    const AttributesMap& extraAttributes() const;
    const ElementsList& extraElements() const;

protected:
    const FrameImpl* m_pImpl;
};

} // namespace commsdsl
