#pragma once

#include <memory>

class Material;

enum BloomPass
{
	PREFILTER,
	DOWNSAMPLE,
	UPSAMPLE,
	COUNT
};

std::shared_ptr<Material> CreateBloomMaterial(BloomPass bloomPass, uint32_t iterIndex);
std::shared_ptr<Material> CreateCombineMaterial();
