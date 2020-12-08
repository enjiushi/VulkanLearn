#include "PlanetLODLayer.h"
#include "PlanetTile.h"

bool PlanetLODLayer::Init(const std::shared_ptr<PlanetLODLayer>& pPlanetLODLayer)
{
	if (!SelfRefBase<PlanetLODLayer>::Init(pPlanetLODLayer))
		return false;

	for (uint32_t i = 0; i < (uint32_t)TileAdjacency::COUNT; i++)
	{
		m_tiles[i] = PlanetTile::Create();
		m_tileAvailable[i] = false;
	}

	return true;
}

std::shared_ptr<PlanetLODLayer> PlanetLODLayer::Create()
{
	std::shared_ptr<PlanetLODLayer> pPlanetLODLayer = std::make_shared<PlanetLODLayer>();
	if (pPlanetLODLayer != nullptr && pPlanetLODLayer->Init(pPlanetLODLayer))
		return pPlanetLODLayer;
	return nullptr;
}