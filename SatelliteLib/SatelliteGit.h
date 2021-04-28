#pragma once

#include "Export.h"
#include "Error.h"
#include "SatelliteBase.h"
#include "ISatellite.h"

namespace Sat
{

class SATELLITELIB_API SatelliteGit : public SatelliteBase
{
public:
    virtual void load() override;
    virtual void unload() override;
    virtual CheckoutInfo checkout_info(char const* p_targetPath) override;
};

}