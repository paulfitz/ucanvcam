// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2008 Paul Fitzpatrick
 * CopyPolicy: Released under the terms of the GNU GPL v2.0.
 *
 */

// Based on an example by rep movsd via the mad hatter

#define EXPORT __declspec(dllexport)

#include <streams.h>
#include <olectl.h>
#include <initguid.h>
#include <dllsetup.h>
#include "Filters.h"

#include <string>
using namespace std;
#include "registry.h"
#include "registry_keys.h"

// we need to be careful here with MINGW
#define PFSTDAPI EXTERN_C EXPORT HRESULT STDAPICALLTYPE

#define CreateComObject(clsid, iid, var) CoCreateInstance( clsid, NULL, CLSCTX_INPROC_SERVER, iid, (void **)&var);

PFSTDAPI AMovieSetupRegisterServer( CLSID   clsServer, LPCWSTR szDescription, LPCWSTR szFileName, LPCWSTR szThreadingModel = L"Both", LPCWSTR szServerType     = L"InprocServer32" );
PFSTDAPI AMovieSetupUnregisterServer( CLSID clsServer );


DEFINE_GUID(CLSID_VirtualCam, 0x1195d708, 0x2f98, 
            0x4643, 0xa7, 0xfe, 0x59, 0x47, 0x04, 0xe4, 0xfc, 0xb6);

const AMOVIESETUP_MEDIATYPE AMSMediaTypesVCam = 
{ 
    &MEDIATYPE_Video, 
    &MEDIASUBTYPE_NULL 
};

const AMOVIESETUP_PIN AMSPinVCam=
{
    L"Output",             // Pin string name
    FALSE,                 // Is it rendered
    TRUE,                  // Is it an output
    FALSE,                 // Can we have none
    FALSE,                 // Can we have many
    &CLSID_NULL,           // Connects to filter
    NULL,                  // Connects to pin
    1,                     // Number of types
    &AMSMediaTypesVCam      // Pin Media types
};

const AMOVIESETUP_FILTER AMSFilterVCam =
{
    &CLSID_VirtualCam,  // Filter CLSID
    L"ucanvcam virtual camera",     // String name
    MERIT_DO_NOT_USE,      // Filter merit
    1,                     // Number pins
    &AMSPinVCam             // Pin details
};

CFactoryTemplate g_Templates[] = 
{
    {
        L"ucanvcam virtual camera",
        &CLSID_VirtualCam,
        CVCam::CreateInstance,
        NULL,
        &AMSFilterVCam
    },
    // This entry is for the property page.
    { 
        L"ucanvcam properties",
        &CLSID_SaturationProp,
        CGrayProp::CreateInstance, 
        NULL, NULL
    }
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

PFSTDAPI RegisterFilters(BOOL bRegister) {
    HRESULT hr = NOERROR;
    WCHAR achFileName[MAX_PATH];
    char achTemp[MAX_PATH];
    ASSERT(g_hInst != 0);
    
    if( 0 == GetModuleFileNameA(g_hInst, achTemp, sizeof(achTemp))) 
        return AmHresultFromWin32(GetLastError());
    
    MultiByteToWideChar(CP_ACP, 0L, achTemp, lstrlenA(achTemp) + 1, 
                        achFileName, NUMELMS(achFileName));
    
    hr = CoInitialize(0);
    if(bRegister) {
        hr = AMovieSetupRegisterServer(CLSID_VirtualCam, L"ucanvcam virtual camera", achFileName, L"Both", L"InprocServer32");
    }
    
    if(SUCCEEDED(hr)) {
        IFilterMapper2 *fm = 0;
        hr = CreateComObject( CLSID_FilterMapper2, IID_IFilterMapper2, fm );
        if (SUCCEEDED(hr)) {
            if(bRegister) {
                IMoniker *pMoniker = 0;
                REGFILTER2 rf2;
                rf2.dwVersion = 1;
                rf2.dwMerit = MERIT_DO_NOT_USE;
                rf2.cPins = 1;
                rf2.rgPins = &AMSPinVCam;
                hr = fm->RegisterFilter(CLSID_VirtualCam, L"ucanvcam virtual camera", &pMoniker, &CLSID_VideoInputDeviceCategory, NULL, &rf2);
            } else {
                hr = fm->UnregisterFilter(&CLSID_VideoInputDeviceCategory, 0, CLSID_VirtualCam);
            }
        }
        
        // release interface
        //
        if(fm)
            fm->Release();
    }

    if (SUCCEEDED(hr) && !bRegister)
        hr = AMovieSetupUnregisterServer( CLSID_VirtualCam );
    
    CoFreeUnusedLibraries();
    CoUninitialize();
    return hr;
}

static HANDLE me = 0;

PFSTDAPI DllRegisterServer() {
    
    // We used to need the dll location, in order to get
    // media resources stored nearby.  Not actually needed any more,
    // with NSIS install.
    if (me!=0) {
        char buf[1000];
        GetModuleFileName((HINSTANCE)(me),&buf[0],1000);
        printf(">>>>> dll is %s\n", buf);
        
        std::string path("");
        std::string sfname = buf;
        int index = sfname.rfind('/');
        if (index==-1) {
            index = sfname.rfind('\\');
        }
        if (index!=-1) {
            path = sfname.substr(0,index);
        }
        printf(">>>>> path %s\n", path.c_str());
        putRegistry(KEY_ROOT,path.c_str());
        std::string readback = getRegistry(KEY_ROOT);
        printf(">>>>> registered %s\n", readback.c_str());
    }
    
    return RegisterFilters(TRUE);
}

PFSTDAPI DllUnregisterServer() {
    return RegisterFilters(FALSE);
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

EXTERN_C EXPORT BOOL STDAPICALLTYPE
DllMain(HANDLE hModule, DWORD  dwReason, LPVOID lpReserved) {
    me = hModule;
    return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}


void RegisterService() {
    DllRegisterServer();
}

void UnregisterService() {
    DllUnregisterServer();
}




