#pragma once

#include <memory>

class Material;

enum class BloomPass
{
	PREFILTER,
	DOWNSAMPLE,
	UPSAMPLE,
	COUNT
};

enum class DOFPass
{
	PREFILTER,
	BLUR,
	POSTFILTER,
	COMBINE,
	COUNT
};

std::shared_ptr<Material> CreateDOFMaterial(DOFPass dofPass);
std::shared_ptr<Material> CreateBloomMaterial(BloomPass bloomPass, uint32_t iterIndex);
std::shared_ptr<Material> CreateCombineMaterial();
