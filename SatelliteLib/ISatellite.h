#pragma once
#include <string>
#include <memory>
#include "Error.h"
#include "CheckoutInfo.h"

namespace Sat
{

class ISatellite
{
public:
    virtual void load() = 0;
    virtual void unload() = 0;

    /// @brief Returns VCS CheckoutInfo for the path.
    /// @param p_targetPath - The path to check.
    virtual Error checkout_info(char const* p_targetPath, CheckoutInfo& p_out_checkoutInfo) = 0;
 
    virtual Error relay(char const * p_originPath, char const* p_query, CStringWrapper& p_out_string) = 0;

    virtual ~ISatellite(){};
};

}