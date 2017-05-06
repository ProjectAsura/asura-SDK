//-------------------------------------------------------------------------------------------------
// File : fndOptional.h
// Desc : Optional Module.
// Copyright(c) Project Asura. All right reserved.
//-------------------------------------------------------------------------------------------------
#pragma once

//-------------------------------------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------------------------------------
#include <type_traits>


namespace fnd {

///////////////////////////////////////////////////////////////////////////////////////////////////
// nullopt structure
///////////////////////////////////////////////////////////////////////////////////////////////////
struct nullopt {};


///////////////////////////////////////////////////////////////////////////////////////////////////
// optional_base class
///////////////////////////////////////////////////////////////////////////////////////////////////
template<size_t size>
class optional_base
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
    optional_base()
    : m_valid(false)
    , m_data ()
    { /* DO_NOTHING */ }

    optional_base(optional_base const& other)
    : m_valid(other.m_valid)
    , m_data (other.m_data)
    { /* DO_NOTHING */ }

    optional_base& operator = (optional_base const& value)
    {
        m_valid = value.m_valid;
        m_data  = value.m_data;
        return *this;
    }

    bool const valid() const
    { return m_valid; }

    bool const invalid() const
    { return !m_valid; }

private:
    //---------------------------------------------------------------------------------------------
    // Using Alias
    //---------------------------------------------------------------------------------------------
    using storage = std::aligned_storage<sizeof(T), alignof(T)>::type;

    //=============================================================================================
    // private variables.
    //=============================================================================================
    bool    m_valid;
    storage m_data;

    //=============================================================================================
    // private methods.
    //=============================================================================================
    /* NOTHING */
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// optional class
///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
class optional : public optional_base<sizeof(T)>
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
    optional()
    { /* DO_NOTHING */ }

    optional(T const& value)
    {
        construct(value);
        m_valid = true;
    }

    optional(std::nullptr_t const&)
    { /* DO_NOTHING */ }

    optional(optional const& other)
    {
        if (!other.m_valid)
        { return; }

        construct(*other);
        m_valid = true;
    }

    ~optional()
    {
        if (m_valid)
        { destruct(); }
    }

    optional& operator = (T const& value)
    {
        if (m_valid)
        { *get() = value; }
        else
        {
            construct(value);
            m_valid = true;
        }

        return *this;
    }

    optional& operator = (optional const& other)
    {
        if (m_valid)
        {
            m_valid = false;
            destruct();
        }

        if (other.m_valid)
        {
            construct(*other);
            m_valid = true;
        }
        return *this;
    }

    bool const operator == (optional const& other) const
    {
        if (invalid() && other.valid())
        { return true; }

        if (valid() ^ other.valid())
        { return false; }

        return (**this) == (*other);
    }

    bool const operator != (optional const& other) const
    { return !(*this == other); }

    bool const operator < (optional const& other) const
    {
        if (invalid() && other.invalid())
        { return false; }

        if (invalid())
        { return true; }

        if (other.invalid())
        { return false; }

        return (**this) < (*other);
    }

    bool const operator > (optional const& other) const
    {
        if (invalid() && other.invalid())
        { return false; }

        if (other.invalid())
        { return true; }

        if (invalid())
        { return false; }

        return (**this) > (*other);
    }

    T const& operator * () const
    { return *get(); }

    T& operator * ()
    { return *get(); }

    T const* const operator -> () const
    { return get(); }

    T* const operator -> ()
    { return get(); }

    void clear()
    {
        if (m_valid)
        {
            m_valid = false;
            destruct();
        }
    }

    bool const valid() const
    { return m_valid; }

    bool const invalid() const
    { return !m_valid; }

private:
    //=============================================================================================
    // private variables.
    //=============================================================================================
    /* NOTHING */

    //=============================================================================================
    // private methods.
    //=============================================================================================
    T const* const get() const
    { return static_cast<T const* const>(static_cast<void*>(&m_data)); }

    T const* get()
    { return static_cast<T const*>(static_cast<void*>(&m_data)); }

    void construct(const T& value)
    { new () T(value); }

    void destruct()
    { get()->~T(); }
};


} // namespace fnd
