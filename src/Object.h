#pragma once

namespace bbmp
{

class Object
{
public:
    enum class ObjKind
    {
        SimpleField,
        Bitfield,
        Bundle,
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

protected:
    virtual ObjKind objKindImpl() const = 0;

private:
    Object* m_parent = nullptr;
};

} // namespace bbmp
