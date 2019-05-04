#pragma once

#include <variant>

#include <QObject>
#include <QMap>
#include <QList>
#include <QVector3D>
#include <QQuaternion>

#include "hue/hue.h"
#include "razer/razer.h"
#include "razer/razerdevices.h"
#include "backend/backend.h"
#include "frontend/utility.h"

// Lightweight frontend Qt wrappers, because I'm very pointedly 
// avoiding Qt extensions in my 'backend' code. This keeps that
// side relatively clean and simple and C++y, while the horrible
// heavy Qt/QML-friendly side is contained over here.

// This is not namespaced, because Qt isn't. 
// Don't #include outside of frontend code.

///////////////////////////////////////////////////////////////////////////
// Qt-friendly version of Math::Transform
struct Transform
{
	Q_GADGET

	Q_PROPERTY(QVector3D location MEMBER location)
	Q_PROPERTY(QVector3D scale MEMBER scale)
	Q_PROPERTY(QQuaternion rotation MEMBER rotation)
public:

	Transform();
	Transform(const Transform& t);
	Transform(Math::Transform);
	Math::Transform ToMathTransform() const;

	QVector3D location;
	QVector3D scale;
	QQuaternion rotation;

	bool operator==(const Transform& b) const
	{
		return location == b.location && scale == b.scale && rotation == b.rotation;
	}
	bool operator!=(const Transform& b) const
	{
		return !(*this == b);
	}
};
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Devices
struct RazerDeviceInfo
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
	Q_PROPERTY(QString bridgeId MEMBER bridgeId)

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
	Q_PROPERTY(QString uniqueid MEMBER uniqueid)
	Q_PROPERTY(QVariant data READ GetData WRITE SetData)

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
	bool operator!=(const DeviceInfo& b) const
	{
		return !(*this == b);
	}

private:
	QVariant GetData();
	void SetData(QVariant& in);
};
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Device providers
struct BridgeInfo
{
	Q_GADGET
	Q_PROPERTY(QString id MEMBER id)
	Q_PROPERTY(QList<QVariant> devices READ GetDevices WRITE SetDevices)

public:

	QString id;

	QList<DeviceInfo> devices;

	bool operator==(const BridgeInfo& b) const
	{
		return devices == b.devices;
	}

private:
	QList<QVariant> GetDevices() { return makeVariantList(devices); }
	void SetDevices(QVariantList& in) { devices = fromVariantList<DeviceInfo>(in); }
};

struct HueInfo
{
	Q_GADGET
	Q_PROPERTY(QList<QVariant> bridges READ GetBridges WRITE SetBridges)

public:

	QList<BridgeInfo> bridges;

	bool operator==(const HueInfo& b) const
	{
		return bridges == b.bridges;
	}

private:
	QList<QVariant> GetBridges() { return makeVariantList(bridges); }
	void SetBridges(QVariantList& in) { bridges = fromVariantList<BridgeInfo>(in); }
};

struct RazerInfo
{
	Q_GADGET
	Q_PROPERTY(QList<QVariant> devices READ GetDevices WRITE SetDevices)

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

private:
	QList<QVariant> GetDevices() { return makeVariantList(devices); }
	void SetDevices(QVariantList& in) { devices = fromVariantList<DeviceInfo>(in); }
};
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Effects

struct EffectWithTransformInfo
{
	Q_GADGET
	Q_PROPERTY(Transform transform MEMBER transform)

public:

	Transform transform;

	EffectWithTransformInfo() : transform()
	{

	}
};

struct SinePulseEffectInfo : public EffectWithTransformInfo
{
	Q_GADGET
public:

	bool operator==(const SinePulseEffectInfo& b) const
	{
		return true;
	}
};

struct ConstantEffectInfo : public EffectWithTransformInfo
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
	Q_PROPERTY(QVariant data READ GetData WRITE SetData)

public:

	EffectVariant data;

	EffectInfo()
	{

	}

	EffectInfo(EffectVariant& inData)
		: data(inData)
	{

	}
	EffectInfo(EffectVariant&& inData)
		: data(inData)
	{

	}

	bool operator==(const EffectInfo& b) const
	{
		return data == b.data;
	}

private:
	QVariant GetData();
	void SetData(QVariant& in);
};
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Scenes
struct DeviceInSceneInfo
{
	Q_GADGET

	Q_PROPERTY(Transform transform MEMBER transform)
	Q_PROPERTY(DeviceInfo device MEMBER device)

public:
	Transform transform;
	DeviceInfo device;

	bool operator==(const DeviceInSceneInfo& b) const
	{
		return transform == b.transform && device == b.device;
	}
};

struct SceneInfo
{
	Q_GADGET

	Q_PROPERTY(QString name MEMBER name)
	Q_PROPERTY(QList<QVariant> devicesInScene READ GetDevicesInScene WRITE SetDevicesInScene)
	Q_PROPERTY(QList<QVariant> effects READ GetEffects WRITE SetEffects)

public:

	QString name;
	QList<DeviceInSceneInfo> devicesInScene;
	QList<EffectInfo> effects;

	bool operator==(const SceneInfo& b) const
	{
		return name == b.name && devicesInScene == b.devicesInScene && effects == b.effects;
	}

private:
	QList<QVariant> GetDevicesInScene() { return makeVariantList(devicesInScene); }
	void SetDevicesInScene(QVariantList& in) { devicesInScene = fromVariantList<DeviceInSceneInfo>(in); }
	QList<QVariant> GetEffects() { return makeVariantList(effects); }
	void SetEffects(QVariantList& in) { effects = fromVariantList<EffectInfo>(in); }
};
///////////////////////////////////////////////////////////////////////////

Q_DECLARE_METATYPE(QList<DeviceInfo>)

///////////////////////////////////////////////////////////////////////////
//Serialization
QDataStream& operator<<(QDataStream&, const Transform&);
QDataStream& operator>>(QDataStream&, Transform&);

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
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//Conversions between frontend/backend types
DeviceInfo				Device_BackendToFrontend(DevicePtr d);
DevicePtr				Device_FrontendToBackend(DeviceInfo d, const Backend& b);

EffectInfo				Effect_BackendToFrontend(const std::unique_ptr<Effect>& e);
std::unique_ptr<Effect> Effect_FrontendToBackend(EffectInfo e);

SceneInfo				Scene_BackendToFrontend(const Scene& s);
Scene					Scene_FrontendToBackend(SceneInfo s, const Backend& b);

BridgeInfo				Bridge_BackendToFrontend(std::shared_ptr<Hue::Bridge> b);
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Make C++ types from within QML without making all of the above QObjects
class TypeFactory : public QObject
{
	Q_OBJECT;

public:
	TypeFactory(QObject* parent = nullptr);
	virtual ~TypeFactory();

	Q_INVOKABLE QVariant NewScene() const;
	Q_INVOKABLE QVariant NewSinePulseEffect() const;
	Q_INVOKABLE QVariant NewConstantEffect() const;
};

///////////////////////////////////////////////////////////////////////////