#include "ImageLoader.h"

#include <string.h>
#include <stdlib.h>
#include <string>


using namespace yarp::sig;

using namespace std;

// is(png|gif|jpg) taken from imglib.cc

/* gifsize, jpgsize and pngsize return the width and height of an
   image of the specified size.

   This code is Copyright (c) 2001, 2005 Tom Murphy 7 and distributed
   under the terms of the GPL.
*/

const char * ispng(const char * name, int & result) {
  FILE * png = fopen(name, "rb");
  char buf[32];
  if (!png) {
    return "ispng: Failure in fopen";
  }
  if (8 != fread(buf, 1, 8, png)) {
    fclose(png);
    return "ispng: couldn't read magic number";
  }
  result = !memcmp(buf, "\x89PNG\x0D\x0A\x1A\x0A", 8);
  fclose(png);
  return 0;
}

const char * isgif(const char * name, int & result) {
  FILE * gif = fopen(name, "rb");
  char buf[32];
  if (!gif) {
    return "isgif: Failure in fopen";
  }
  if (6 != fread(buf, 1, 6, gif)) {
    fclose(gif);
    return "isgif: couldn't read magic number";
  }
  result = !memcmp(buf, "GIF89a", 6);
  fclose(gif);
  return 0;
}

const char * isjpg(const char * name, int & result) {
  FILE * jpg = fopen(name, "rb");
  char buf[32];
  if (!jpg) {
    return "isjpg: Failure in fopen";
  }
  if (2 != fread(buf, 1, 2, jpg)) {
    fclose(jpg);
    return "isjpg: couldn't read magic number";
  }
  result = !memcmp(buf, "\xFF\xD8", 2);
  fclose(jpg);
  return 0;
}


static gdImagePtr loadImage(const char *iname) {
  printf("Load image: looking at %s\n", iname);
  string local = iname;
  char *fname = (char*)local.c_str();
  gdImagePtr img = NULL;
  FILE *fd = NULL;

  const char *suffix = strrchr(fname,'.');
  if (suffix==NULL) {
    int result = 0;
    isjpg(fname,result);
    if (result) { 
      suffix=".jpg"; 
    } else {
      ispng(fname,result);
      if (result) {
	suffix=".png";
      } else {
	isgif(fname,result);
	if (result) {
	  suffix=".gif";
	}
      }
    }
  }

  fd = fopen(fname,"rb");
  if (fd==NULL) {
    return NULL;
  }
  if (strcasecmp(suffix,".png")==0) {
    img = gdImageCreateFromPng(fd);
    if (img!=NULL) {
      gdImageAlphaBlending(img, 0);
    }
  } else if (strcasecmp(suffix,".jpg")*strcasecmp(suffix,".jpeg")==0) {
    img = gdImageCreateFromJpeg(fd);
  } else if (strcasecmp(suffix,".gif")==0) {
    img = gdImageCreateFromGif(fd);
    //} else if (strcasecmp(suffix,"bmp")==0) {
    //img = gdImageCreateFromBMP(fd);
  }
  fclose(fd);
  if (img!=NULL) return img;

  fprintf(stderr,"failed to load %s\n", fname);
  exit(1);
}


static bool gread(gdImagePtr& im, const char *fname) {
  im = loadImage(fname);
  return im!=NULL;
}


bool ImageLoader::load(const char *fname) {
  clear();
  return gread(im,fname);
}


ImageLoader::~ImageLoader() {
  clear();
}


void ImageLoader::clear() {
  if (im!=NULL) {
    gdImageDestroy(im);
    im = NULL;
  }
}


