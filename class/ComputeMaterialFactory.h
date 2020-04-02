#pragma once

#include <memory>
#include <vector>

class Material;
class Image;

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

typedef struct _GaussianBlurParams
{
	int		direction = 0;
	float	scale = 1.0f;
	float	strength = 1.0f;
}GaussianBlurParams;

std::shared_ptr<Material> CreateGaussianBlurMaterial(const std::vector<std::shared_ptr<Image>>& inputImages, const std::vector<std::shared_ptr<Image>>& outputImages, const GaussianBlurParams& params);
std::shared_ptr<Material> CreateDeferredShadingMaterial();
std::shared_ptr<Material> CreateTemporalResolveMaterial(uint32_t pingpong);
std::shared_ptr<Material> CreateDOFMaterial(DOFPass dofPass);
std::shared_ptr<Material> CreateBloomMaterial(BloomPass bloomPass, uint32_t iterIndex);
std::shared_ptr<Material> CreateCombineMaterial();
