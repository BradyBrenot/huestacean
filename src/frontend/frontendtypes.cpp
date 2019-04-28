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
	ds << in.devicesInScene;
	ds << in.effects;
	return ds;
}
QDataStream& operator>>(QDataStream& ds, SceneInfo& out)
{
	ds >> out.devicesInScene;
	ds >> out.effects;
	return ds;
}

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

	return out;
}

DevicePtr Device_FrontendToBackend(DeviceInfo d, const Backend& b)
{
	return b.GetDeviceFromUniqueId(d.uniqueid.toStdString());
}

//EffectInfo Effect_BackendToFrontend(const std::unique_ptr<Effect>& e);
//std::unique_ptr<Effect> Effect_FrontendToBackend(EffectInfo e);

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
		backendDis.transform = dis.transform;
		backendScene.devices.push_back(backendDis);
	}

	for (const auto& e : s.effects)
	{
		backendScene.effects.push_back(Effect_FrontendToBackend(e));
	}

	return backendScene;
}

//BridgeInfo Bridge_BackendToFrontend(std::shared_ptr<Hue::Bridge> b);
