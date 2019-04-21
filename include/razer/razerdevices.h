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

	template<size_t ROWS, size_t COLUMNS, uint32_t LENGTH_CENTIMETRES, uint32_t WIDTH_CENTIMETRES>
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

			constexpr auto height = 10_cm;
			constexpr auto length = 1_cm * LENGTH_CENTIMETRES;
			constexpr auto width = 1_cm * WIDTH_CENTIMETRES;

			return std::vector<Math::Box>();
		}

	protected:
		virtual void UploadInternal(Chroma& Sdk) = 0;
		uint32_t data[ROWS][COLUMNS];
		
	};


	/*
	 *	Various concrete Razer devices
	 */
	class GenericKeyboard 
		: public ChromaDevice<ChromaSDK::Keyboard::MAX_ROW, ChromaSDK::Keyboard::MAX_COLUMN, 45, 15>
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