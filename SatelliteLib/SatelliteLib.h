#pragma once
#include "ISatellite.h"

namespace Sat
{

enum class VCSType : unsigned int
{
	GIT = 0
};

/// @brief Returns an ISatellite interface corresponding to p_vcsType.
/// @param p_vcsType - The VCS type of the ISatellite you wish to create.
std::unique_ptr<ISatellite> satellite_factory(VCSType p_vcsType);

}
