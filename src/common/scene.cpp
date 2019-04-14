#include "common/scene.h"

std::vector<Math::Box> DeviceInScene::GetLightBoundingBoxes() const
{
	auto boxes = device->GetLightBoundingBoxes();

	//transform them
	for (auto& b : boxes)
	{
		b = transform.transformBox(b);
	}

	return boxes;
}