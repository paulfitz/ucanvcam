#if 0
#include <stdio.h>
#include <windows.h>

#include <yarp/os/all.h>
#include <yarp/sig/all.h>
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::sig::file;

#define __forceinline inline

#include "capdrivers.h"
#include "capdriver.h"

#include <vd2/Riza/bitmap.h>

using namespace nsVDCapture;

#define DEFAULT_VIDEO_WIDTH     320
#define DEFAULT_VIDEO_HEIGHT    320

#define APPLICATIONNAME TEXT("Video Capture Previewer (PlayCap)\0")
#define CLASSNAME       TEXT("VidCapPreviewer\0")

HWND ghApp=0;
HINSTANCE g_hInst=0;

FILE *FOUT = NULL;

#define xprintf(a) printf(a); fprintf(FOUT,a)
#define xprintf2(a1,a2) printf(a1,a2); fprintf(FOUT,a1,a2)

LRESULT CALLBACK VDUB__WndMainProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        //        case WM_GRAPHNOTIFY:
            //HandleGraphEvent();
        //  break;

        case WM_SIZE:
            //ResizeVideoWindow();
            break;

        case WM_WINDOWPOSCHANGED:
            //ChangePreviewState(! (IsIconic(hwnd)));
            break;

        case WM_CLOSE:            
            // Hide the main window while the graph is destroyed
            ShowWindow(ghApp, SW_HIDE);
            //CloseInterfaces();  // Stop capturing and release interfaces
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    // Pass this message to the video window for notification of system changes
    //if (g_pVW)
    //g_pVW->NotifyOwnerMessage((LONG_PTR) hwnd, message, wParam, lParam);

    return DefWindowProc (hwnd , message, wParam, lParam);
}

class VDINTERFACE MyCallback : public IVDCaptureDriverCallback {
public:
  vdstructex<BITMAPINFOHEADER> format;

  virtual void CapBegin(sint64 global_clock) {
    xprintf("Capture begins\n");
  }

  virtual void CapEnd(const MyError *pError) {
    xprintf("Capture ends\n");
  }

  virtual bool CapEvent(nsVDCapture::DriverEvent event, int data) {
    xprintf("Capture event\n");
    
	switch(event) {
	case kEventPreroll:
	  xprintf("  event preroll\n");
	  break;

	case kEventCapturing:
	  xprintf("  capturing\n");
	  break;

	case kEventVideoFrameRateChanged:
	case kEventVideoFormatChanged:
	  xprintf("  video format\n");
	  break;

	case kEventVideoFramesDropped:
	  xprintf("  dropped\n");
	  break;

	case kEventVideoFramesInserted:
	  xprintf("  inserted\n");
	  break;

	case kEventVideoSourceChanged:
	  xprintf("  video changed\n");
	  break;

	case kEventAudioSourceChanged:
	  xprintf("  audio changed\n");
	  break;
	}

	return true;

  }

  virtual void CapProcessData(int stream, const void *data, uint32 size, sint64 timestamp, bool key, sint64 global_clock) {
    xprintf("Capture data\n");
    xprintf2("Stream is %d\n", stream);
    xprintf2("format width %d\n", format->biWidth);
    xprintf2("format height %d\n", format->biHeight);
    xprintf2("format size %ld\n", format->biSize);
    xprintf2("incoming size %ld\n", size);
    if (size==format->biWidth*format->biHeight*3) {
      xprintf("understandable!\n");
      char buf[256];
      static int ct = 0;
      sprintf(buf,"img_%06d.ppm",ct);
      ct++;
      ImageOf<PixelBgr> img;
      img.setQuantum(1);
      img.setTopIsLowIndex(false);
      img.setExternal((char*)data,format->biWidth,format->biHeight);
      write(img,buf);
    }
  }

};


int PASCAL VDUB__WinMain(HINSTANCE hInstance, HINSTANCE hInstP, LPSTR lpCmdLine, int nCmdShow)
{
  FOUT = fopen("act.txt","w");
    xprintf("starting up...\n");
    //Network yarp;
    //DriverCollection dev;
    //printf("devices %s\b", dev.status().c_str());
    g_hInst = hInstance;

    MSG msg={0};

    WNDCLASS wc;

    // Initialize COM
    if(FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
    {
      //Msg(TEXT("CoInitialize Failed!\r\n"));   
        exit(1);
    } 

    // Register the window class
    ZeroMemory(&wc, sizeof wc);
    wc.lpfnWndProc   = VDUB__WndMainProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASSNAME;
    wc.lpszMenuName  = NULL;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon         = LoadIcon (hInstance, IDI_APPLICATION);
    //LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VIDPREVIEW));
    if(!RegisterClass(&wc))
    {
      //Msg(TEXT("RegisterClass Failed! Error=0x%x\r\n"), GetLastError());
        CoUninitialize();
        exit(1);
    }

    // Create the main window.  The WS_CLIPCHILDREN style is required.
    ghApp = CreateWindow(CLASSNAME, APPLICATIONNAME,
                         WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_CLIPCHILDREN,
                         CW_USEDEFAULT, CW_USEDEFAULT,
                         DEFAULT_VIDEO_WIDTH, DEFAULT_VIDEO_HEIGHT,
                         0, 0, hInstance, 0);

    if (!ghApp) {
        xprintf("failed to create a window\n");
    }

    if(ghApp)
    {
      //Msg(TEXT("Hello!"));   
      //IVDCaptureSystem *cap = VDCreateCaptureSystemVFW();
      IVDCaptureSystem *cap = VDCreateCaptureSystemDS();
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

	IVDCaptureDriver *dd = NULL;
	if (ilongest>=0) {
	  wprintf(L"Trying to make %d: %s\n", 
		  ilongest, 
		  cap->GetDeviceName(ilongest));
	  dd = cap->CreateDriver(ilongest);
	}
	if (dd!=NULL) {
	  xprintf("Capture device made\n");

	  MyCallback callback;
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

	  //int variant;
	  //int format = VDBitmapFormatToPixmapFormat(*vformat, variant);

	  //if (!format) {
	  //printf("Cannot get compatible video format\n");
	  //}

	// some code needs this, so we need to do it anyway
	  //VDMakeBitmapCompatiblePixmapLayout(mFilterInputLayout, vformat->biWidth, vformat->biHeight, format, variant);



	  xprintf("Capture start\n");
	  dd->CaptureStart();
	  Time::delay(10);
	  dd->CaptureStop();
	  xprintf("Capture stop\n");

	  delete dd; dd = NULL;
	}

	delete cap; cap = NULL;
      }
    }

    // Release COM
    CoUninitialize();

    fclose(FOUT);
    return (int) msg.wParam;
}
#endif
