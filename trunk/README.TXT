 _   _  ___ __ _ _ __   __   _____ __ _ _ __ ___
| | | |/ __/ _` | '_ \  \ \ / / __/ _` | '_ ` _ \
| |_| | (_| (_| | | | |  \ V / (_| (_| | | | | | |
 \__,_|\___\__,_|_| |_|   \_/ \___\__,_|_| |_| |_|

ucanvcam is (C) 2007, 2008, 2011 Paul Fitzpatrick (paulfitz@alum.mit.edu)
It is free software, under the GPLv2, see GPL.TXT

Certain effects from the EffecTV project are included, under the GPL.

== On Windows ==

On WINDOWS only, ucanvcam contains a plugin called "ucanvcamx" which
has minimal dependencies (DirectX), and implements a virtual camera.

The remainder of ucanvcam depends on the following free libraries:
  gd, freetype, SDL, YARP, wxwidgets
See DEPENDENCIES.TXT for details

== On Linux ==

To use with the skype beta current at the time of writing(version 2.0.0.43),
you'll need the "gstfakevideo" workaround (do a web search).

== Compiling on Windows with Visual Studio (Linux is easy) ==

Compile using CMake:
  http://www.cmake.org
  
You'll need DirectX installed.  These days, it is bundled with
the Windows Platform SDK.  Install that.

Chose any build directory you want, and configure.  Hopefully, DirectX
will be found automatically.  Otherwise, edit the two variables
DIRECTX_BASECLASSES_DIR and DIRECTX_INCLUDE_DIR by hand.  They should
look something like this (at the time of writing):

DIRECTX_BASECLASSES_DIR:
  C:/Program Files/Microsoft SDKs/Windows/v7.1/Samples/multimedia/directshow/baseclasses

DIRECTX_INCLUDE_DIR:
  C:/Program Files (x86)/Microsoft SDKs/Windows/v7.0A/Include

Configure again if you need to, then Generate.  All going well, you
should now have a "ucanvcam.sln" in your build directory.  Open
this, and compile.  All going well, you'll get a "ucanvcamx*.dll"
file.  There may be warnings about sprintf and the like.

Now, if you run "cmd.exe" with administrator privileges, you should
be able to do:
  regsvr32.dll ucanvcamx*.dll
and get a happy message:
  DllRegisterServer in ucanvcamx*.dll succeeded.

You should now have a functioning virtual camera, sitting and waiting
for input.  Try it out in e.g. amcap.exe.  There should also be a 
program called "test_bus.exe" to do a basic test of the camera.
Run:
  test_bus.exe image 320 240
and you should see a change in the image shown by the virtual camera
for a second or two.

To compile the ucanvcam GUI, you'll need wxwidgets (try installing
from "wxpack") and YARP - see:
  http://eris.liralab.it/wiki/Binary_packages#Install_precompiled_versions_of_iCub_and_YARP
  
Reconfigure, recompile.
