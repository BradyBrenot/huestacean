#include "common/room.h"

std::vector<Math::Box> DeviceInRoom::GetLightBoundingBoxes() const
{
	auto boxes = device->GetLightBoundingBoxes();

	//transform them
	for (auto& b : boxes)
	{
		b = transform.transformBox(b);
	}

	return boxes;
}