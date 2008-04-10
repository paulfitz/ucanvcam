//#ifdef vcam_EXPORTS
// the dll exports
#define EXPORT __declspec(dllexport)
//#else
// the exe imports
//#define EXPORT __declspec(dllimport)
//#endif



//////////////////////////////////////////////////////////////////////////
//  This file contains routines to register / Unregister the 
//  Directshow filter 'Virtual Cam'
//  We do not use the inbuilt BaseClasses routines as we need to register as
//  a capture source
//////////////////////////////////////////////////////////////////////////
#pragma comment(lib, "kernel32")
#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")
#pragma comment(lib, "advapi32")
#pragma comment(lib, "winmm")
#pragma comment(lib, "ole32")
#pragma comment(lib, "oleaut32")

#ifdef _DEBUG
    #pragma comment(lib, "strmbasd")
#else
    #pragma comment(lib, "strmbase")
#endif


#include <streams.h>
#include <olectl.h>
#include <initguid.h>
#include <dllsetup.h>
#include "Filters.h"

#include <string>
using namespace std;
//#include "registry.h"
//#include "registry_keys.h"

//#include "Register.h"

#define PFSTDAPI EXTERN_C EXPORT HRESULT STDAPICALLTYPE

#define CreateComObject(clsid, iid, var) CoCreateInstance( clsid, NULL, CLSCTX_INPROC_SERVER, iid, (void **)&var);

PFSTDAPI AMovieSetupRegisterServer( CLSID   clsServer, LPCWSTR szDescription, LPCWSTR szFileName, LPCWSTR szThreadingModel = L"Both", LPCWSTR szServerType     = L"InprocServer32" );
PFSTDAPI AMovieSetupUnregisterServer( CLSID clsServer );



// {8E14549A-DB61-4309-AFA1-3578E927E933}
DEFINE_GUID(CLSID_VirtualCam,
            0x8e14549a, 0xdb61, 0x4319, 0xaf, 0xa1, 0x33, 0x78, 0xe9, 0x27, 0xe9, 0x13);


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
        L"Saturation Props",
        &CLSID_SaturationProp,
        CGrayProp::CreateInstance, 
        NULL, NULL
    }
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

PFSTDAPI RegisterFilters( BOOL bRegister )
{
  //CVCamStream stream(NULL,NULL,L"bozo");
  //printf("Dialog!\n");
  //stream.ShowDialog(0,0);

    HRESULT hr = NOERROR;
    WCHAR achFileName[MAX_PATH];
    char achTemp[MAX_PATH];
    ASSERT(g_hInst != 0);

    if( 0 == GetModuleFileNameA(g_hInst, achTemp, sizeof(achTemp))) 
        return AmHresultFromWin32(GetLastError());

    MultiByteToWideChar(CP_ACP, 0L, achTemp, lstrlenA(achTemp) + 1, 
                       achFileName, NUMELMS(achFileName));
  
    hr = CoInitialize(0);
    if(bRegister)
    {
        hr = AMovieSetupRegisterServer(CLSID_VirtualCam, L"ucanvcam virtual camera", achFileName, L"Both", L"InprocServer32");
    }

    if( SUCCEEDED(hr) )
    {
        IFilterMapper2 *fm = 0;
        hr = CreateComObject( CLSID_FilterMapper2, IID_IFilterMapper2, fm );
        if( SUCCEEDED(hr) )
        {
            if(bRegister)
            {
                IMoniker *pMoniker = 0;
                REGFILTER2 rf2;
                rf2.dwVersion = 1;
                rf2.dwMerit = MERIT_DO_NOT_USE;
                rf2.cPins = 1;
                rf2.rgPins = &AMSPinVCam;
                hr = fm->RegisterFilter(CLSID_VirtualCam, L"ucanvcam virtual camera", &pMoniker, &CLSID_VideoInputDeviceCategory, NULL, &rf2);
            }
            else
            {
                hr = fm->UnregisterFilter(&CLSID_VideoInputDeviceCategory, 0, CLSID_VirtualCam);
            }
        }

      // release interface
      //
      if(fm)
          fm->Release();
    }

    if( SUCCEEDED(hr) && !bRegister )
        hr = AMovieSetupUnregisterServer( CLSID_VirtualCam );

    CoFreeUnusedLibraries();
    CoUninitialize();
    return hr;
}

static HANDLE me = 0;

PFSTDAPI DllRegisterServer()
{
  printf("Register all\n"); fflush(stdout);
  /*
  if (me!=0) {
    char buf[1000];
    GetModuleFileName((HINSTANCE)(me),&buf[0],1000);
    printf(">>>>> file %s\n", buf);
    
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
    printf(">>>>> reg  %s\n", readback.c_str());
  }
  */

  AMovieDllRegisterServer2(TRUE);
  printf("Register filters\n"); fflush(stdout);
  return RegisterFilters(TRUE);
}

PFSTDAPI DllUnregisterServer()
{
  AMovieDllRegisterServer2(FALSE);
  return RegisterFilters(FALSE);
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

//BOOL APIENTRY 
EXTERN_C EXPORT BOOL STDAPICALLTYPE
DllMain(HANDLE hModule, DWORD  dwReason, LPVOID lpReserved)
{
  me = hModule;
  return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}


void RegisterService() {
  DllRegisterServer();
}

void UnregisterService() {
  DllUnregisterServer();
}




