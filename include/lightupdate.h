#pragma once

#include "transform.h"
#include "color.h"

#include <vector>

struct DeviceData
{

};

struct LightUpdateParams
{
	std::vector<HsluvColor>::iterator colorsBegin;
	std::vector<HsluvColor>::iterator colorsEnd;
	bool colorsDirty;

	std::vector<Box>::iterator positionsBegin;
	std::vector<Box>::iterator positionsEnd;
	bool positionsDirty;

	std::vector<unique_ptr<struct DeviceData> >::iterator dataBegin;
	std::vector<unique_ptr<struct DeviceData>>::iterator dataEnd;
	bool dataDirty;
};