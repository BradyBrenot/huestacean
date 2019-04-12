#pragma once

class ProviderType
{
public:
	enum Type : uint8_t
	{
		Unknown = 0,
		Hue = 1,
		Razer = 2,
		Max = 2
	};

	Type type;

	ProviderType() = delete;
	ProviderType(Type inType) : type(inType)
	{

	};

	bool operator==(ProviderType a) const { return type == a.type; }
	bool operator!=(ProviderType a) const { return type != a.type; }
};