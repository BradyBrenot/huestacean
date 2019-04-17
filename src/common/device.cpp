#include "common/device.h"

#include <sstream>

std::string Device::GetUniqueId() const
{
	std::stringstream ss;
	ss << type.ToString() << "|" << GetUniqueIdInternal();
	return ss.str();
}