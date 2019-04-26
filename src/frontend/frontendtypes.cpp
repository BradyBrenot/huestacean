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

QDataStream& operator<<(QDataStream& ds, const DeviceInfo& in)
{
	ds << static_cast<quint8>(in.type);

	switch (in.type)
	{
	case DeviceInfo::DeviceType::Hue:
		ds << in.hue;
		break;
	case DeviceInfo::DeviceType::Razer:
		ds << in.razer;
		break;
	}

	return ds;
}
QDataStream& operator>>(QDataStream& ds, DeviceInfo& out)
{
	quint8 iType;
	ds >> iType;

	out.type = static_cast<DeviceInfo::DeviceType>(iType);

	switch (out.type)
	{
	case DeviceInfo::DeviceType::Hue:
		ds >> out.hue;
		break;
	case DeviceInfo::DeviceType::Razer:
		ds >> out.razer;
		break;
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

QDataStream& operator<<(QDataStream& ds, const EffectInfo& in)
{
	ds << static_cast<quint8>(in.type);

	switch (in.type)
	{
	case EffectInfo::EffectType::SinePulse:
		ds << in.sine;
		break;
	case EffectInfo::EffectType::Constant:
		ds << in.constant;
		break;
	case EffectInfo::EffectType::None:
	default:
		break;
	}

	return ds;
}
QDataStream& operator>>(QDataStream& ds, EffectInfo& out)
{
	quint8 iType;
	ds >> iType;

	out.type = static_cast<EffectInfo::EffectType>(iType);

	switch (out.type)
	{
	case EffectInfo::EffectType::SinePulse:
		ds >> out.sine;
		break;
	case EffectInfo::EffectType::Constant:
		ds >> out.constant;
		break;
	case EffectInfo::EffectType::None:
	default:
		break;
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