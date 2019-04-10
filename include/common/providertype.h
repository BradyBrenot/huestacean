#pragma once

struct ProviderType
{
	enum class Type : uint8
	{
		Hue,
		Razer
	};

	Type type;
	ProviderType(Type inType) : type(inType)
	{

	};
};