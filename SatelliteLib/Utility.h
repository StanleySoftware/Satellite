#pragma once
#include <string>
#include <memory>

namespace Sat
{

template<typename ... Args>
std::string str_format( std::string const & format, Args ... args )
{
    // Extra space for terminating character
    int size_s = snprintf( nullptr, 0, format.c_str(), args ... ) + 1;
    if( size_s <= 0 )
    { 
        std::string err_msg = "snprintf failed to format the following string: '" + format + "'";
        throw std::runtime_error( err_msg); 
    }
    auto size = static_cast<size_t>( size_s );
    auto buf = std::make_unique<char[]>( size );
    snprintf( buf.get(), size, format.c_str(), args ... );
    // Cut the terminating character
    return std::string( buf.get(), buf.get() + size - 1 );
}

}