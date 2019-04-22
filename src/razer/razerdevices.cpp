#include "windows.h"
#include "razer/razerdevices.h"
#include "razer/chroma.h"

using namespace Razer;
using namespace Math;

uint32_t Razer::RgbFrom(RgbColor& c)
{
	return RGB(c.r * 255.0, c.g * 255.0, c.b * 255.0);
}

/*
 *	Razer devices that use a grid layout
 */

void GenericKeyboard::UploadInternal(Chroma& Sdk)
{
	Sdk.CreateKeyboardEffect(ChromaSDK::Keyboard::EFFECT_TYPE::CHROMA_CUSTOM,
		&data, 
		nullptr);
}

void GenericMouse::UploadInternal(Chroma& Sdk)
{
	Sdk.CreateMouseEffect(ChromaSDK::Mouse::EFFECT_TYPE::CHROMA_CUSTOM,
		&data,
		nullptr);
}

void GenericKeypad::UploadInternal(Chroma& Sdk)
{
	Sdk.CreateKeypadEffect(ChromaSDK::Keypad::EFFECT_TYPE::CHROMA_CUSTOM,
		&data,
		nullptr);
}

void GenericHeadset::UploadInternal(Chroma& Sdk)
{
	Sdk.CreateHeadsetEffect(ChromaSDK::Headset::EFFECT_TYPE::CHROMA_CUSTOM,
		&data,
		nullptr);
}

void GenericChromaLink::UploadInternal(Chroma& Sdk)
{
	Sdk.CreateChromaLinkEffect(ChromaSDK::ChromaLink::EFFECT_TYPE::CHROMA_CUSTOM,
		&data,
		nullptr);
}

/*
 * Devices with weird LED layouts
 */

void GenericMousepad::Upload(std::vector<Math::HsluvColor>::iterator& colorsItMutable,
	std::vector<DevicePtr>::iterator& devicesItMutable,
	Chroma& Sdk)
{
	for (int i = 0; i < size; ++i)
	{
		data[i] = RgbFrom(Math::RgbColor(*colorsItMutable++));
		devicesItMutable++;
	}

	Sdk.CreateMousematEffect(ChromaSDK::Mousepad::EFFECT_TYPE::CHROMA_CUSTOM,
		&data,
		nullptr);
}

std::vector<Math::Box> GenericMousepad::GetLightBoundingBoxes() const
{
	using namespace Math;

	auto boxes = std::vector<Math::Box>(ChromaSDK::Mousepad::MAX_LEDS);
	
	//Quoth the Razer SDK

	//First LED starts from top-right corner.
	//!< LED 0-4 right side, 5-9 bottom side, 10-14 left side.

	constexpr auto xSize = 26.0_cm;
	constexpr auto ySize = 36.0_cm;
	constexpr auto zSize = 2_cm;

	constexpr auto SideHalfSize = Vector3d{ xSize / 5.0 / 2.0, 3.0, zSize / 2.0 };
	constexpr auto BottomHalfSize = Vector3d{ 3.0, ySize / 5.0 / 2.0, zSize / 2.0 };

	constexpr distance xStart = (xSize / 2.0) + SideHalfSize.x;
	constexpr distance yStart = 1.0 * (ySize / 2.0) + BottomHalfSize.y;

	return {
		Box {Vector3d{xStart + 0 * SideHalfSize.x, ySize / 2.0, 0.0}, SideHalfSize},
		Box {Vector3d{xStart - 1 * SideHalfSize.x, ySize / 2.0, 0.0}, SideHalfSize},
		Box {Vector3d{xStart - 2 * SideHalfSize.x, ySize / 2.0, 0.0}, SideHalfSize},
		Box {Vector3d{xStart - 3 * SideHalfSize.x, ySize / 2.0, 0.0}, SideHalfSize},
		Box {Vector3d{xStart - 4 * SideHalfSize.x, ySize / 2.0, 0.0}, SideHalfSize},

		Box {Vector3d{xSize / 2.0, yStart + 0 * BottomHalfSize.y, 0.0}, SideHalfSize},
		Box {Vector3d{xSize / 2.0, yStart + 1 * BottomHalfSize.y, 0.0}, SideHalfSize},
		Box {Vector3d{xSize / 2.0, yStart + 2 * BottomHalfSize.y, 0.0}, SideHalfSize},
		Box {Vector3d{xSize / 2.0, yStart + 3 * BottomHalfSize.y, 0.0}, SideHalfSize},
		Box {Vector3d{xSize / 2.0, yStart + 4 * BottomHalfSize.y, 0.0}, SideHalfSize},

		Box {Vector3d{-xStart + 0 * SideHalfSize.x, ySize / -2.0, 0.0}, SideHalfSize},
		Box {Vector3d{-xStart + 1 * SideHalfSize.x, ySize / -2.0, 0.0}, SideHalfSize},
		Box {Vector3d{-xStart + 2 * SideHalfSize.x, ySize / -2.0, 0.0}, SideHalfSize},
		Box {Vector3d{-xStart + 3 * SideHalfSize.x, ySize / -2.0, 0.0}, SideHalfSize},
		Box {Vector3d{-xStart + 4 * SideHalfSize.x, ySize / -2.0, 0.0}, SideHalfSize}
	};
}