#pragma once

#include <limits>

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
        return m_minSinceVersion;
    }

    unsigned getMaxSinceVersion() const
    {
        return m_maxSinceVersion;
    }

    unsigned getDeprecated() const
    {
        return m_deprecated;
    }

    void setMaxSinceVersion(unsigned val)
    {
        m_maxSinceVersion = val;
    }

protected:
    virtual ObjKind objKindImpl() const = 0;

    void setMinSinceVersion(unsigned val)
    {
        m_minSinceVersion = val;
    }

    void setDeprecated(unsigned val)
    {
        m_deprecated = val;
    }

private:
    Object* m_parent = nullptr;
    unsigned m_minSinceVersion = 0U;
    unsigned m_maxSinceVersion = 0U;
    unsigned m_deprecated = std::numeric_limits<unsigned>::max();
};

} // namespace bbmp
