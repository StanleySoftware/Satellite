#pragma once
#include <Export.h>
#include <Error.h>
#include <ISatellite.h>
#include <stack>
#include <vector>

namespace nlohmann
{
    class json;
}

namespace Sat
{

class SATELLITELIB_API SatelliteBase : public ISatellite
{
public:

    virtual void load() = 0;
    virtual void unload() = 0;
    virtual CheckoutInfo checkout_info(char const* p_targetPath) = 0;


    /// @brief Receives a query string and returns the corresponding string from the lookup.
    /// @param p_query 
    /// @return An std::unique_ptr<char const[]> to the string returned by the lookup OR nullptr
    /// if an error is encountered.
    std::unique_ptr<char const[]> relay(char const * p_originPath, char const* p_query);
    /// @brief Returns the last error encountered by the object.
    Error get_last_error();
protected:
    Error resolve_expression(std::vector<std::string>& p_tokens, std::string p_originalExpression, nlohmann::json& p_currentDict, std::stack<std::string>& p_fileStack);

    Error m_lastError{};
};

}