#pragma once

#include <limits>

#include "bbmp/Protocol.h"

namespace bbmp
{

class Object
{
public:
    enum class ObjKind
    {
        Namespace,
        Field,
        Message,
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

    unsigned getMinSinceVersion() const
    {
        return m_rState.m_minSinceVersion;
    }

    unsigned getMaxSinceVersion() const
    {
        return m_rState.m_maxSinceVersion;
    }

    unsigned getDeprecated() const
    {
        return m_rState.m_deprecated;
    }

    void setMaxSinceVersion(unsigned val)
    {
        m_rState.m_maxSinceVersion = val;
        if (m_parent != nullptr) {
            m_parent->setMaxSinceVersion(val);
        }
    }

protected:
    virtual ObjKind objKindImpl() const = 0;

    void setMinSinceVersion(unsigned val)
    {
        m_rState.m_minSinceVersion = val;
    }

    void setDeprecated(unsigned val)
    {
        m_rState.m_deprecated = val;
    }

    void reuseState(const Object& other)
    {
        m_rState = other.m_rState;
    }


private:
    struct ReusableState
    {
        unsigned m_minSinceVersion = 0U;
        unsigned m_maxSinceVersion = 0U;
        unsigned m_deprecated = Protocol::notYetDeprecated();
    };

    Object* m_parent = nullptr;
    ReusableState m_rState;
};

} // namespace bbmp
