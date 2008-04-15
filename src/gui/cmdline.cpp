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

int main(int argc, char *argv[]) {
    Property options;
    options.fromCommand(argc,argv);

    if (argc==1) {
        printf("Welcome to command-line ucanvcam.\n");
    }

    printf("command-line ucanvcam does not do anything yet, sorry.\n");

    return 0;
}

