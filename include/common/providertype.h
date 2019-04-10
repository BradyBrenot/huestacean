#pragma once

struct ProviderType
{
	enum class Type : uint8_t
	{
		Hue,
		Razer
	};

	Type type;
	ProviderType(Type inType) : type(inType)
	{

	};
};