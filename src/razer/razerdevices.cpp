#include "windows.h"
#include "razer/razerdevices.h"
#include "razer/chroma.h"

using namespace Razer;
using namespace Math;

uint32_t Razer::RgbFrom(RgbColor& c)
{
	return RGB(c.r * 255.0, c.g * 255.0, c.b * 255.0);
}

std::string GenericKeyboard::GetUniqueIdInternal() const
{
	return "GenericKeyboard";
}
void GenericKeyboard::UploadInternal(Chroma& Sdk)
{
	Sdk.CreateKeyboardEffect(ChromaSDK::Keyboard::EFFECT_TYPE::CHROMA_CUSTOM,
		&data, 
		nullptr);
}