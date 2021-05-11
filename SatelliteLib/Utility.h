#pragma once
#include <string>
#include <memory>
#include <stdexcept>
#include <locale>
#include <codecvt>
#include <vector>

#if defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    // Windows Header Files
    #include <windows.h>
    #include <shellapi.h>
#endif

#include <Error.h>
#include <CStringWrapper.h>

namespace Sat
{

template<typename ... Args>
inline CStringWrapper str_format( std::string const & format, Args ... args )
{
    // Extra space for terminating character
    int size_s = snprintf( nullptr, 0, format.c_str(), args ... ) + 1;
    if( size_s <= 0 )
    { 
        std::string err_msg = "snprintf failed to format the following string: '" + format + "'";
        throw std::runtime_error( err_msg); 
    }
    auto size = static_cast<size_t>( size_s );
    CStringWrapper buff(size);
    snprintf( buff.get(), size, format.c_str(), args ... );
    return buff;
}

inline std::vector<std::string> str_split(std::string const & p_terms, std::string p_delimiter)
{
    std::vector<std::string> tokens{};
    size_t offset{};
    size_t substrEnd{};
    size_t const termsLength = p_terms.length();
    size_t const delimLength = p_delimiter.length();

    do
    {
        substrEnd = p_terms.find(p_delimiter, offset);
        if(offset < termsLength)
        {
            size_t count = substrEnd - offset;
            tokens.push_back(p_terms.substr(offset, count));
            offset = substrEnd + delimLength;
        }
    }
    while(substrEnd != std::string::npos);

    return tokens;
}

#if defined(_WIN32)
inline std::string GetWinErrorStringFromID(DWORD p_errorMessageID)
{
    //Get the error message ID, if any.
    if(p_errorMessageID == 0) {
        return std::string(); //No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, 
        p_errorMessageID, 
        MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
        (LPSTR)&messageBuffer,
        0,
        NULL);

    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
}
#endif

/// @brief Piggyback on the OS method for separating args.
/// @param p_args - A string containing all the args.
/// @param p_out_args - An output vector for the separated args.
inline Error separate_arg_string(std::string const & p_args, std::vector<std::string>& p_out_args)
{
    //Avoid CommandLineToArgvW if p_args is an empty string.
    //Otherwise it will return this executables name, which we don't want.
    if (p_args.empty())
    {
        p_out_args.resize(0u);
        return Error{};
    }

    int argc{};
    wchar_t** argv{};
#if defined(_WIN32)

#pragma warning( push )
#pragma warning( disable: 4996 )
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wideArgString = converter.from_bytes(p_args);
#pragma warning( pop )
    argv = CommandLineToArgvW(wideArgString.c_str(), &argc);
    if (!argv || argc < 0)
    {
        std::string errString = GetWinErrorStringFromID(GetLastError());
        Error err{};
        err.m_errorType = ErrorType::SEPARATEARGFAILURE;
        err.m_errorMessage = str_format("Satellite: separate_arg_string failed to separate arg string \'%s\'. Additional Error Info: %s", p_args.c_str(), errString.c_str());
        return err;
    }

 #else
    #error Not implemented
 #endif

    p_out_args.resize(argc);

    for (size_t i = 0u; i < p_out_args.size(); i++)
    {
#pragma warning( push )
#pragma warning( disable: 4996 )
        std::wstring wideArg(*(argv + i));
        p_out_args[i] = converter.to_bytes(wideArg);
#pragma warning( pop )
    }  

    return Error{};
}

}