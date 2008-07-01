#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/wait.h>
#include <linux/videodev.h>

#include "Vcam.h"

#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/dev/all.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;

#include "drivers.h"

#include "Effects.h"

static int exitRequested = 0;

void sig_handler(int signo) {
  exitRequested = 1;
}

class VcamLinux : public Vcam {
private:
  Network yarp;
  DriverCollection dev;
  PolyDriver source;
  IFrameGrabberImage *grabber;
  ImageOf<PixelRgb> cache, proc;
  Bottle sources;
  PolyDriver destDriver;
  ConstString outputName;
  ConstString sourceName;
  int vformat;
  int w, h;
  int hout;
  int readImage;
  char *vidOut;
  ImageOf<PixelBgr> src, dest;
  ImageOf<PixelRgb> src2;
  bool output;

  int start_pipe (int dev, int width, int height);
  int put_image(int dev, char *image, int width, int height);

public:
  VcamLinux() {
    hout = -1;
    sourceName = "none";
    outputName = "none";
    open("test");
    output = false;
  }

  virtual ~VcamLinux() {
    stopOutput();
  }

  bool startFileOutput(const char *name = "capture.avi") {
    stopOutput();
    Property p;
    p.put("device", "ffmpeg_writer");
    p.put("out",name);
    p.put("framerate", 25);  // random number
    output = destDriver.open(p);
    return output;
  }

  bool startDevOutput(const char *name = "/dev/video0") {
    stopOutput();
    vformat = VIDEO_PALETTE_RGB24;
    w = 320;
    h = 240;
    printf("Loopback device has assumptions that should be generalized:\n");
    printf("   use format VIDEO_PALETTE_RGB24 (as opposed to YUV420P\n");
    printf("   w x h = 320 x 240\n");
    printf("   video output to /dev/video0\n");
    printf("   (loopback device should mirror /dev/video0 to /dev/video1)\n");
    printf("Make sure you have vloopback module loaded.\n");
    hout = -1;
    hout = ::open(name,O_RDWR);
    if (hout<0) { 
      printf("*** FAILED to open video output\n");
      return false;
    }
    readImage = 0;
    dest.setQuantum(1);
    src.setQuantum(1);
    vidOut = new char[w*h*3];
    if (vidOut!=NULL) {
      dest.setExternal(vidOut,w,h);
      start_pipe(hout,w,h);
      signal(SIGTERM,sig_handler);
    } else {
      printf("*** FAILED to start capture\n");
      return false;
    }
    output = true;
    return true;
  }

  bool stopOutput() {
    if (output) {
      output = false;
      if (hout!=-1) {
	::close(hout);
	hout = -1;
      }
      if (vidOut!=NULL) {
	delete[] vidOut;
	vidOut = NULL;
      }
      destDriver.close();
    }
    return true;
  }

  bool open(const char *name) {
    sourceName = name;
    grabber = NULL;
    source.close();
    Property pSource;
    pSource.put("device","ffmpeg_grabber");
    pSource.put("v4l",1);
    //pSource.put("v4ldevice","/dev/video0");
    pSource.put("v4ldevice",name);
    pSource.put("width",320);
    pSource.put("height",240);
    pSource.put("w",320);
    pSource.put("h",240);
    //pSource.put("source","/scratch/camera/dcim/135canon/mvi_3549.avi");
    bool ok = false;
    if (ConstString(name)!="test") {
      source.open(pSource);
    }
    if (!ok) {
        pSource.put("device","test_grabber");
        pSource.put("width",320);
        pSource.put("height",240);
        //pSource.put("mode","ball");
        pSource.put("mode","grid");
        source.open(pSource);
    }
    source.view(grabber);
    return true;
  }

  virtual bool isImage() {
    if (grabber==NULL) {
      return false;
    }
    return true; // not quite true...
  }

  virtual yarp::sig::Image *getImage() {
    if (grabber==NULL) return false;
    bool result = grabber->getImage(cache);
    if (!result) return NULL;
    if (!output) {
      Effects::apply(cache,proc);
      return &proc;
    } else {
      if (destDriver.isValid()) {
	Effects::apply(cache,proc);
	IFrameWriterImage *writer = NULL;
	destDriver.view(writer);
	writer->putImage(proc);
      } else {
	Effects::apply(cache,dest);
	if (put_image(hout, vidOut, w, h)==0) {
	  printf("*** FAILED to send image\n");
	}
      }
      return &dest;
    }
  }

  virtual bool getImage(ImageOf<PixelRgb>& img) {
    if (grabber==NULL) return false;
    bool result = grabber->getImage(cache);
    if (!result) return false;
    Effects::apply(cache,img);
    if (output) {
      if (destDriver.isValid()) {
	IFrameWriterImage *writer = NULL;
	destDriver.view(writer);
	writer->putImage(img);
      } else {
	dest.copy(img);
	if (put_image(hout, vidOut, w, h)==0) {
	  printf("*** FAILED to send image\n");
	}
      }
    }
    return true;
  }

  virtual bool close() {
    source.close();
  }

  virtual yarp::os::Bottle getSources() {
    sources.clear();
    sources.addString("test");
    for (int i=0; i<10; i++) {
      char buf[256];
      sprintf(buf,"/dev/video%d", i);
      struct stat s;
      int r = stat(buf,&s);
      if (r>=0) {
	sources.addString(buf);
	printf("adding source %s\n", buf);
      }
    }
    return sources;
  }

  virtual void setSource(const char *name) {
    if (sources.size()<1) {
      getSources();
    }
    printf("Should switch source to %s\n", name);
    open(name);
  }


  virtual yarp::os::Bottle getOutputs() {
    yarp::os::Bottle b;
    b.addString("none");
    for (int i=0; i<10; i++) {
      char buf[256];
      sprintf(buf,"/dev/video%d", i);
      struct stat s;
      int r = stat(buf,&s);
      if (r>=0) {
	b.addString(buf);
	printf("adding possible output %s\n", buf);
      }
    }
    b.addString("capture.mpeg2video");
    return b;
  }

  virtual bool setOutput(const char *name) {
    printf("Should set output to %s\n", name);
    if (ConstString(name)=="none") {
      stopOutput();
    } else if (name[0]=='c') {
      stopOutput();
      startFileOutput(name);      
    } else {
      stopOutput();
      startDevOutput(name);
    }
    outputName = name;
    return true;
  }


  virtual yarp::os::ConstString getCurrentOutput() {
    return outputName;
  }

  virtual yarp::os::ConstString getCurrentSource() {
    return sourceName;
  }

};



Vcam *getVcam() {
  return new VcamLinux;
}



/*
 * The following functions are from the videoloopback device examples.
 */


/*	invert.c
 *
 *	Example program for videoloopback device.
 *	Copyright 2000 by Jeroen Vreeken (pe1rxq@amsat.org)
 *	Copyright 2005 by Angel Carpintero (ack@telefonica.net)
 *	This software is distributed under the GNU public license version 2
 *	See also the file 'COPYING'.
 *
 */


int VcamLinux::start_pipe (int dev, int width, int height)
{
        struct video_capability vid_caps;
	struct video_window vid_win;
	struct video_picture vid_pic;
	int fmt = vformat;

	if (ioctl (dev, VIDIOCGCAP, &vid_caps) == -1) {
		printf ("ioctl (VIDIOCGCAP)\nError[%s]\n",strerror(errno));
		return (1);
	}
	if (ioctl (dev, VIDIOCGPICT, &vid_pic)== -1) {
		printf ("ioctl VIDIOCGPICT\nError[%s]\n",strerror(errno));
		return (1);
	}
	vid_pic.palette=fmt;
	if (ioctl (dev, VIDIOCSPICT, &vid_pic)== -1) {
		printf ("ioctl VIDIOCSPICT\nError[%s]\n",strerror(errno));
		return (1);
	}
	if (ioctl (dev, VIDIOCGWIN, &vid_win)== -1) {
		printf ("ioctl VIDIOCGWIN\nError[%s]\n",strerror(errno));
		return (1);
	}
	vid_win.width=width;
	vid_win.height=height;
	if (ioctl (dev, VIDIOCSWIN, &vid_win)== -1) {
		printf ("ioctl VIDIOCSWIN\nError[%s]\n",strerror(errno));
		return (1);
	}
	return 0;
}

int VcamLinux::put_image(int dev, char *image, int width, int height)
{
	if (write(dev, image, width*height*3)!=width*height*3) {
		printf("Error writing image to pipe!\nError[%s]\n",strerror(errno));
		return 0;
	}
	return 1;
}


