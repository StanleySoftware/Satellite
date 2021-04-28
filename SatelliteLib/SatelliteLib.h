#pragma once
#include "Export.h"
#include "ISatellite.h"

namespace Sat
{

enum class VCSType : unsigned int
{
	DEFAULT = 0,
	GIT
};

/// @brief Returns an ISatellite interface corresponding to p_vcsType.
/// @param p_vcsType - The VCS type of the ISatellite you wish to create.
SATELLITELIB_API std::unique_ptr<ISatellite> satellite_factory(VCSType p_vcsType);

}
