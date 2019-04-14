#pragma once

#include "common/math.h"
#include "device.h"

#include <memory>
#include <vector>


struct LightUpdateParams
{
	//is std::span a better fit? It's a C++20 feature, so eh
	std::vector<Math::HsluvColor>::iterator colorsBegin;
	std::vector<Math::HsluvColor>::iterator colorsEnd;
	bool colorsDirty;

	std::vector<Math::Box>::iterator boundingBoxesBegin;
	std::vector<Math::Box>::iterator boundingBoxesEnd;
	bool boundingBoxesDirty;

	std::vector<DevicePtr>::iterator devicesBegin;
	std::vector<DevicePtr>::iterator devicesEnd;
	bool devicesDirty;
};