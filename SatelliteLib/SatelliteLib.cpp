// SatelliteLib.cpp : Defines the exported functions for the DLL.
//

#include "framework.h"
#include "SatelliteLib.h"
#include "SatelliteGit.h"

namespace Sat
{

/// @brief Returns an ISatellite interface corresponding to p_vcsType.
/// @param p_vcsType - The VCS type of the ISatellite you wish to create.
std::unique_ptr<ISatellite> satellite_factory(VCSType p_vcsType)
{
	switch (p_vcsType)
	{
	case VCSType::DEFAULT:
	case VCSType::GIT:
	{
		std::unique_ptr<ISatellite> satellite = std::make_unique<SatelliteGit>();
		return satellite;
	}
	default:
		return nullptr;
	}
}

}
