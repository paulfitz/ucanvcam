// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2008 Paul Fitzpatrick
 * CopyPolicy: Released under the terms of the GNU GPL v2.0.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <yarp/os/all.h>
#include <yarp/sig/all.h>

#include "Vcam.h"
#include "Effects.h"

using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::sig::file;

extern Vcam *getVcam();
static Vcam *myVcam = NULL;
static Vcam& theVcam() {
    if (myVcam==NULL) {
        myVcam = getVcam();
        if (myVcam==NULL) {
            printf("failed to allocate vcam\n");
            exit(1);
        }
    }
    return *myVcam;
}



int main(int argc, char *argv[]) {
    Property options;
    options.fromCommand(argc,argv);

    if (options.check("help")||argc<2) {
        printf("Welcome to command-line ucanvcam.\n");
        printf("To get a list of effects, sources, and outputs:\n"); 
        printf("  ucanvcam --list\n");
        printf("To run:\n"); 
        printf("  ucanvcam [--source </dev/videoN>]    # choose input\n");
        printf("           [--output </dev/videoN>]    # choose output\n");
        printf("           [--effect <effect>]         # choose effect\n");
        printf("           [--save <prefix>]           # save frames as .ppm\n");
        printf("           [--run]           # use all default options\n");
        printf("To get this help:\n");
        printf("  ucanvcam --help\n");
        return 0;
    }

    printf("==============================================================\n");
    printf("== Loading effects\n");
    printf("==\n");

    Effects effect;
    Bottle lst = effect.getEffects();

    printf("==============================================================\n");
    printf("== Finding inputs / outputs\n");
    printf("==\n");
    Bottle sources = theVcam().getSources();
    Bottle outputs = theVcam().getOutputs();
    printf("==============================================================\n");
    printf("== Starting ucanvcam\n");
    printf("==\n");
    printf("EFFECTS: %s\n", lst.toString().c_str());
    printf("SOURCES: %s\n", sources.toString().c_str());
    printf("OUTPUTS: %s\n", outputs.toString().c_str());

    if (options.check("list")) {
        // we are done
        return 0;
    }

    if (options.check("source")) {
        theVcam().setSource(options.check("source",
                                       sources.get(0)).asString().c_str());
    }
    
    if (options.check("output")) {
        theVcam().setOutput(options.check("output",
                                          outputs.get(0)).asString().c_str());
    }

    if (options.check("effect")) {
        effect.setEffect(options.check("effect",
                                       Value("TickerTV")).asString().c_str());
    }

    printf("Selected effect is: %s\n", effect.getCurrentEffect().c_str());
    printf("Selected source is: %s\n", theVcam().getCurrentSource().c_str());
    printf("Selected output is: %s\n", theVcam().getCurrentOutput().c_str());


    ConstString prefix = options.check("save",Value("demo_")).asString();
    bool shouldSave = options.check("save");

    ImageOf<PixelRgb> img;
    double start = Time::now();
    double last = start-5.0001;
    while (true) {
        static int ct = 0;
        double now = Time::now();
        if (now-last>5) {
            last += 5;
            printf("Time %lds; Frame: %d\n",(int)(now-start+0.5),ct);
        }
        ct++;
        theVcam().getImage(img);
        if (shouldSave) {
            char buf[1000];
            snprintf(buf,sizeof(buf),"%s%06d.ppm",prefix.c_str(),ct);
            write(img,buf);
            printf("Saved image to %s\n", buf);
        }
    }

    return 0;
}

