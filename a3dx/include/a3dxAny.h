//-------------------------------------------------------------------------------------------------
// File : a3dxAny.h
// Desc : Any Data Type.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <typeinfo>


namespace a3dx {

///////////////////////////////////////////////////////////////////////////////////////////////////
// any class
///////////////////////////////////////////////////////////////////////////////////////////////////
class any
{
    //=============================================================================================
    // list of friend classes and methods.
    //=============================================================================================
    /* NOTHING */

public:
    //=============================================================================================
    // public variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // public methods.
    //=============================================================================================
    any()
    : m_item(nullptr)
    { /* DO_NOTHING */ }

    any(const any& value)
    : m_item( (value.m_item != nullptr) ? value.m_item->clone() : nullptr)
    { /* DO_NOTHING */ }

    template<typename T>
    any(const T& value)
    : m_item( new holder<T>(value) )
    { /* DO_NOTHING */ }

    ~any()
    { delete m_item; }

    any& swap(any& rhs) noexcept
    {
        std::swap(m_item, rhs.m_item);
        return *this;
    }

    any& operator = (const any& rhs)
    {
        any(rhs).swap(*this);
        return *this;
    }

    any& operator = (any&& rhs)
    {
        rhs.swap(*this);
        any().swap(rhs);
        return *this;
    }

    template<typename T>
    any& operator = (T&& rhs)
    {
        any(static_cast<T&&>(rhs)).swap(*this);
        return *this;
    }

    bool has_value() const noexcept
    { return m_item != nullptr; }

    void reset()
    { any().swap(*this); }

    const std::type_info& type() const
    { return (m_item != nullptr) ? m_item->type() : typeid(void); }

    size_t size() const
    { return (m_item != nullptr) ? m_item->size() : 0; }

    size_t align() const
    { return (m_item != nullptr) ? m_item->align() : 1; }

    template<typename T>
    T cast() const
    {
        return (m_item != nullptr) && (m_item->type() == typeid(T)) 
            ? static_cast<holder<T>*>(m_item)->m_value : T();
    }

    template<typename T>
    T unsafe_cast() const
    { return static_cast<holder<T>*>(m_item)->m_value; }

private:
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // holder_base structure
    ///////////////////////////////////////////////////////////////////////////////////////////////
    struct holder_base
    {
        virtual ~holder_base() {}
        virtual const std::type_info&   type () const = 0;
        virtual holder_base*            clone() const = 0;
        virtual size_t                  size () const = 0;
        virtual size_t                  align() const = 0;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // holder class
    ///////////////////////////////////////////////////////////////////////////////////////////////
    template<typename T>
    class holder : public holder_base
    {
    public:
        T m_value;

        holder(const T& value)
        : m_value(value)
        { /* DO_NOTHING */ }

        virtual ~holder()
        { /* DO_NOTHING */ }

        holder(T value) 
        : m_value(value) 
        { /* DO_NOTHING */ }

        const std::type_info& type () const override
        { return typeid(T); }

        holder_base* clone() const override
        { return new holder(m_value); }

        size_t size() const override
        { return sizeof(T); }

        size_t align() const override
        { return alignof(T); }

    private:
        holder& oeprator = (const holder&) = delete;
    };

    //=============================================================================================
    // private variables.
    //=============================================================================================
    holder_base*    m_item;

    //=============================================================================================
    // private methods.
    //=============================================================================================
    /* NOTHING */
};


} // namespace a3dx
