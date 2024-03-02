#include "SatelliteLib.h"

#include "Satellite.h"

namespace Sat
{

/// @brief Returns an ISatellite interface corresponding to p_vcsType.
/// @param p_vcsType - The VCS type of the ISatellite you wish to create.
std::unique_ptr<ISatellite> satellite_factory()
{
	std::unique_ptr<ISatellite> satellite = std::make_unique<Satellite>();
	return satellite;
}

}
