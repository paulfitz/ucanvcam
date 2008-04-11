/*	invert.c
 *
 *	Example program for videoloopback device.
 *	Copyright 2000 by Jeroen Vreeken (pe1rxq@amsat.org)
 *	Copyright 2005 by Angel Carpintero (ack@telefonica.net)
 *	This software is distributed under the GNU public license version 2
 *	See also the file 'COPYING'.
 *
 */

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

using namespace yarp::sig;

int fmt=0;
int noexit = 1;
int read_img=0;

char *start_capture (int dev, int width, int height)
{
        struct video_capability vid_caps;
	struct video_window vid_win;
	struct video_mbuf vid_buf;
	char *map;

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
		read_img=1;
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

int start_pipe (int dev, int width, int height)
{
        struct video_capability vid_caps;
	struct video_window vid_win;
	struct video_picture vid_pic;

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

char *next_capture (int dev, char *map, int width, int height)
{
	int i;
	char *grey, *rgb;
	struct video_mmap vid_mmap;

    	sigset_t    set, old;

	if (read_img) {
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

int put_image(int dev, char *image, int width, int height)
{
	if (write(dev, image, width*height*3)!=width*height*3) {
		printf("Error writing image to pipe!\nError[%s]\n",strerror(errno));
		return 0;
	}
	return 1;
}


void sig_handler(int signo)
{
	noexit = 0;
}

int main (int argc, char **argv)
{
	int i, devin, devout;
	int width;
	int height;
	char *image_out, *image_new;
	char palette[10]={'\0'};
	
	if (argc != 5) {
		printf("Usage:\n\n");
		printf("sweetcam input output widthxheight rgb24|yuv420p\n\n");
		printf("example: ./sweetcam /dev/video2 /dev/video0 320x240 rgb24\n\n");
		printf("  (assumes that you have a webcam available as /dev/video2)\n");
		printf("  (modified version will be available as /dev/video1)\n");
		exit(1);
	}
	sscanf(argv[3], "%dx%d", &width, &height);
	sscanf(argv[4], "%s", palette);

	if (!strcmp(palette,"rgb24")) fmt = VIDEO_PALETTE_RGB24;
	else if (!strcmp(palette,"yuv420p")) fmt = VIDEO_PALETTE_YUV420P;
	else fmt = VIDEO_PALETTE_RGB24;

	
	image_out=(char*)malloc(width*height*3);

	devin=open (argv[1], O_RDWR);
	if (devin < 0) {
		printf("Failed to open input video device [%s]\nError:[%s]\n",argv[1],strerror(errno));
		exit(1);
	}

	devout=open (argv[2], O_RDWR);
	if (devout < 0) {
		printf ("Failed to open output video device [%s]\nError:[%s]\n",argv[2],strerror(errno));
		exit(1);
	}

	image_new=start_capture (devin, width, height);
	if (!image_new) {
		printf("Capture error \n Error[%s]\n",strerror(errno));
		exit(1);
	}

	start_pipe(devout, width, height);

	signal(SIGTERM, sig_handler);
	
	printf("Starting video stream.\n");
	while ( (next_capture(devin, image_new, width, height)) && (noexit) ){
	  //for (i=width*height*3; i>=0; i--) image_out[i]=-image_new[i];
	  //for (i=width*height*3; i>=0; i--) image_out[i]=image_new[i];
	  ImageOf<PixelBgr> src, dest;
	  src.setExternal(image_new,width,height);
	  dest.setExternal(image_out,width,height);
	  yarpy_apply(src,dest);
	  if (put_image(devout, image_out, width, height)==0)
	    exit(1);
	}
	printf("You bought vaporware!\nError[%s]\n",strerror(errno));
	yarpy_stop();
	close (devin);
	close (devout);
	free(image_out);
	exit(0);
}
