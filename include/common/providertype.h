#pragma once

#include <functional>
#include <string>

class ProviderType
{
public:
	enum Type : uint8_t
	{
		Invalid = 0,
		Hue = 1,
		Razer = 2
	};

	Type type;

	ProviderType() = delete;
	ProviderType(Type inType) : type(inType)
	{

	};

	std::string ToString() const
	{
		switch (type)
		{
		case Hue:
			return "Hue";
			break;
		case Razer:
			return "Razer";
			break;
		case Invalid:
		default:
			break;
		}

		return "INVALID";
	}

	static ProviderType FromString(std::string s)
	{
		if (s == ProviderType(Hue).ToString())
		{
			return ProviderType{ Hue };
		}
		else if (s == ProviderType(Razer).ToString())
		{
			return ProviderType{ Razer };
		}
		
		return ProviderType{ Invalid };
	}

	bool operator==(ProviderType a) const { return type == a.type; }
	bool operator!=(ProviderType a) const { return type != a.type; }
	bool operator>(ProviderType a) const { return type > a.type; }
	bool operator<(ProviderType a) const { return type < a.type; }
};

namespace std
{
	template <>
	struct hash<ProviderType>
	{
		std::size_t operator()(const ProviderType& x) const
		{
			return hash<uint8_t>()(x.type);
		}
	};
}