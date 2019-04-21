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
	class ChromaDevice : public ChromaDeviceBase
	{
	public:
		ChromaDevice() : 
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
			for (auto x = 0; x < COLUMNS; ++x)
			{
				for (auto y = 0; y < ROWS; ++y)
				{
					constexpr distance cellXSize = xSize / static_cast<double>(COLUMNS);
					constexpr distance cellYSize = ySize / static_cast<double>(ROWS);
					constexpr distance cellZSize = zSize;

					constexpr distance xStart = -1.0 * (xSize / 2.0) + cellXSize / 2.0;
					constexpr distance yStart = -1.0 * (ySize / 2.0) + cellYSize / 2.0;

					boxes.push_back(Math::Box{ 
						Vector3d{xStart + cellXSize * x, yStart + cellYSize * y, 0.0},
						Vector3d{cellXSize / 2.0, cellYSize / 2.0, cellZSize / 2.0}});
				}
			}

			return boxes;
		}

	protected:
		virtual void UploadInternal(Chroma& Sdk) = 0;
		uint32_t data[ROWS][COLUMNS];
		
	};


	/*
	 *	Various concrete Razer devices
	 */
	class GenericKeyboard 
		: public ChromaDevice<ChromaSDK::Keyboard::MAX_ROW, ChromaSDK::Keyboard::MAX_COLUMN, 15, 45>
	{
	protected:
		virtual std::string GetUniqueIdInternal() const override;
		virtual void UploadInternal(Chroma& Sdk) override;
	};

#if 0
	class GenericMouse : public ChromaDevice
	{
	public:
		GenericKeyboard();
	protected:
		virtual std::string GetUniqueIdInternal() const;
	};

	class GenericMousemat : public ChromaDevice
	{
	public:
		GenericKeyboard();
	protected:
		virtual std::string GetUniqueIdInternal() const;
	};

	class GenericKeypad : public ChromaDevice
	{
	public:
		GenericKeyboard();
	protected:
		virtual std::string GetUniqueIdInternal() const;
	};

	class GenericHeadset : public ChromaDevice
	{
	public:
		GenericKeyboard();
	protected:
		virtual std::string GetUniqueIdInternal() const;
	};

	class GenericChromaLink : public ChromaDevice
	{
	public:
		GenericKeyboard();
	protected:
		virtual std::string GetUniqueIdInternal() const;
	};
#endif
};