#pragma once

#include <memory>

namespace Sat
{

/// @brief A dumb RAII wrapper around
/// a C string.
class CStringWrapper
{
public:
    CStringWrapper() : m_buffer(nullptr) {}
    CStringWrapper(CStringWrapper const &) = delete;
    CStringWrapper& operator=(CStringWrapper const &) = delete;

    CStringWrapper(CStringWrapper && p_other) noexcept
        : m_buffer(std::move(p_other.m_buffer))
    {
        p_other.m_buffer = nullptr;
    }

    CStringWrapper& operator=(CStringWrapper&& p_other) noexcept
    {
        reset();
        m_buffer = std::move(p_other.m_buffer);
        p_other.m_buffer = nullptr;
        return *this;
    }

    CStringWrapper(char *&& p_buffer) : m_buffer(p_buffer){}
    CStringWrapper(size_t p_bufferSize) : m_buffer(static_cast<char*>(std::malloc(sizeof(char)*p_bufferSize))){}

    void reset()
    {
        if (m_buffer)
        {
            free(m_buffer);
            m_buffer = nullptr;
        }
    }

    void realloc(size_t p_bufferSize)
    {
        reset();
        m_buffer = static_cast<char*>(std::malloc(sizeof(char)*p_bufferSize));
    }

    char* get()
    {
        return m_buffer;
    }

    char const* get() const
    {
        return m_buffer;
    }

    char* release()
    {
        char * temp = m_buffer;
        m_buffer = nullptr;
        return temp;
    }

    ~CStringWrapper()
    {
        if (m_buffer)
        {
            free(m_buffer);
            m_buffer = nullptr;
        }
    }

private:

    char *  m_buffer;
};

}