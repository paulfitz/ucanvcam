#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "yarpy1.h"
#include "yeffects.h"

#include <yarp/sig/Image.h>
#include <yarp/sig/ImageDraw.h>
#include <yarp/os/Time.h>

#include <gd.h>
#include <gdfontl.h>

#include "location.h"

using namespace yarp::os;
using namespace yarp::sig;

class YarpyTickerEffect : public YarpEffect {
private:
  gdImageStruct access;
  ImageOf<PixelBgra> idest;
public:
  YarpyTickerEffect() {
    access.sx = 0;
    access.sy = 0;
  }

  //virtual void draw(ImageOf<PixelRgba>& src2, ImageOf<PixelRgba>& dest2);

  virtual yarp::sig::Image *pdraw(yarp::sig::Image& src,
				  yarp::sig::Image& dest);

  std::string getName() {
    return "TickerTV";
  }
};

YarpEffect *ytickerRegister() {
  return new YarpyTickerEffect();
}


yarp::sig::Image *YarpyTickerEffect::pdraw(yarp::sig::Image& src,
					   yarp::sig::Image& dest) {

  gdImagePtr im;

  if (src.width()!=access.sx ||
      src.height()!=access.sy) {

    gdImagePtr im2;
    im2 = gdImageCreateTrueColor(src.width(),src.height());
    access = *im2;
    gdImageDestroy(im2);
  }

  idest.copy(src);
  access.tpixels = (int**)idest.getRowArray();
  access.sx = idest.width();
  access.sy = idest.height();

  im = &access;

  int margin = 50;
  
  /* Now draw a solid white rectangle in the middle of the image. */
  //gdImageFilledRectangle(im, 24, 24, 300, 300, 
  //gdTrueColorAlpha(0, 255, 255, 128));

  static double first = Time::now();
  static double last = first;
  double now = Time::now();
  double offset = (now-first)*src.width()/4.0;
  //printf("offset %g\n", offset);
  last = now;

  int colr = ((int)(offset*2))%512-256;
  if (colr<0) colr = -colr;

  //#ifndef WIN32
  //gdFTStringExtra config;
  //  config.flags = gdFTEX_XSHOW;

  int brect[8];
  //"/font/comic.ttf",
  char buf[1000];
  sprintf(buf,"%s/sybig.ttf",getResourceLocation().c_str());
  //char *fnt = "/usr/share/fonts/truetype/ttf-larabie-uncommon/sybig___.ttf";
  char *fnt = buf;
  int pt = 24;
  int xx = src.width()-(int)(offset+0.5);
  int yy = src.height()-10;
  char *txt = "Happy VALENTINE's day!";

  for (int i=-1; i<=1; i++) {
    for (int j=-1; j<=1; j++) {
      //if (i!=0||j!=0) {
      if (i!=0&&j!=0) {
	gdImageStringFT(im, &brect[0], 
			gdTrueColorAlpha(10, 10, 10, 0),
			fnt, pt, 0, xx+i, yy+j, txt);
      }
    }
  }

  gdImageStringFT(im, &brect[0], 
		  gdTrueColorAlpha(255, colr, 0, 0),
		  fnt, pt, 0, xx, yy, txt);

  // we've gone off the left; wrap
  if (brect[2]<0) {
    first = now;
  }



  //#else
  /*
      gdImageString(im, gdFontLarge,
		src.width()-(int)(offset+0.5), 20, (unsigned char*)"Hello, you fool, I love you.  Come on the joy ride.  I said: Hello, you fool, I love you.  Come on the joy ride!", 
		gdTrueColorAlpha(255, 255, 0, 0));
      
      if (now-first>10) {
	first=now;
      }
  */

  //#endif

  /*
  dest2.resize(base);
  IMGFOR(base,x,y) {
    PixelRgb& pix1 = dest2.safePixel(x,y);
    PixelBgr *pix0 = (PixelBgr*)(im->tpixels[y] + x);
    //pix1 = (*pix0);
    if (pix0->r != 0 || pix0->g != 0 || pix0->b != 0) {
      pix1.r = pix0->r;
      pix1.g = pix0->g;
      pix1.b = pix0->b;
    }
    //gdImageGetPixel(im,x,y);
  }
  */

  //dest2.copy(src2);

  /*
  if (im->tpixels==NULL) {
    printf("no image\n");
    //    exit(1);
  }

  int start = src2.height()-margin;
  IMGFOR(dest2,x,y) {
    if (y>=start) {
      PixelRgb& pix1 = dest2.safePixel(x,y);
      PixelBgr *pix0 = (PixelBgr*)(im->tpixels[y-start] + x);
      //pix1 = (*pix0);
      if (pix0->r != 0 || pix0->g != 0 || pix0->b != 0) {
	pix1.r = pix0->r;
	pix1.g = pix0->g;
	pix1.b = pix0->b;
      }
    }
  }
  */
  //gdImageDestroy(im);

  return &idest;
}



