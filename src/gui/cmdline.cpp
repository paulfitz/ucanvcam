// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright (C) 2008 Paul Fitzpatrick
 * CopyPolicy: Released under the terms of the GNU GPL v2.0.
 *
 */

#include <stdio.h>

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

    if (argc==1) {
        printf("Welcome to command-line ucanvcam.\n");
        printf("Call as:\n"); 
        printf("  ./ucanvcam --list\n");
        printf("  ./ucanvcam [--source /dev/videoN] [--output /dev/videoN] [--effect effect]\n");
       exit(0);
    }

    printf("command-line ucanvcam does not do anything yet, sorry.\n");

    Effects effect;
    Bottle lst = effect.getEffects();
    Bottle sources = theVcam().getSources();
    Bottle outputs = theVcam().getOutputs();
    printf("\n\n");
    printf("Effects are %s\n", lst.toString().c_str());
    printf("Sources are %s\n", sources.toString().c_str());
    printf("Outputs are %s\n", outputs.toString().c_str());

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

    // just as a test, save images...
    ImageOf<PixelRgb> img;
    while (true) {
        printf("Waiting for image\n");
        theVcam().getImage(img);
        char buf[256];
        static int ct = 0;
        sprintf(buf,"demo_%06d.ppm",ct);
        ct++;
        write(img,buf);
        printf("Saved image to %s\n", buf);
    }

    return 0;
}

