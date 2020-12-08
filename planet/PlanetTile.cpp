#include "PlanetTile.h"

bool PlanetTile::Init(const std::shared_ptr<PlanetTile>& pPlanetTile)
{
	if (!SelfRefBase<PlanetTile>::Init(pPlanetTile))
		return false;
	return true;
}

std::shared_ptr<PlanetTile> PlanetTile::Create()
{
	std::shared_ptr<PlanetTile> pPlanetTile = std::make_shared<PlanetTile>();
	if (pPlanetTile != nullptr && pPlanetTile->Init(pPlanetTile))
		return pPlanetTile;
	return nullptr;
}