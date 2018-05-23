#include "commsdsl/Schema.h"

#include <limits>
#include <cassert>

#include "SchemaImpl.h"
#include "common.h"

namespace commsdsl
{

Schema::Schema(const SchemaImpl* impl)
  : m_pImpl(impl)
{
}

bool Schema::valid() const
{
    return m_pImpl != nullptr;
}

const std::string& Schema::name() const
{
    if (!valid()) {
        assert(!"Unexpected call on invalid schema object");
        return common::emptyString();
    }

    return m_pImpl->name();
}

const std::string& Schema::description() const
{
    if (!valid()) {
        assert(!"Unexpected call on invalid schema object");
        return common::emptyString();
    }

    return m_pImpl->description();
}

unsigned Schema::id() const
{
    if (!valid()) {
        assert(!"Unexpected call on invalid schema object");
        return std::numeric_limits<unsigned>::max();
    }

    return m_pImpl->id();
}

unsigned Schema::version() const
{
    if (!valid()) {
        assert(!"Unexpected call on invalid schema object");
        return std::numeric_limits<unsigned>::max();
    }

    return m_pImpl->version();
}

Endian Schema::endian() const
{
    if (!valid()) {
        assert(!"Unexpected call on invalid schema object");
        return Endian_NumOfValues;
    }

    return m_pImpl->endian();
}

bool Schema::nonUniqueMsgIdAllowed() const
{
    if (!valid()) {
        assert(!"Unexpected call on invalid schema object");
        return false;
    }

    return m_pImpl->nonUniqueMsgIdAllowed();
}

const Schema::AttributesMap& Schema::extraAttributes() const
{
    if (!valid()) {
        assert(!"Unexpected call on invalid schema object");
        static const AttributesMap Map;
        return Map;
    }

    return m_pImpl->extraAttributes();
}

const Schema::ElementsList& Schema::extraElements() const
{
    if (!valid()) {
        assert(!"Unexpected call on invalid schema object");
        static const ElementsList List;
        return List;
    }

    return m_pImpl->extraChildrenElements();

}

} // namespace commsdsl
