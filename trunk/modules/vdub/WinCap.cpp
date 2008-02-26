
#include "WinCap.h"

HINSTANCE g_hInst;

bool WinCap::open(yarp::os::Searchable& config) {
  close();
  cap = VDCreateCaptureSystemDS();
  
  ghApp = (HWND)config.check("hwnd",
			     yarp::os::Value(0),
			     "graphics handle").asInt();
  
  g_hInst = (HINSTANCE)config.check("hinstance",
				    yarp::os::Value(0),
				    "instance handle").asInt();
  
  if (cap!=NULL) {
    xprintf("Capture system made\n");
    cap->EnumerateDrivers();
    int ct = cap->GetDeviceCount();
    xprintf2("Got %d devices\n", ct);
    int ilongest = -1;
    int longest = 0;
    for (int i=0; i<ct; i++) {
      wprintf(L"  %d: %s\n", i, cap->GetDeviceName(i));
      if (wcslen(cap->GetDeviceName(i)) > longest) {
	longest = wcslen(cap->GetDeviceName(i));
	ilongest = i;
      }
    }
    if (ilongest>=0) {
      wprintf(L"Trying to make %d: %s\n", 
	      ilongest, 
	      cap->GetDeviceName(ilongest));
      dd = cap->CreateDriver(ilongest);
    }
    if (dd!=NULL) {
      xprintf("Capture device made\n");
      
      if (!dd->Init((VDGUIHandle)ghApp)) {
	printf("Failure to init capture device\n");
      }
      
      vdstructex<BITMAPINFOHEADER> vformat;
      
      if (!dd->GetVideoFormat(vformat)) {
	printf("Cannot get video format\n");
      }
      printf("Got video format\n");
      
      callback.format = vformat;
      
      dd->SetCallback(&callback);
      
      xprintf("Capture start\n");
      closed = false;
      dd->CaptureStart();
      return true;
    } else {
      if (cap!=NULL) delete cap;
      cap = NULL;
    }
  }
  return false;
}
