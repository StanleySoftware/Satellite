#pragma once

#include "Error.h"
#include "SatelliteBase.h"
#include "ISatellite.h"

namespace Sat
{

class SatelliteGit : public SatelliteBase
{
public:
    virtual void load() override;
    virtual void unload() override;
    virtual Error checkout_info(char const* p_targetPath, CheckoutInfo& p_out_checkoutInfo) override;

    virtual ~SatelliteGit(){}
};

}