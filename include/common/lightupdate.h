#pragma once

#include "transform.h"
#include "color.h"

#include <memory>
#include <vector>


struct LightUpdateParams
{
	std::vector<HsluvColor>::iterator colorsBegin;
	std::vector<HsluvColor>::iterator colorsEnd;
	bool colorsDirty;

	std::vector<Box>::iterator positionsBegin;
	std::vector<Box>::iterator positionsEnd;
	bool positionsDirty;

	std::vector<std::unique_ptr<Device> >::iterator devicesBegin;
	std::vector<std::unique_ptr<Device> >::iterator devicesEnd;
	bool devicesDirty;
};