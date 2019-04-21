#pragma once

//oh no
#include "windows.h"

#include "RzChromaSDKDefines.h"
#include "RzChromaSDKTypes.h"
#include "RzErrors.h"

namespace Razer
{
	class Chroma
	{
	public:
		Chroma();
		~Chroma();

	public:
		static Chroma& Get();

		void CreateEffect(RZDEVICEID DeviceId, ChromaSDK::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId = NULL);
		void CreateKeyboardEffect(ChromaSDK::Keyboard::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId = NULL);
		void CreateMouseEffect(ChromaSDK::Mouse::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId = NULL);
		void CreateMousematEffect(ChromaSDK::Mousepad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId = NULL);
		void CreateKeypadEffect(ChromaSDK::Keypad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId = NULL);
		void CreateHeadsetEffect(ChromaSDK::Headset::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId = NULL);
		void CreateChromaLinkEffect(ChromaSDK::ChromaLink::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId = NULL);
		void SetEffect(RZEFFECTID EffectId);
		void DeleteEffect(RZEFFECTID EffectId);
		void QueryDevice(RZDEVICEID DeviceId, ChromaSDK::DEVICE_INFO_TYPE& DeviceInfo);

	private:
		HMODULE m_hModule;
		HANDLE m_hEvent;

		typedef RZRESULT(*INIT)(void);
		typedef RZRESULT(*UNINIT)(void);
		typedef RZRESULT(*CREATEEFFECT)(RZDEVICEID DeviceId, ChromaSDK::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
		typedef RZRESULT(*CREATEKEYBOARDEFFECT)(ChromaSDK::Keyboard::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
		typedef RZRESULT(*CREATEHEADSETEFFECT)(ChromaSDK::Headset::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
		typedef RZRESULT(*CREATEMOUSEPADEFFECT)(ChromaSDK::Mousepad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
		typedef RZRESULT(*CREATEMOUSEEFFECT)(ChromaSDK::Mouse::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
		typedef RZRESULT(*CREATEKEYPADEFFECT)(ChromaSDK::Keypad::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
		typedef RZRESULT(*CREATECHROMALINKEFFECT)(ChromaSDK::ChromaLink::EFFECT_TYPE Effect, PRZPARAM pParam, RZEFFECTID* pEffectId);
		typedef RZRESULT(*SETEFFECT)(RZEFFECTID EffectId);
		typedef RZRESULT(*DELETEEFFECT)(RZEFFECTID EffectId);
		typedef RZRESULT(*REGISTEREVENTNOTIFICATION)(HWND hWnd);
		typedef RZRESULT(*UNREGISTEREVENTNOTIFICATION)(void);
		typedef RZRESULT(*QUERYDEVICE)(RZDEVICEID DeviceId, ChromaSDK::DEVICE_INFO_TYPE& DeviceInfo);

		INIT _Init;
		UNINIT _UnInit;
		CREATEEFFECT _CreateEffect;
		CREATEKEYBOARDEFFECT _CreateKeyboardEffect;
		CREATEMOUSEEFFECT _CreateMouseEffect;
		CREATEHEADSETEFFECT _CreateHeadsetEffect;
		CREATEMOUSEPADEFFECT _CreateMousematEffect;
		CREATEKEYPADEFFECT _CreateKeypadEffect;
		CREATECHROMALINKEFFECT _CreateChromaLinkEffect;
		SETEFFECT _SetEffect;
		DELETEEFFECT _DeleteEffect;
		QUERYDEVICE _QueryDevice;
	};
}
