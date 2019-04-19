#include "common/device.h"

#include <sstream>

std::string Device::GetUniqueId() const
{
	std::stringstream ss;
	ss << type.ToString() << "|" << GetUniqueIdInternal();
	return ss.str();
}

ProviderType Device::GetProviderTypeFromUniqueId(std::string id)
{
	std::istringstream f(id);
	std::string s;
	if (getline(f, s, '|')) {
		return ProviderType::FromString(s);
	}

	return ProviderType::Invalid;
}