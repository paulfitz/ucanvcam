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

#include "yarpy.h"

#include "Vcam.h"

#include "drivers.h"

#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/dev/all.h>

using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;

static int exitRequested = 0;

void sig_handler(int signo) {
  exitRequested = 1;
}

class VcamLinuxLoopback : public Vcam {
private:
  Network yarp;
  DriverCollection dev;
  int vformat;
  int w, h;
  int hin, hout;
  int readImage;
  char *vid;
  char *vidOut;
  bool got;
  ImageOf<PixelBgr> src, dest;
  ImageOf<PixelRgb> src2;
  PolyDriver source;
  IFrameGrabberImage *grabber;

  
  char *start_capture(int dev, int width, int height);
  int start_pipe (int dev, int width, int height);
  char *next_capture (int dev, char *map, int width, int height);
  int put_image(int dev, char *image, int width, int height);

public:
  VcamLinuxLoopback() {
    vformat = VIDEO_PALETTE_RGB24;
    w = 320;
    h = 240;
    printf("Loopback device has assumptions that should be generalized:\n");
    printf("   use format VIDEO_PALETTE_RGB24 (as opposed to YUV420P\n");
    printf("   w x h = 320 x 240\n");
    printf("   video input on /dev/video2\n");
    printf("   video output to /dev/video0\n");
    printf("   (loopback device should mirror /dev/video0 to /dev/video1)\n");
    printf("Make sure you have vloopback module loaded.\n");
    hin = hout = -1;
    //hin = open("/dev/video2",O_RDWR);
    //if (hin<0) printf("*** FAILED to open video input\n");
    hout = open("/dev/video0",O_RDWR);
    if (hout<0) printf("*** FAILED to open video output\n");
    readImage = 0;
    dest.setQuantum(1);
    src.setQuantum(1);
    vid = NULL;
    //vid = start_capture(hin,w,h);
    if (0) { //vid==NULL) {
      printf("*** FAILED to start capture\n");
    } else {
      //src.setExternal(vid,w,h);
      vidOut = new char[w*h*3];
      if (vidOut!=NULL) {
	dest.setExternal(vidOut,w,h);
	start_pipe(hout,w,h);
	signal(SIGTERM,sig_handler);
      } else {
	printf("*** FAILED to start capture\n");
      }
    }
    got = false;

    Property pSource;
    pSource.put("device","ffmpeg_grabber");
    pSource.put("v4l",1);
    //pSource.put("v4ldevice","/dev/video0");
    pSource.put("v4ldevice","/dev/video2");
    pSource.put("width",320);
    pSource.put("height",240);
    pSource.put("w",320);
    pSource.put("h",240);
    //pSource.put("source","/scratch/camera/dcim/135canon/mvi_3549.avi");
    bool ok = source.open(pSource);
    if (!ok) {
        pSource.put("device","test_grabber");
        pSource.put("width",320);
        pSource.put("height",240);
        pSource.put("mode","ball");
        source.open(pSource);
    }
    source.view(grabber);
  }

  virtual ~VcamLinuxLoopback() {
    if (hin>=0) { close(hin); hin = -1; }
    if (hout>=0) { close(hout); hout = -1; }
    if (vidOut!=NULL) { delete[] vidOut; vidOut = NULL; }
  }

  virtual bool isImage() {
    if (grabber==NULL) return false;
    if (!got) {
      got = grabber->getImage(src2) && (!exitRequested);
      //got = (next_capture(hin, vid, w,h)) && (!exitRequested);
      return got;
    }
    return true;
  }

  virtual yarp::sig::Image *getImage() {
    if (!got) {
      isImage();
    }
    if (!got) {
      return NULL;
    }
    got = false;
    yarpy_apply(src2,dest);
    //dest.copy(src);
    if (put_image(hout, vidOut, w, h)==0) {
      printf("*** FAILED to send image\n");
    }
    return &dest;
  }
};



static VcamLinuxLoopback my_vcam;

Vcam& getVcam() {
    return my_vcam;
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


char *VcamLinuxLoopback::start_capture (int dev, int width, int height)
{
        struct video_capability vid_caps;
	struct video_window vid_win;
	struct video_mbuf vid_buf;
	char *map;
	int fmt = vformat;

	if (ioctl (dev, VIDIOCGCAP, &vid_caps) == -1) {
		printf ("ioctl (VIDIOCGCAP)\nError[%s]\n",strerror(errno));
		return (NULL);
	}
	if (vid_caps.type & VID_TYPE_MONOCHROME) fmt=VIDEO_PALETTE_GREY;
	if (ioctl (dev, VIDIOCGMBUF, &vid_buf) == -1) {
		fprintf(stderr, "no mmap falling back on read\n");
		if (ioctl (dev, VIDIOCGWIN, &vid_win)== -1) {
			printf ("ioctl VIDIOCGWIN\nError[%s]\n",strerror(errno));
			return (NULL);
		}
		vid_win.width=width;
		vid_win.height=height;
		if (ioctl (dev, VIDIOCSWIN, &vid_win)== -1) {
			printf ("ioctl VIDIOCSWIN\nError[%s]\n",strerror(errno));
			return (NULL);
		}
		readImage=1;
		map=(char*)malloc(width*height*3);
		return (map);
	}
	/* If we are going to capture greyscale we need room to blow the image up */
	if (fmt==VIDEO_PALETTE_GREY)
		map=(char*)mmap(0, vid_buf.size*3, PROT_READ|PROT_WRITE, MAP_SHARED, dev, 0);
	else
		map=(char*)mmap(0, vid_buf.size, PROT_READ|PROT_WRITE, MAP_SHARED, dev, 0);
	
	if ((unsigned char *)-1 == (unsigned char *)map)
		return (NULL);
	return map;
}

int VcamLinuxLoopback::start_pipe (int dev, int width, int height)
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

char *VcamLinuxLoopback::next_capture (int dev, char *map, int width, 
				       int height)
{
	int i;
	char *grey, *rgb;
	struct video_mmap vid_mmap;
	int fmt = vformat;

    	sigset_t    set, old;

	if (readImage) {
		if (fmt==VIDEO_PALETTE_GREY) {
			if (read(dev, map, width*height) != width*height)
				return NULL;
		} else {
			if (read(dev, map, width*height*3) != width*height*3)
				return NULL;
		}
	} else {
		vid_mmap.format=fmt;
		vid_mmap.frame=0;
		vid_mmap.width=width;
		vid_mmap.height=height;
    
        	sigemptyset (&set);	     //BTTV hates signals during IOCTL
	        sigaddset (&set, SIGCHLD);   //block SIGCHLD & SIGALRM
       		sigaddset (&set, SIGALRM);   //for the time of ioctls
        	sigprocmask (SIG_BLOCK, &set, &old);
                    
		if (ioctl(dev, VIDIOCMCAPTURE, &vid_mmap) == -1) {
	        	sigprocmask (SIG_UNBLOCK, &old, NULL);
			return (NULL);
		}
		if (ioctl(dev, VIDIOCSYNC, &vid_mmap) == -1) {
	        	sigprocmask (SIG_UNBLOCK, &old, NULL);
			return (NULL);
		}
		
        	sigprocmask (SIG_UNBLOCK, &old, NULL); //undo the signal blocking
	}
	/* Blow up a grey */
	if (fmt==VIDEO_PALETTE_GREY) {
		i=width*height;
		grey=map+i-1;
		rgb=map+i*3;
		for (; i>=0; i--, grey--) {
			*(rgb--)=*grey;
			*(rgb--)=*grey;
			*(rgb--)=*grey;
		}
	}
	return map;
}

int VcamLinuxLoopback::put_image(int dev, char *image, int width, int height)
{
	if (write(dev, image, width*height*3)!=width*height*3) {
		printf("Error writing image to pipe!\nError[%s]\n",strerror(errno));
		return 0;
	}
	return 1;
}


