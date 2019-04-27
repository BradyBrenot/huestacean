#pragma once

#include <variant>

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

	QString uniqueid;

	std::variant< std::monostate, HueDeviceInfo, RazerDeviceInfo > data;


	DeviceInfo()
	{

	}

	DeviceInfo(const DeviceInfo& b) {
		uniqueid = b.uniqueid;
		data = b.data;
	}

	~DeviceInfo() {

	}

	bool operator==(const DeviceInfo& b) const
	{
		return uniqueid == b.uniqueid;
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

	std::variant<std::monostate, SinePulseEffectInfo, ConstantEffectInfo> data;

	EffectInfo()
	{

	}

	bool operator==(const EffectInfo& b) const
	{
		return data == b.data;
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