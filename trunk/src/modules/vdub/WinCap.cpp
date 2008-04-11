
#include "WinCap.h"

HINSTANCE g_hInst;

#include <stdlib.h>
#include <string>

using namespace yarp::os;
using namespace std;

yarp::os::Bottle WinCap::getSources() {
  /*
  printf("Scanning for sources\n");
  if (cap == NULL) {
    cap = VDCreateCaptureSystemDS();
  }
  
  if (cap!=NULL) {
    printf("Getting sources\n");
    cap->EnumerateDrivers();
    int ct = cap->GetDeviceCount();
    printf("Got %d sources\n", ct);
    for (int i=0; i<ct; i++) {
      char buf[1000];
      wcstombs(buf,cap->GetDeviceName(i),sizeof(buf));
      printf("Adding source %s\n", buf);
      sources.addString(buf);
    }
  }
  */
  return sourcesList;
}

yarp::os::ConstString WinCap::guessSource() {
  Bottle source = getSources();
  int ilongest = -1;
  int longest = 0;
  for (int i=0; i<source.size(); i++) {
    string name = source.get(i).asString().c_str();
    if (name.length() > longest) {
      longest = name.length();
      ilongest = i;
    }
  }
  return source.get(ilongest).asString();
}


bool WinCap::open(yarp::os::Searchable& config) {
  printf("open %s:%d\n",__FILE__,__LINE__); fflush(stdout);
  close();
  printf("open %s:%d\n",__FILE__,__LINE__); fflush(stdout);
  cap = VDCreateCaptureSystemDS();
  printf("open %s:%d\n",__FILE__,__LINE__); fflush(stdout);
  
  ghApp = (HWND)config.check("hwnd",
			     yarp::os::Value(0),
			     "graphics handle").asInt();
  
  printf("open %s:%d\n",__FILE__,__LINE__); fflush(stdout);
  g_hInst = (HINSTANCE)config.check("hinstance",
				    yarp::os::Value(0),
				    "instance handle").asInt();
  
  printf("open %s:%d\n",__FILE__,__LINE__); fflush(stdout);
  if (cap!=NULL) {
    xprintf("Capture system made\n");
    cap->EnumerateDrivers();
    int ct = cap->GetDeviceCount();
    xprintf2("Got %d devices\n", ct);
    int ilongest = -1;
    int longest = 0;
    int imatch = -1;
    ConstString targetName = config.check("source",
					  Value("notset")).asString();
    for (int i=0; i<ct; i++) {
      wprintf(L"  %d: %s\n", i, cap->GetDeviceName(i));

      TCHAR buf[2048];
      WideCharToMultiByte(CP_ACP,0,cap->GetDeviceName(i), 
			  -1, buf, 2048, NULL, NULL);
      printf("Adding source %s\n", buf);
      sourcesList.addString(buf);
      printf("Added source %s\n", buf);

      if (wcslen(cap->GetDeviceName(i)) > longest) {
	longest = wcslen(cap->GetDeviceName(i));
	ilongest = i;
      }

      if (targetName == buf) {
	imatch = i;
      }
    }
    if (config.check("passive")) {
      xprintf("Passive WinCap\n");
      return true;
    }

    if (imatch>=0) {
      ilongest = imatch;
    }

    if (ilongest>=0) {
      wprintf(L"Trying to make %d: %s\n", 
	      ilongest, 
	      cap->GetDeviceName(ilongest));
      dd = cap->CreateDriver(ilongest);
    }
    if (dd!=NULL) {
      xprintf("Capture device made, ok\n");
      
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
