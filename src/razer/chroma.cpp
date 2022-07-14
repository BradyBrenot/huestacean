// based on RazerChromaSampleApplication, license not assumed but this file is pretty trivial,
// what's left here is "the only way you could really do this" anyways

#include "razer/chroma.h"

#include <map>
#include <vector>

#ifdef _WIN64
#define CHROMASDKDLL        TEXT("RzChromaSDK64.dll")
#elif _WIN32
#define CHROMASDKDLL        TEXT("RzChromaSDK.dll")
#endif

using namespace std;
using namespace Razer;
using namespace ChromaSDK;
using namespace ChromaSDK::Keyboard;
using namespace ChromaSDK::Keypad;
using namespace ChromaSDK::Mouse;
using namespace ChromaSDK::Mousepad;
using namespace ChromaSDK::Headset;

#define EVENT_NAME  TEXT("{4784D90A-1179-4F7D-8558-52511D809190}")

using namespace Razer;

Chroma::Chroma() :
	m_hModule(0),
	m_hEvent(0)
{
	_Init = 0;
	_UnInit = 0;
	_CreateEffect = 0;
	_CreateKeyboardEffect = 0;
	_CreateMouseEffect = 0;
	_CreateHeadsetEffect = 0;
	_CreateMousematEffect = 0;
	_CreateKeypadEffect = 0;
	_CreateChromaLinkEffect = 0;
	_SetEffect = 0;
	_DeleteEffect = 0;
	_QueryDevice = 0;

	#ifdef _WIN32
	if (m_hModule == 0)
	{
		m_hModule = ::LoadLibrary(CHROMASDKDLL);
		if (m_hModule != 0)
		{
			_Init = (INIT)::GetProcAddress(m_hModule, "Init");
			if (_Init != 0)
			{
				RZRESULT rzResult = _Init();
				if (rzResult == RZRESULT_SUCCESS)
				{
					_CreateEffect = (CREATEEFFECT)::GetProcAddress(m_hModule, "CreateEffect");
					_CreateKeyboardEffect = (CREATEKEYBOARDEFFECT)::GetProcAddress(m_hModule, "CreateKeyboardEffect");
					_CreateMouseEffect = (CREATEMOUSEEFFECT)::GetProcAddress(m_hModule, "CreateMouseEffect");
					_CreateMousematEffect = (CREATEMOUSEPADEFFECT)::GetProcAddress(m_hModule, "CreateMousepadEffect");
					_CreateKeypadEffect = (CREATEKEYPADEFFECT)::GetProcAddress(m_hModule, "CreateKeypadEffect");
					_CreateHeadsetEffect = (CREATEHEADSETEFFECT)::GetProcAddress(m_hModule, "CreateHeadsetEffect");
					_CreateChromaLinkEffect = (CREATECHROMALINKEFFECT)::GetProcAddress(m_hModule, "CreateChromaLinkEffect");
					_SetEffect = (SETEFFECT)GetProcAddress(m_hModule, "SetEffect");
					_DeleteEffect = (DELETEEFFECT)GetProcAddress(m_hModule, "DeleteEffect");
					_QueryDevice = (QUERYDEVICE)::GetProcAddress(m_hModule, "QueryDevice");
				}
			}
		}
	}

	if (m_hEvent == 0)
	{
		m_hEvent = ::CreateEvent(0, TRUE, FALSE, EVENT_NAME);
	}
	#endif
}

Chroma::~Chroma()
{
	#ifdef _WIN32
	if (m_hEvent != 0)
	{
		::CloseHandle(m_hEvent);
		m_hEvent = 0;
	}

	if (m_hModule != 0)
	{
		_UnInit = (UNINIT)::GetProcAddress(m_hModule, "UnInit");
		if (_UnInit != 0)
		{
			RZRESULT rzResult = _UnInit();
			if (rzResult != RZRESULT_SUCCESS)
			{
				// Some error here
			}
		}

		::FreeLibrary(m_hModule);
		m_hModule = 0;
	}
	#endif
}

void Chroma::CreateEffect(RZDEVICEID DeviceId, ChromaSDK::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId)
{
	if (_CreateEffect == 0) return;

	_CreateEffect(DeviceId, Effect, pParam, pEffectId);
}

void Chroma::CreateKeyboardEffect(ChromaSDK::Keyboard::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID * pEffectId)
{
	if (_CreateKeyboardEffect == 0) return;

	_CreateKeyboardEffect(Effect, pParam, pEffectId);
}

void Chroma::CreateMouseEffect(ChromaSDK::Mouse::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID * pEffectId)
{
	if (_CreateMouseEffect == 0) return;

	_CreateMouseEffect(Effect, pParam, pEffectId);
}

void Chroma::CreateMousematEffect(ChromaSDK::Mousepad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID * pEffectId)
{
	if (_CreateMousematEffect == 0) return;

	_CreateMousematEffect(Effect, pParam, pEffectId);
}

void Chroma::CreateKeypadEffect(ChromaSDK::Keypad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID * pEffectId)
{
	if (_CreateKeypadEffect == 0) return;

	_CreateKeypadEffect(Effect, pParam, pEffectId);
}

void Chroma::CreateHeadsetEffect(ChromaSDK::Headset::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID * pEffectId)
{
	if (_CreateHeadsetEffect == 0) return;

	_CreateHeadsetEffect(Effect, pParam, pEffectId);
}

void Chroma::CreateChromaLinkEffect(ChromaSDK::ChromaLink::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID * pEffectId)
{
	if (_CreateChromaLinkEffect == 0) return;

	_CreateChromaLinkEffect(Effect, pParam, pEffectId);
}

void Chroma::SetEffect(RZEFFECTID EffectId)
{
	if (_SetEffect == 0) return;

	_SetEffect(EffectId);
}

void Chroma::DeleteEffect(RZEFFECTID EffectId)
{
	if (_DeleteEffect == 0) return;

	_DeleteEffect(EffectId);
}