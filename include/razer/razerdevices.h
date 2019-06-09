#pragma once

#include "common/math.h"
#include "common/device.h"
#include "razer/razer.h"
#include "razer/chroma.h"

namespace Razer
{
	//Convert floating point color to 32-bit int
	uint32_t RgbFrom(Math::RgbColor& c);

	class ChromaDeviceBase : public Device
	{
	public:
		ChromaDeviceBase(size_t inSize)
			: Device(ProviderType::Razer),
			size(inSize)
		{

		}

		virtual void Upload(std::vector<Math::HsluvColor>::iterator& colorsItMutable,
			std::vector<DevicePtr>::iterator& devicesItMutable,
			Chroma& Sdk) = 0;

		size_t size;
	};

	template<size_t ROWS, size_t COLUMNS, uint32_t X_SIZE_CENTIMETRES, uint32_t Y_SIZE_CENTIMETRES>
	class ChromaDeviceGrid : public ChromaDeviceBase
	{
	public:
		ChromaDeviceGrid() :
			ChromaDeviceBase(ROWS* COLUMNS)
		{

		}

		virtual void Upload(std::vector<Math::HsluvColor>::iterator& colorsItMutable,
			std::vector<DevicePtr>::iterator& devicesItMutable,
			Chroma& Sdk) override
		{
			for (int i = 0; i < size; ++i)
			{
				(reinterpret_cast<uint32_t*>(&data))[i] = RgbFrom(Math::RgbColor(*colorsItMutable++));
				devicesItMutable++;
			}

			UploadInternal(Sdk);
		}

		virtual std::vector<Math::Box> GetLightBoundingBoxes() const override
		{
			using namespace Math;

			constexpr distance xSize = 1_cm * X_SIZE_CENTIMETRES;
			constexpr distance ySize = 1_cm * Y_SIZE_CENTIMETRES;
			constexpr distance zSize = 10_cm;

			auto boxes = std::vector<Math::Box>(ROWS * COLUMNS);
			for (auto x = 0; x < ROWS; ++x)
			{
				for (auto y = 0; y < COLUMNS; ++y)
				{
					constexpr distance cellXSize = xSize / static_cast<double>(ROWS);
					constexpr distance cellYSize = ySize / static_cast<double>(COLUMNS);
					constexpr distance cellZSize = zSize;

					constexpr distance xStart = -1.0 * (xSize / 2.0) + cellXSize / 2.0;
					constexpr distance yStart = -1.0 * (ySize / 2.0) + cellYSize / 2.0;

					boxes[x * COLUMNS + y] = Math::Box{ 
						Vector3d{xStart + cellXSize * x, yStart + cellYSize * y, 0.0},
						Vector3d{cellXSize / 2.0, cellYSize / 2.0, cellZSize / 2.0}};
				}
			}

			return boxes;
		}

	protected:
		virtual void UploadInternal(Chroma& Sdk) = 0;
		uint32_t data[ROWS][COLUMNS];
		
	};


	/*
	 *	Razer devices that use a grid layout
	 */

	class GenericKeyboard 
		: public ChromaDeviceGrid<ChromaSDK::Keyboard::MAX_ROW, ChromaSDK::Keyboard::MAX_COLUMN, 15, 45>
	{
	protected:
		virtual std::string GetUniqueIdInternal() const override { return "GenericKeyboard"; }
		virtual void UploadInternal(Chroma& Sdk) override;
	};

	class GenericMouse
		: public ChromaDeviceGrid<ChromaSDK::Mouse::MAX_ROW, ChromaSDK::Mouse::MAX_COLUMN, 15, 7>
	{
	protected:
		virtual std::string GetUniqueIdInternal() const override { return "GenericMouse"; };
		virtual void UploadInternal(Chroma& Sdk) override;
	};

	class GenericKeypad
		: public ChromaDeviceGrid<ChromaSDK::Keypad::MAX_ROW, ChromaSDK::Keypad::MAX_COLUMN, 15, 7>
	{
	protected:
		virtual std::string GetUniqueIdInternal() const override { return "GenericKeypad"; };
		virtual void UploadInternal(Chroma& Sdk) override;
	};

	class GenericHeadset
		: public ChromaDeviceGrid<1, ChromaSDK::Headset::MAX_LEDS, 15, 35>
	{
	protected:
		virtual std::string GetUniqueIdInternal() const override { return "GenericHeadset"; };
		virtual void UploadInternal(Chroma& Sdk) override;
	};

	class GenericChromaLink
		: public ChromaDeviceGrid<1, ChromaSDK::ChromaLink::MAX_LEDS, 30, 60>
	{
	protected:
		virtual std::string GetUniqueIdInternal() const override { return "GenericChromaLink"; };
		virtual void UploadInternal(Chroma& Sdk) override;
	};

	/* 
	 * Devices with weird LED layouts
	 */

	class GenericMousepad : public ChromaDeviceBase
	{
	public:
		GenericMousepad() :
			ChromaDeviceBase(ChromaSDK::Mousepad::MAX_LEDS)
		{

		}

		virtual std::vector<Math::Box> GetLightBoundingBoxes() const override;
	protected:
		virtual std::string GetUniqueIdInternal() const override { return "GenericMousepad"; };
		virtual void Upload(std::vector<Math::HsluvColor>::iterator& colorsItMutable,
			std::vector<DevicePtr>::iterator& devicesItMutable,
			Chroma& Sdk) override;

		uint32_t data[ChromaSDK::Mousepad::MAX_LEDS];
	};
};