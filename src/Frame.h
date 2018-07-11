#pragma once

#include <map>
#include <string>
#include <memory>

#include "commsdsl/Frame.h"

#include "Layer.h"

namespace commsdsl2comms
{

class Generator;
class Frame
{
public:

    //using FieldsMap = std::map<std::string, FieldPtr>;
    explicit Frame(Generator& gen, const commsdsl::Frame& obj)
      : m_generator(gen),
        m_dslObj(obj)
    {
    }

    const std::string name() const
    {
        return m_dslObj.name();
    }

    bool prepare();

    bool write();

    std::string getDefaultOptions() const;

    const std::string& externalRef() const
    {
        return m_externalRef;
    }

private:

    bool writeProtocol();
    std::string getDescription() const;
    std::string getIncludes() const;
    std::string getLayersDef() const;
    std::string getFrameDef() const;
    std::string getLayersAccess() const;
    std::string getLayersAccessDoc() const;
    std::string getInputMessages() const;

    bool hasIdLayer() const;

    Generator& m_generator;
    commsdsl::Frame m_dslObj;
    std::string m_externalRef;
    std::vector<LayerPtr> m_layers;
};

using FramePtr = std::unique_ptr<Frame>;

inline
FramePtr createFrame(Generator& gen, const commsdsl::Frame& msg)
{
    return FramePtr(new Frame(gen, msg));
}

} // namespace commsdsl2comms
