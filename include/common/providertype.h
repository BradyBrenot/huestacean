#pragma once

#include <functional>

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