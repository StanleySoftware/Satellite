#pragma once
#include "ISatellite.h"

namespace Sat
{

/// @brief Returns an ISatellite interface corresponding to p_vcsType.
std::unique_ptr<ISatellite> satellite_factory();

}
