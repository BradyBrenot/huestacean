#pragma once

#include "rep_frontend.h"

// Lightweight frontend Qt wrappers, because I'm being stubborn and
// trying to keep my "non-frontend" code from becoming "too Qt-y"

namespace FrontendTypes
{
	Q_NAMESPACE

	struct RazerDeviceInfo
	{
	};
	Q_DECLARE_METATYPE(RazerDeviceInfo);

	struct HueDeviceInfo
	{
		QString bridgeId;
	};
	Q_DECLARE_METATYPE(HueDeviceInfo);

	enum class DeviceType : uint8_t
	{
		Hue,
		Razer
	};
	Q_ENUM_NS(DeviceType)

	struct DeviceInfo
	{
		QString uniqueId;

		union
		{
			HueDeviceInfo hue;
			RazerDeviceInfo razer;
		};
	};

	struct SceneInfo
	{
		QMap<QString, DeviceInfo> Devices;
	};
	Q_DECLARE_METATYPE(SceneInfo);

	struct BridgeInfo
	{
		QString id;

		QList<HueDeviceInfo> devices;
	};
	Q_DECLARE_METATYPE(BridgeInfo);

	struct HueInfo
	{
		QList<BridgeInfo> bridges;
	};

	struct RazerInfo
	{
		QList<RazerDeviceInfo> devices;
	};

	Q_DECLARE_METATYPE(DeviceInfo);

	struct SinePulseEffectInfo
	{

	};
	Q_DECLARE_METATYPE(SinePulseEffectInfo);

	struct ConstantEffectInfo
	{

	};
	Q_DECLARE_METATYPE(ConstantEffectInfo);

	enum class EffectType : uint8_t
	{
		SinePulse,
		Constant
	};
	Q_ENUM_NS(EffectType)

	struct EffectInfo
	{
		enum class Type : uint8_t
		{
			SinePulse,
			Constant
		};

		union
		{
			SinePulseEffectInfo sine;
			ConstantEffectInfo constant;
		};
	};
	Q_DECLARE_METATYPE(EffectInfo);

}