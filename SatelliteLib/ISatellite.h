#pragma once
#include <string>
#include <memory>
#include "Export.h"
#include "Error.h"
#include "CheckoutInfo.h"

namespace Sat
{

struct SATELLITELIB_API ISatellite
{
    virtual void load() = 0;
    virtual void unload() = 0;

    /// @brief Returns VCS CheckoutInfo for the path.
    /// @param p_targetPath - The path to check.
    virtual CheckoutInfo checkout_info(char const* p_targetPath) = 0;

    /// @brief Receives a query string and returns the corresponding string from the lookup.
    /// @param p_query 
    /// @return An std::unique_ptr<char const[]> to the string returned by the lookup OR nullptr
    /// if an error is encountered.
    virtual std::unique_ptr<char const[]> relay(char const * p_originPath, char const* p_query) = 0;

    /// @brief Returns the last error encountered by the object.
    virtual Error get_last_error() = 0;
};

}