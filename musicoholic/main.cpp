#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>

#include <atlstr.h>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <endpointvolume.h>

#define __C ATLENSURE_SUCCEEDED

class CNotification :
	public IAudioSessionEvents
{
private:
	INT m_nSessionIndex;

public:
	// CNotification
	CNotification(INT nSessionIndex) :
		m_nSessionIndex(nSessionIndex)
	{
	}

	// IUnknown
	STDMETHOD(QueryInterface)(REFIID, VOID** ppvObject)
	{
		_tprintf(_T("m_nSessionIndex %d, QueryInterface\n"));
		return E_NOINTERFACE;
	}
	STDMETHOD_(ULONG, AddRef)()
	{
		return 2;
	}
	STDMETHOD_(ULONG, Release)()
	{
		return 1;
	}

	// IAudioSessionEvents 
	STDMETHOD(OnDisplayNameChanged)(LPCWSTR NewDisplayName, LPCGUID EventContext)
	{
		return S_OK;
	}
	STDMETHOD(OnIconPathChanged)(LPCWSTR NewIconPath, LPCGUID EventContext)
	{
		return S_OK;
	}
	STDMETHOD(OnSimpleVolumeChanged)(float NewVolume, BOOL NewMute, LPCGUID EventContext)
	{
		_tprintf(_T("m_nSessionIndex %d, NewVolume %.2f, NewMute %d\n"), m_nSessionIndex, NewVolume, NewMute);
		return S_OK;
	}
	STDMETHOD(OnChannelVolumeChanged)(DWORD ChannelCount, float NewChannelVolumeArray[], DWORD ChangedChannel, LPCGUID EventContext)
	{
		_tprintf(_T("m_nSessionIndex %d, ChannelCount %d, NewChannelVolumeArray][ { %.2f, ... }, ChangedChannel %d\n"), m_nSessionIndex, ChannelCount, NewChannelVolumeArray[0], ChangedChannel);
		return S_OK;
	}
	STDMETHOD(OnGroupingParamChanged)(LPCGUID NewGroupingParam, LPCGUID EventContext)
	{
		return S_OK;
	}
	STDMETHOD(OnStateChanged)(AudioSessionState NewState)
	{
		return S_OK;
	}
	STDMETHOD(OnSessionDisconnected)(AudioSessionDisconnectReason DisconnectReason)
	{
		return S_OK;
	}
};

int main()
{
	__C(CoInitializeEx(NULL, COINIT_MULTITHREADED));
	CComPtr<IMMDeviceEnumerator> pDeviceEnumerator;
	__C(pDeviceEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator)));
	CComPtr<IMMDevice> pDevice;
	__C(pDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDevice));
	CComPtr<IAudioSessionManager2> pAudioSessionManager2;
	__C(pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (VOID**)&pAudioSessionManager2));
	
	for (;;)
	{
		system("cls");
		CComPtr<IAudioSessionEnumerator> pAudioSessionEnumerator;
		__C(pAudioSessionManager2->GetSessionEnumerator(&pAudioSessionEnumerator));

		INT nSessionCount;
		__C(pAudioSessionEnumerator->GetCount(&nSessionCount));
		_tprintf(_T("nSessionCount %d\n"), nSessionCount);

		for (INT nSessionIndex = 0; nSessionIndex < nSessionCount; nSessionIndex++)
		{
			CComPtr<IAudioSessionControl> pSessionControl;
			if (FAILED(pAudioSessionEnumerator->GetSession(nSessionIndex, &pSessionControl))) {
				continue;
			}

			CComHeapPtr<WCHAR> pszDisplayName;
			__C(pSessionControl->GetDisplayName(&pszDisplayName));

			CNotification* pNotification = new CNotification(nSessionIndex);
			__C(pSessionControl->RegisterAudioSessionNotification(pNotification));

			CComQIPtr<IAudioMeterInformation> pMeterInformation(pSessionControl);
			DWORD nMask;
			__C(pMeterInformation->QueryHardwareSupport(&nMask));
			FLOAT fPeakValue;
			__C(pMeterInformation->GetPeakValue(&fPeakValue));

			if (fPeakValue <= 0) {
				continue;
			}
			_tprintf(_T("nSessionIndex %d, pszDisplayName \"%s\", fPeakValue %.2f, nMask 0x%x\n"), nSessionIndex, CString(pszDisplayName).GetString(), fPeakValue, nMask);
		}
	}
	//Sleep(INFINITE);
	CoUninitialize();
	return 0;
}