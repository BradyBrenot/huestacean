#include "frontend/frontendtypes.h"
#include "effects/effects.h"
#include "hue/bridge.h"

#include <algorithm>

#include <QDataStream>
#include <QQuaternion>

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;

///////////////////////////////////////////////////////////////////////////
// Transform
Transform::Transform()
	: location(), scale(), rotation()
{

}
Transform::Transform(const Transform& t)
	: location(t.location), scale(t.scale), rotation(t.rotation)
{

}
Transform::Transform(Math::Transform t)
	: location(t.location.x, t.location.y, t.location.z), 
	scale(t.scale.x, t.scale.y, t.scale.z), 
	rotation(QQuaternion::fromEulerAngles(t.rotation.pitch, t.rotation.yaw, t.rotation.roll))
{

}
Math::Transform Transform::ToMathTransform() const
{
	auto rotAngles = rotation.toEulerAngles();

	return Math::Transform{
		Math::Vector3d{location.x(), location.y(), location.z()},
		Math::Vector3d{scale.x(), scale.y(), scale.z()},
		Math::Rotator{rotAngles.x(), rotAngles.y(), rotAngles.z()}
	};
}
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Devices
QVariant DeviceInfo::GetData()
{
	QVariant out;

	std::visit(
		overloaded{
			[&out](std::monostate) { },
			[&out](HueDeviceInfo hue) { out.setValue(hue); },
			[&out](RazerDeviceInfo razer) { out.setValue(razer); },
		},
		data);

	return out;
}
void DeviceInfo::SetData(QVariant& in)
{
	if (in.canConvert<HueDeviceInfo>())
	{
		data = in.value<HueDeviceInfo>();
	}
	else if (in.canConvert<RazerDeviceInfo>())
	{
		data = in.value<RazerDeviceInfo>();
	}
	else
	{
		data = std::monostate{};
	}
}
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Effects
QVariant EffectInfo::GetData()
{
	QVariant out;

	std::visit(
		overloaded{
			[&out](std::monostate) {},
			[&out](SinePulseEffectInfo info) { out.setValue(info); },
			[&out](ConstantEffectInfo info) { out.setValue(info); },
		},
		data);

	return out;
}
void EffectInfo::SetData(QVariant& in)
{
	if (in.canConvert<SinePulseEffectInfo>())
	{
		data = in.value<SinePulseEffectInfo>();
	}
	else if (in.canConvert<ConstantEffectInfo>())
	{
		data = in.value<ConstantEffectInfo>();
	}
	else
	{
		data = std::monostate{};
	}
}
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
// Serialization
QDataStream& operator<<(QDataStream& ds, const Transform& in)
{
	ds << in.location;
	ds << in.scale;
	ds << in.rotation;
	return ds;
}
QDataStream& operator>>(QDataStream& ds, Transform& out)
{
	ds >> out.location;
	ds >> out.scale;
	ds >> out.rotation;
	return ds;
}

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

QDataStream& operator<<(QDataStream& ds, const std::monostate& in)
{
	return ds;
}

QDataStream& operator<<(QDataStream& ds, const DeviceInfo& in)
{
	ds << in.uniqueid;
	ds << in.size;

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
	ds >> out.uniqueid;
	ds >> out.size;

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

QDataStream& operator<<(QDataStream& ds, const Math::Vector3d& in)
{
	ds << in.x;
	ds << in.y;
	ds << in.z;
	return ds;
}
QDataStream& operator>>(QDataStream& ds, Math::Vector3d& out)
{
	ds >> out.x;
	ds >> out.y;
	ds >> out.z;
	return ds;
}

QDataStream& operator<<(QDataStream& ds, const Math::Rotator& in)
{
	ds << in.pitch;
	ds << in.yaw;
	ds << in.roll;
	return ds;
}
QDataStream& operator>>(QDataStream& ds, Math::Rotator& out)
{
	ds >> out.pitch;
	ds >> out.yaw;
	ds >> out.roll;
	return ds;
}

QDataStream& operator<<(QDataStream& ds, const Math::Transform& in)
{
	ds << in.location;
	ds << in.scale;
	ds << in.rotation;
	return ds;
}
QDataStream& operator>>(QDataStream& ds, Math::Transform& out)
{
	ds >> out.location;
	ds >> out.scale;
	ds >> out.rotation;
	return ds;
}

QDataStream& operator<<(QDataStream& ds, const DeviceInSceneInfo& in)
{
	ds << in.transform;
	ds << in.device;
	return ds;
}
QDataStream& operator>>(QDataStream& ds, DeviceInSceneInfo& out)
{
	ds >> out.transform;
	ds >> out.device;
	return ds;
}

QDataStream& operator<<(QDataStream& ds, const SceneInfo& in)
{
	ds << in.name;
	ds << in.devicesInScene;
	ds << in.effects;
	return ds;
}
QDataStream& operator>>(QDataStream& ds, SceneInfo& out)
{
	ds >> out.name;
	ds >> out.devicesInScene;
	ds >> out.effects;
	return ds;
}
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Conversions between frontend/backend types
DeviceInfo Device_BackendToFrontend(DevicePtr d)
{
	auto out = DeviceInfo();
	out.uniqueid = d->GetUniqueId().c_str();

	if (auto * chroma = dynamic_cast<Razer::ChromaDeviceBase*>(d.get()))
	{
		out.data = RazerDeviceInfo{};
	}
	else if (auto * hue = dynamic_cast<Hue::Light*>(d.get()))
	{
		out.data = HueDeviceInfo{};
		std::get<HueDeviceInfo>(out.data).bridgeId = hue->bridgeid.c_str();
	}

	out.size = [&]() {
		auto boundingBoxes = d->GetLightBoundingBoxes();

		Math::Vector3d boundsMin, boundsMax;
		for (const auto& box : boundingBoxes) {
			boundsMin.x = std::min(boundsMin.x, box.center.x - box.halfSize.x);
			boundsMax.x = std::max(boundsMin.x, box.center.x + box.halfSize.x);

			boundsMin.y = std::min(boundsMin.y, box.center.y - box.halfSize.y);
			boundsMax.y = std::max(boundsMin.y, box.center.y + box.halfSize.y);

			boundsMin.z = std::min(boundsMin.z, box.center.z - box.halfSize.z);
			boundsMax.z = std::max(boundsMin.z, box.center.z + box.halfSize.z);
		}

		return QVector3D{static_cast<float>(boundsMax.x - boundsMin.x),
			static_cast<float>(boundsMax.y - boundsMin.y),
			static_cast<float>(boundsMax.z - boundsMin.z)};
	}();

	return out;
}

DevicePtr Device_FrontendToBackend(DeviceInfo d, const Backend& b)
{
	return b.GetDeviceFromUniqueId(d.uniqueid.toStdString());
}

EffectInfo Effect_BackendToFrontend(const std::unique_ptr<Effect>& e)
{
	auto out = EffectInfo();

	if (auto * sine = dynamic_cast<SinePulseEffect*>(e.get()))
	{
		out.data = SinePulseEffectInfo{};
		std::get<SinePulseEffectInfo>(out.data).transform = sine->transform;
	}
	else if (auto * constant = dynamic_cast<ConstantEffect*>(e.get()))
	{
		out.data = ConstantEffectInfo{};
		std::get<ConstantEffectInfo>(out.data).transform = constant->transform;
	}

	return out;
}

std::unique_ptr<Effect> Effect_FrontendToBackend(EffectInfo e)
{
	if (auto sinePtr = std::get_if<SinePulseEffectInfo>(&e.data))
	{
		return std::make_unique<SinePulseEffect>();
	}
	else if (auto constantPtr = std::get_if<ConstantEffectInfo>(&e.data))
	{
		return std::make_unique<ConstantEffect>();
	}

	return nullptr;
}

SceneInfo Scene_BackendToFrontend(const Scene& s)
{
	SceneInfo frontendScene;

	frontendScene.name = s.name.c_str();

	for (const auto& dis : s.devices)
	{
		DeviceInSceneInfo frontendDis;
		frontendDis.device = Device_BackendToFrontend(dis.device);
		frontendDis.transform = dis.transform;
		frontendScene.devicesInScene.push_back(frontendDis);
	}

	for (const auto& e : s.effects)
	{
		frontendScene.effects.push_back(Effect_BackendToFrontend(e));
	}

	return frontendScene;
}

Scene Scene_FrontendToBackend(SceneInfo s, const Backend& b)
{
	Scene backendScene;
	backendScene.name = s.name.toStdString();

	for (const auto& dis : s.devicesInScene)
	{
		DeviceInScene backendDis;
		backendDis.device = b.GetDeviceFromUniqueId(dis.device.uniqueid.toStdString());
		backendDis.transform = dis.transform.ToMathTransform();
		backendScene.devices.push_back(backendDis);
	}

	for (const auto& e : s.effects)
	{
		backendScene.effects.push_back(Effect_FrontendToBackend(e));
	}

	return backendScene;
}

BridgeInfo Bridge_BackendToFrontend(std::shared_ptr<Hue::Bridge> b)
{
	BridgeInfo frontendBridge;
	frontendBridge.id = b->id.c_str();
	
	auto backendDevices = b->devices;
	for (auto& device : backendDevices)
	{
		frontendBridge.devices.push_back(Device_BackendToFrontend(device));
	}

	return frontendBridge;
}
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
// SceneInfo

void SceneInfo::AddDevice(QVariant Device)
{
	RemoveDevice(Device);
	
	devicesInScene.push_back(DeviceInSceneInfo{ 
		Transform{
			Math::Transform{
				{size.x() / 2.0, size.y() / 2.0, size.z() / 2.0}, {1.0, 1.0, 1.0}, {0, 0, 0}}},
				Device.value<DeviceInfo>() });
}

void SceneInfo::RemoveDevice(QVariant Device)
{
	QMutableListIterator i(devicesInScene);
	while (i.hasNext()) 
	{
		if (i.next().device == Device.value<DeviceInfo>())
		{
			i.remove();
		}
			
	}
}

QList<QVariant> SceneInfo::GetDevices()
{
	QList<QVariant> out;
	for (const auto& deviceInScene : devicesInScene)
	{
		out.push_back(QVariant::fromValue(deviceInScene.device));
	}
	return out;
}
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
// Type Factory
TypeFactory::TypeFactory(QObject* parent)
	: QObject(parent)
{

}

TypeFactory::~TypeFactory()
{

}

QVariant TypeFactory::NewSinePulseEffect() const
{
	return QVariant::fromValue(EffectInfo{ SinePulseEffectInfo{} });
}
QVariant TypeFactory::NewConstantEffect() const
{
	return QVariant::fromValue(EffectInfo{ ConstantEffectInfo{} });
}
///////////////////////////////////////////////////////////////////////////