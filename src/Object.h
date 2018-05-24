#pragma once

#include <limits>

#include "commsdsl/Protocol.h"

namespace commsdsl
{

class Object
{
public:
    enum class ObjKind
    {
        Namespace,
        Field,
        Message,
        Interface,
        Frame,
        Layer,
        NumOfValues
    };

    Object* getParent()
    {
        return m_parent;
    }

    const Object* getParent() const
    {
        return m_parent;
    }

    void setParent(Object* obj)
    {
        m_parent = obj;
    }

    ObjKind objKind() const
    {
        return objKindImpl();
    }

    unsigned getSinceVersion() const
    {
        return m_rState.m_sinceVersion;
    }

    void setSinceVersion(unsigned val)
    {
        m_rState.m_sinceVersion = val;
    }

    unsigned getDeprecated() const
    {
        return m_rState.m_deprecated;
    }

    bool isDeprecatedRemoved() const
    {
        return m_rState.m_deprecatedRemoved;
    }

protected:
    virtual ObjKind objKindImpl() const = 0;

    void setDeprecated(unsigned val)
    {
        m_rState.m_deprecated = val;
    }

    void setDeprecatedRemoved(bool val)
    {
        m_rState.m_deprecatedRemoved = val;
    }

    void reuseState(const Object& other)
    {
        m_rState = other.m_rState;
    }


private:
    struct ReusableState
    {
        unsigned m_sinceVersion = 0U;
        unsigned m_deprecated = Protocol::notYetDeprecated();
        bool m_deprecatedRemoved = false;
    };

    Object* m_parent = nullptr;
    ReusableState m_rState;
};

} // namespace commsdsl
