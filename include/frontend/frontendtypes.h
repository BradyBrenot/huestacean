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

//AABB
struct Box
{
	Q_GADGET

	Q_PROPERTY(QVector3D center MEMBER center)
	Q_PROPERTY(QVector3D halfSize MEMBER halfSize)
public:

	QVector3D center;
	QVector3D halfSize;

	Box()
		: center(), halfSize()
	{
	}

	Box(QVector3D inCenter, QVector3D inHalfSize)
		: center(inCenter), halfSize(inHalfSize)
	{
	}

	bool operator==(const Box& b) const
	{
		return center == b.center && halfSize == b.halfSize;
	}
	bool operator!=(const Box& b) const
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
	Q_PROPERTY(QVector3D size MEMBER size) //TODO: do more with this / display individual lights?

	Q_PROPERTY(bool canRotate READ CanRotate)
	Q_PROPERTY(bool canScale READ CanScale)

public:

	QString uniqueid;
	QVector3D size;

	DeviceVariant data;

	DeviceInfo()
	{

	}

	DeviceInfo(const DeviceInfo& b) {
		uniqueid = b.uniqueid;
		data = b.data;
		size = b.size;
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

	bool CanRotate();
	bool CanScale();

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
	QList<QVariant> GetDevices() const { return makeVariantList(devices); }
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
	QList<QVariant> GetDevices() const { return makeVariantList(devices); }
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

class EffectInfo : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QVariant data READ GetData WRITE SetData NOTIFY DataChanged)

public:

	EffectVariant data;

	EffectInfo() : QObject()
	{

	}


	EffectInfo(const EffectInfo& b)
		: QObject(),
		data(b.data)
	{
	};

	EffectInfo(EffectVariant& inData)
		: QObject(), data(inData)
	{

	}
	EffectInfo(EffectVariant&& inData)
		: QObject(), data(inData)
	{

	}

	bool operator==(const EffectInfo& b) const
	{
		return data == b.data;
	}

signals:
	void DataChanged();

private:
	QVariant GetData();
	void SetData(QVariant& in);
};
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Scenes
class DeviceInSceneInfo : public QObject
{
	Q_OBJECT

	Q_PROPERTY(Transform transform MEMBER transform)
	Q_PROPERTY(DeviceInfo device MEMBER device)

public:
	Transform transform;
	DeviceInfo device;
	QString test;

	DeviceInSceneInfo(QObject* parent = nullptr) : QObject(parent)
	{
	};

	DeviceInSceneInfo(Transform inTransform, DeviceInfo inDevice)
		: QObject(), transform(inTransform), device(inDevice)
	{
	};

	DeviceInSceneInfo(const DeviceInSceneInfo& b)
		: QObject(),
		transform(b.transform),
		device(b.device)
	{
	};

	bool operator==(const DeviceInSceneInfo& b) const
	{
		return transform == b.transform && device == b.device;
	}
};

class SceneInfo : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString name MEMBER name NOTIFY nameChanged)
	Q_PROPERTY(QList<QObject*> devicesInScene READ GetDevicesInScene NOTIFY devicesInSceneChanged)
	Q_PROPERTY(QList<QObject*> effects READ GetEffects NOTIFY effectsChanged)
	Q_PROPERTY(QList<QVariant> devices READ GetDevices NOTIFY devicesInSceneChanged)
	Q_PROPERTY(QVector3D size MEMBER size NOTIFY sizeChanged)

public:

	SceneInfo(QObject* parent = nullptr) : QObject(parent),
		size(3,3,3)
	{
	};
	SceneInfo(const SceneInfo& b) 
		: name(b.name),
		size(b.size),
		m_DevicesInScene(b.m_DevicesInScene),
		m_EffectsList(b.m_EffectsList)
	{
	};
	virtual ~SceneInfo() {};

	QString name;
	QVector3D size;

	bool operator==(const SceneInfo& b) const
	{
		return name == b.name 
			&& size == b.size
			&& m_DevicesInScene == b.m_DevicesInScene 
			&& m_EffectsList == b.m_EffectsList;
	}

	SceneInfo& operator=(const SceneInfo& b)
	{
		name = b.name;
		size = b.size;
		m_DevicesInScene = b.m_DevicesInScene;
		m_EffectsList = b.m_EffectsList;
		return *this;
	}

	Q_INVOKABLE void AddDevice(QVariant Device);
	Q_INVOKABLE void RemoveDevice(QVariant Device);

signals:
	void devicesInSceneChanged();
	void effectsChanged();
	void nameChanged();
	void sizeChanged();

public:
	QList<QVariant> GetDevices() const;
	QList< QSharedPointer<DeviceInSceneInfo> > m_DevicesInScene;
	QList< QSharedPointer<EffectInfo> > m_EffectsList;

	QList<QObject*> GetDevicesInScene() { return makeQObjectList(m_DevicesInScene); }
	QList<QObject*> GetEffects() { return makeQObjectList(m_EffectsList); }
};
///////////////////////////////////////////////////////////////////////////

Q_DECLARE_METATYPE(Transform)
Q_DECLARE_METATYPE(QList<DeviceInfo>)
Q_DECLARE_METATYPE(SceneInfo)
Q_DECLARE_METATYPE(QList<SceneInfo>)

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

QSharedPointer<EffectInfo>	Effect_BackendToFrontend(const std::unique_ptr<Effect>& e);
std::unique_ptr<Effect>		Effect_FrontendToBackend(EffectInfo& e);

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

	Q_INVOKABLE void AddSinePulseEffect(QObject* OwnerSceneInfo) const;
	Q_INVOKABLE void AddConstantEffect(QObject* OwnerSceneInfo) const;
};

///////////////////////////////////////////////////////////////////////////