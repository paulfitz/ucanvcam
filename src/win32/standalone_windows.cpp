// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2007 Paul Fitzpatrick
 * CopyPolicy: Released under the terms of the GNU GPL v2.0.
 *
 */


#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/dev/all.h>

#include "drivers.h"

#include "yarpy.h"

#define NETWORKED 1

using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;
using namespace yarp::sig::file;

#ifdef WIN32

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

//void Msg(TCHAR *szFormat, ...);

#define DEFAULT_VIDEO_WIDTH     320
#define DEFAULT_VIDEO_HEIGHT    320

#define APPLICATIONNAME TEXT("Video Capture Previewer (PlayCap)\0")
#define CLASSNAME       TEXT("VidCapPreviewer\0")

//
//HWND ghApp=0;

//FILE *FOUT = NULL;

void Msg(TCHAR *szFormat, ...)
{
    TCHAR szBuffer[1024];  // Large buffer for long filenames or URLs
    const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
    const int LASTCHAR = NUMCHARS - 1;

    // Format the input string
    va_list pArgs;
    va_start(pArgs, szFormat);

    // Use a bounded buffer size to prevent buffer overruns.  Limit count to
    // character size minus one to allow for a NULL terminating character.
    _vsnprintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);
    va_end(pArgs);

    // Ensure that the formatted string is NULL-terminated
    szBuffer[LASTCHAR] = TEXT('\0');

    MessageBox(NULL, szBuffer, TEXT("PlayCap Message"), MB_OK | MB_ICONERROR);
}

#include "WinCap.h"

#endif


class Monitor : public Thread {
public:
    IService *service;
    PolyDriver *closer;

    virtual void run() {
        bool ok = true;
        while (ok) {
            ok = service->updateService();
            if (!ok) {
                printf("Monitor alerted of shutdown\n");
                closer->close();
            } 
        }
    }
};


int base_main(int argc, char *argv[]) {
    Network yarp;
    DriverCollection dev;


    printf("And so it begins\n");
#if WIN32
    Drivers::factory().add(new DriverCreatorOf<WinCap>("wincap",
                                                       "grabber",
                                                       ""));
#endif

    printf("devices %s\b", dev.status().c_str());

    PolyDriver source, sink;


    Property input;
    input.fromCommand(argc,argv);

    ConstString portName = input.check("name",
                                       Value("/yarpview"),
                                       "name of port").asString();

    // common shortcut
    if (argc==2) {
        portName = argv[1];
    }

    // actual useful configuration - port to wxsdl

#if 0
    Property pSource;
    pSource.put("device","test_grabber");
    pSource.put("width",320);
    pSource.put("height",240);
    pSource.put("w",320);
    pSource.put("h",240);
    pSource.put("mode","ball");
    pSource.put("framerate",30);
    //    pSource.put("local",portName.c_str());
    source.open(pSource);
#else

#ifdef WIN32
    Property pSource;
    pSource.put("device","wincap");
    //pSource.put("device","vfw_grabber");
    source.open(pSource);
#else
    Property pSource;
    //pSource.put("device","opencv_grabber");
    pSource.put("device","ffmpeg_grabber");
    //pSource.put("v4l",1);
    //pSource.put("v4ldevice","/dev/video0");
    //pSource.put("v4ldevice","/dev/video2");
    pSource.put("source","/scratch/camera/dcim/135canon/mvi_3549.avi");
    //pSource.put("source","/home/paulfitz/daily/auto/foo.avi");
    bool ok = source.open(pSource);
    if (!ok) {
        pSource.put("device","test_grabber");
        pSource.put("width",320);
        pSource.put("height",240);
        pSource.put("mode","ball");
        source.open(pSource);
    }
#endif

#endif


    //testCap();
    //    exit(1);

    Property pSink = input;
    pSink.put("device","wxsdl");
    pSink.put("w",320);
    pSink.put("h",240);
    pSink.put("title",portName.c_str());
    sink.open(pSink);

    IFrameGrabberImage *iSource;
    source.view(iSource);
    IFrameWriterImage *iSink;
    sink.view(iSink);
    IService *iThread;
    sink.view(iThread);

    bool done = false;

    if (iSource==NULL||iSink==NULL) {
        if (iSource!=NULL) {
            printf("Cannot find image source\n");
        }
        if (iSink!=NULL) {
            printf("Cannot find image sink\n");
        }
        done = true;
    }

    //Network::connect("/grabber","/foo");

    Monitor monitor;
    monitor.service = iThread;
    monitor.closer = &source;
    if (iThread!=NULL) {
        printf("Starting monitor\n");
        monitor.start();
    }
    Time::delay(1);

    ImageOf<PixelRgb> image, proc;
    while (!done) {
        bool ok = iSource->getImage(image);
        if (ok) {
            yarpy_apply(image,proc);
            ok = iSink->putImage(proc);
        } 
        if (!ok) {
            done = true;
        }
    }

    printf("Stopping effect\n");
    yarpy_stop();

    printf("Stopping monitor\n");
    monitor.stop();

    sink.close();
    source.close();

    return 0;
}



#ifndef WIN32
int main(int argc, char *argv[]) {
    printf("NOT win32\n");
    return base_main(argc,argv);
}
#else


LRESULT CALLBACK WndMainProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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



int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hInstP, LPSTR lpCmdLine, int nCmdShow)
{
    printf("WinMain\n");

    FOUT = fopen("out.txt","w");

    //printf("starting up...\n");
    //Network yarp;
    //DriverCollection dev;
    //printf("devices %s\b", dev.status().c_str());
    
    MSG msg={0};

    WNDCLASS wc;

    // Initialize COM
    if(FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
    {
        Msg(TEXT("CoInitialize Failed!\r\n"));   
        exit(1);
    } 

    // Register the window class
    ZeroMemory(&wc, sizeof wc);
    wc.lpfnWndProc   = WndMainProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASSNAME;
    wc.lpszMenuName  = NULL;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon         = LoadIcon (hInstance, IDI_APPLICATION);
    //LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VIDPREVIEW));
    if(!RegisterClass(&wc))
    {
        Msg(TEXT("RegisterClass Failed! Error=0x%x\r\n"), GetLastError());
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
        printf("failed to create a window\n");
    }

    if(ghApp)
    {
        ghApp = NULL;
        base_main(0,NULL);
    }

    // Release COM
    CoUninitialize();

    return (int) msg.wParam;
}



#endif
