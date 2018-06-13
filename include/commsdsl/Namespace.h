#pragma once

#include <string>
#include <vector>

#include "CommsdslApi.h"
#include "Field.h"
#include "Message.h"
#include "Schema.h"
#include "Interface.h"
#include "Frame.h"

namespace commsdsl
{

class NamespaceImpl;
class COMMSDSL_API Namespace
{
public:
    using NamespacesList = std::vector<Namespace>;
    using FieldsList = std::vector<Field>;
    using MessagesList = std::vector<Message>;
    using InterfacesList = std::vector<Interface>;
    using FramesList = std::vector<Frame>;
    using AttributesMap = Schema::AttributesMap;
    using ElementsList = Schema::ElementsList;

    explicit Namespace(const NamespaceImpl* impl);
    Namespace(const Namespace& other);
    ~Namespace();

    const std::string& name() const;
    const std::string& description() const;
    NamespacesList namespaces() const;
    FieldsList fields() const;
    MessagesList messages() const;
    InterfacesList interfaces() const;
    FramesList frames() const;
    std::string externalRef() const;

    const AttributesMap& extraAttributes() const;
    const ElementsList& extraElements() const;

private:
    const NamespaceImpl* m_pImpl;
};

} // namespace commsdsl
