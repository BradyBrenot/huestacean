#pragma once

#include <variant>

#include <QObject>
#include <QMap>
#include <QList>

#include "hue/hue.h"
#include "razer/razer.h"
#include "razer/razerdevices.h"
#include "backend/backend.h"

// Lightweight frontend Qt wrappers, because I'm very pointedly 
// avoiding Qt extensions in my 'backend' code.

// This is not namespaced, because Qt isn't. 
// Don't #include outside of frontend code.

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

typedef std::variant< std::monostate, HueDeviceInfo, RazerDeviceInfo > DeviceVariant;

struct DeviceInfo
{
	Q_GADGET
public:

	QString uniqueid;

	DeviceVariant data;

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

	QList<DeviceInfo> devices;

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

	QList<DeviceInfo> devices;

	bool operator==(const RazerInfo& b) const
	{
		return devices == b.devices;
	}
	bool operator!=(const RazerInfo& b) const
	{
		return devices != b.devices;
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

typedef std::variant<std::monostate, SinePulseEffectInfo, ConstantEffectInfo> EffectVariant;

struct EffectInfo
{
	Q_GADGET
public:

	EffectVariant data;

	EffectInfo()
	{

	}

	bool operator==(const EffectInfo& b) const
	{
		return data == b.data;
	}
};

struct DeviceInSceneInfo
{
	Math::Transform transform;
	DeviceInfo device;

	bool operator==(const DeviceInSceneInfo& b) const
	{
		return transform == b.transform && device == b.device;
	}
};

struct SceneInfo
{
	Q_GADGET
public:

	QString name;
	QList<DeviceInSceneInfo> devicesInScene;
	QList<EffectInfo> effects;

	bool operator==(const SceneInfo& b) const
	{
		return name == b.name && devicesInScene == b.devicesInScene && effects == b.effects;
	}
};

Q_DECLARE_METATYPE(QList<DeviceInfo>)


//Serialization
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

QDataStream& operator<<(QDataStream&, const DeviceInSceneInfo&);
QDataStream& operator>>(QDataStream&, DeviceInSceneInfo&);

QDataStream& operator<<(QDataStream&, const SceneInfo&);
QDataStream& operator>>(QDataStream&, SceneInfo&);


//Conversions
DeviceInfo				Device_BackendToFrontend(DevicePtr d);
DevicePtr				Device_FrontendToBackend(DeviceInfo d, const Backend& b);

EffectInfo				Effect_BackendToFrontend(const std::unique_ptr<Effect>& e);
std::unique_ptr<Effect> Effect_FrontendToBackend(EffectInfo e);

SceneInfo				Scene_BackendToFrontend(const Scene& s);
Scene					Scene_FrontendToBackend(SceneInfo s, const Backend& b);

BridgeInfo				Bridge_BackendToFrontend(std::shared_ptr<Hue::Bridge> b);