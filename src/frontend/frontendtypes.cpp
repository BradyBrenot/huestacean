#include "frontend/frontendtypes.h"

#include <QDataStream>

QDataStream& operator<<(QDataStream& ds, const RazerDeviceInfo& in)
{
	return ds;
}
QDataStream& operator>>(QDataStream& ds, RazerDeviceInfo& out)
{
	return ds;
}

QDataStream& operator<<(QDataStream& ds, const HueDeviceInfo& in)
{
	ds << in.bridgeId;
	return ds;
}
QDataStream& operator>>(QDataStream& ds, HueDeviceInfo& out)
{
	ds >> out.bridgeId;
	return ds;
}

enum class DeviceType : quint8 {
	None,
	Hue,
	Razer
};

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;

QDataStream& operator<<(QDataStream& ds, const std::monostate& in)
{
	return ds;
}

QDataStream& operator<<(QDataStream& ds, const DeviceInfo& in)
{
	std::visit(
		overloaded{
			[&ds](std::monostate) { ds << static_cast<quint8>(DeviceType::None); },
			[&ds](HueDeviceInfo) { ds << static_cast<quint8>(DeviceType::Hue); },
			[&ds](RazerDeviceInfo) { ds << static_cast<quint8>(DeviceType::Razer); },
		},
	in.data);

	std::visit([&ds](const auto & d) {
		ds << d;
	}, in.data);

	return ds;
}
QDataStream& operator>>(QDataStream& ds, DeviceInfo& out)
{
	quint8 iType;
	ds >> iType;
	DeviceType type = static_cast<DeviceType>(iType);

	HueDeviceInfo hue;
	RazerDeviceInfo razer;

	switch (type)
	{
	case DeviceType::Hue:
		ds >> hue;
		out.data = hue;
		break;
	case DeviceType::Razer:
		ds >> razer;
		out.data = razer;
		break;
	default:
		out.data = std::monostate();
	}

	return ds;
}

QDataStream& operator<<(QDataStream& ds, const BridgeInfo& in)
{
	ds << in.id;
	ds << in.devices;
	return ds;
}
QDataStream& operator>>(QDataStream& ds, BridgeInfo& out)
{
	ds >> out.id;
	ds >> out.devices;
	return ds;
}

QDataStream& operator<<(QDataStream& ds, const HueInfo& in)
{
	ds << in.bridges;
	return ds;
}
QDataStream& operator>>(QDataStream& ds, HueInfo& out)
{
	ds >> out.bridges;
	return ds;
}

QDataStream& operator<<(QDataStream& ds, const RazerInfo& in)
{
	ds << in.devices;
	return ds;
}
QDataStream& operator>>(QDataStream& ds, RazerInfo& out)
{
	ds >> out.devices;
	return ds;
}

QDataStream& operator<<(QDataStream& ds, const SinePulseEffectInfo& in)
{
	return ds;
}
QDataStream& operator>>(QDataStream& ds, SinePulseEffectInfo& out)
{
	return ds;
}

QDataStream& operator<<(QDataStream& ds, const ConstantEffectInfo& in)
{
	return ds;
}
QDataStream& operator>>(QDataStream& ds, ConstantEffectInfo& out)
{
	return ds;
}

enum class EffectType : quint8
{
	None,
	SinePulse,
	Constant
};

QDataStream& operator<<(QDataStream& ds, const EffectInfo& in)
{
	std::visit(
		overloaded{
			[&ds](std::monostate) { ds << static_cast<quint8>(EffectType::None); },
			[&ds](SinePulseEffectInfo) { ds << static_cast<quint8>(EffectType::SinePulse); },
			[&ds](ConstantEffectInfo) { ds << static_cast<quint8>(EffectType::Constant); },
		},
		in.data);

	std::visit([&ds](const auto & d) {
		ds << d;
		}, in.data);

	return ds;
}
QDataStream& operator>>(QDataStream& ds, EffectInfo& out)
{
	quint8 iType;
	ds >> iType;
	EffectType type = static_cast<EffectType>(iType);

	SinePulseEffectInfo sine;
	ConstantEffectInfo constant;

	switch (type)
	{
	case EffectType::SinePulse:
		ds >> sine;
		out.data = sine;
		break;
	case EffectType::Constant:
		ds >> constant;
		out.data = constant;
		break;
	default:
		out.data = std::monostate();
	}

	return ds;
}

QDataStream& operator<<(QDataStream& ds, const SceneInfo& in)
{
	ds << in.devices;
	ds << in.effects;
	return ds;
}
QDataStream& operator>>(QDataStream& ds, SceneInfo& out)
{
	ds >> out.devices;
	ds >> out.effects;
	return ds;
}