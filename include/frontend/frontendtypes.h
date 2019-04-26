#pragma once

#include <QObject>
#include <QMap>
#include <QList>

// Lightweight frontend Qt wrappers, because I'm being stubborn and
// trying to keep my "non-frontend" code from becoming "too Qt-y"

class RazerDeviceInfo
{
	Q_GADGET
public:

	bool operator==(const RazerDeviceInfo& b) const
	{
		return true;
	}
};

struct HueDeviceInfo
{
	Q_GADGET
public:

	QString bridgeId;

	bool operator==(const HueDeviceInfo& b) const
	{
		return bridgeId == b.bridgeId;
	}
};


struct DeviceInfo
{
	Q_GADGET
public:

	enum class DeviceType : uint8_t
	{
		None,
		Hue,
		Razer
	};
	Q_ENUM(DeviceType);

	DeviceType type;
	QString uniqueid;

	union
	{
		HueDeviceInfo hue;
		RazerDeviceInfo razer;
	};

	DeviceInfo() :
		type(DeviceType::None)
	{

	}

	DeviceInfo(const DeviceInfo& b) {
		type = b.type;
		uniqueid = b.uniqueid;

		switch (type)
		{
		case DeviceType::Hue:
			hue = b.hue;
			break;
		case DeviceType::Razer:
			razer = b.razer;
			break;
		case DeviceType::None:
		default:
			break;
		}
	}

	~DeviceInfo() {

	}

	bool operator==(const DeviceInfo& b) const
	{
		if (type != b.type
			|| uniqueid != b.uniqueid) {
			return false;
		}

		switch (type)
		{
		case DeviceType::Hue:
			return hue == b.hue;
			break;
		case DeviceType::Razer:
			return razer == b.razer;
			break;
		case DeviceType::None:
		default:
			return true;
			break;
		}

		return false;
	}
};

struct BridgeInfo
{
	Q_GADGET
public:

	QString id;

	QList<HueDeviceInfo> devices;

	bool operator==(const BridgeInfo& b) const
	{
		return devices == b.devices;
	}
};

struct HueInfo
{
	Q_GADGET
public:

	QList<BridgeInfo> bridges;

	bool operator==(const HueInfo& b) const
	{
		return bridges == b.bridges;
	}
};

struct RazerInfo
{
	Q_GADGET
public:

	QList<RazerDeviceInfo> devices;

	bool operator==(const RazerInfo& b) const
	{
		return devices == b.devices;
	}
};

struct SinePulseEffectInfo
{
	Q_GADGET
public:

	bool operator==(const SinePulseEffectInfo& b) const
	{
		return true;
	}
};

struct ConstantEffectInfo
{
	Q_GADGET
public:

	bool operator==(const ConstantEffectInfo& b) const
	{
		return true;
	}
};

struct EffectInfo
{
	Q_GADGET
public:

	enum class EffectType : uint8_t
	{
		None,
		SinePulse,
		Constant
	};
	Q_ENUM(EffectType)

	EffectType type;

	union
	{
		SinePulseEffectInfo sine;
		ConstantEffectInfo constant;
	};

	EffectInfo() :
		type(EffectType::None)
	{

	}

	bool operator==(const EffectInfo& b) const
	{
		if (type != b.type) {
			return false;
		}

		switch (type)
		{
		case EffectType::SinePulse:
			return sine == b.sine;
			break;
		case EffectType::Constant:
			return constant == b.constant;
			break;
		case EffectType::None:
		default:
			return true;
			break;
		}

		return false;
	}
};

struct SceneInfo
{
	Q_GADGET
public:

	QMap<QString, DeviceInfo> devices;
	QMap<QString, EffectInfo> effects;

	bool operator==(const SceneInfo& b) const
	{
		return devices == b.devices && effects == b.effects;
	}
};

Q_DECLARE_METATYPE(QList<DeviceInfo>)

QDataStream& operator<<(QDataStream&, const RazerDeviceInfo&);
QDataStream& operator>>(QDataStream&, RazerDeviceInfo&);

QDataStream& operator<<(QDataStream&, const HueDeviceInfo&);
QDataStream& operator>>(QDataStream&, HueDeviceInfo&);

QDataStream& operator<<(QDataStream&, const DeviceInfo&);
QDataStream& operator>>(QDataStream&, DeviceInfo&);

QDataStream& operator<<(QDataStream&, const BridgeInfo&);
QDataStream& operator>>(QDataStream&, BridgeInfo&);

QDataStream& operator<<(QDataStream&, const HueInfo&);
QDataStream& operator>>(QDataStream&, HueInfo&);

QDataStream& operator<<(QDataStream&, const RazerInfo&);
QDataStream& operator>>(QDataStream&, RazerInfo&);

QDataStream& operator<<(QDataStream&, const SinePulseEffectInfo&);
QDataStream& operator>>(QDataStream&, SinePulseEffectInfo&);

QDataStream& operator<<(QDataStream&, const ConstantEffectInfo&);
QDataStream& operator>>(QDataStream&, ConstantEffectInfo&);

QDataStream& operator<<(QDataStream&, const EffectInfo&);
QDataStream& operator>>(QDataStream&, EffectInfo&);

QDataStream& operator<<(QDataStream&, const SceneInfo&);
QDataStream& operator>>(QDataStream&, SceneInfo&);