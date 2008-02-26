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

int main(int argc, char *argv[]) {
    Network yarp;
    DriverCollection dev;

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

    Property pSource;
    pSource.put("device","ffmpeg_grabber");
    //pSource.put("v4l",1);
    //pSource.put("v4ldevice","/dev/video0");
    //pSource.put("v4ldevice","/dev/video2");
    pSource.put("source","/scratch/camera/dcim/135canon/mvi_3549.avi");
    bool ok = source.open(pSource);
    if (!ok) {
        pSource.put("device","test_grabber");
        pSource.put("width",320);
        pSource.put("height",240);
        pSource.put("mode","ball");
        source.open(pSource);
    }

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



