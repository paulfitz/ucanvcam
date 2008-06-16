#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "Effect.h"

#include <yarp/sig/Image.h>
#include <yarp/sig/ImageDraw.h>
#include <yarp/os/Time.h>

#include <gd.h>
#include <gdfontl.h>

#include "location.h"

#include <string>

using namespace yarp::os;
using namespace yarp::sig;

using namespace std;

/**
 *
 * A news-ticker effect.  Really needs more choice of fonts, it 
 * is specialized for a lovey-dovey look right now.  You can
 * change the text being scrolled at any time at runtime.
 *
 */
class TickerEffect : public Effect {
private:
  gdImageStruct access;
  ImageOf<PixelBgra> idest;
  Property options;
  string txt;
  string fnt;
public:
  TickerEffect() {
    access.sx = 0;
    access.sy = 0;
    options.put("text", "Welcome to ucanvcam!");
    options.put("font",
		(getResourceLocation()+"/"+"sybig.ttf").c_str());
    reconfigure(options);
  }

  //virtual void draw(ImageOf<PixelRgba>& src2, ImageOf<PixelRgba>& dest2);

  virtual yarp::sig::Image *pdraw(yarp::sig::Image& src,
				  yarp::sig::Image& dest);

  std::string getName() {
    return "TickerTV";
  }

  virtual bool reconfigure(yarp::os::Searchable& config) {
    options.fromString(config.toString());
    if (options.check("text")) {
      txt = options.check("text",Value("huh?")).toString().c_str();
    }
    if (options.check("font")) {
      fnt = options.check("font",Value("notset.ttf")).toString().c_str();
    }
    return true;
  }

  virtual Property getConfiguration() {
    return options;
  }
};

Effect *tickerRegister() {
  return new TickerEffect();
}


yarp::sig::Image *TickerEffect::pdraw(yarp::sig::Image& src,
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
  //char buf[1000];
  //sprintf(buf,"%s",getResourceLocation().c_str());
  //char *fnt = "/usr/share/fonts/truetype/ttf-larabie-uncommon/sybig___.ttf";
  //char *fnt = buf;
  int pt = 24;
  int xx = src.width()-(int)(offset+0.5);
  int yy = src.height()-10;

  for (int i=-1; i<=1; i++) {
    for (int j=-1; j<=1; j++) {
      //if (i!=0||j!=0) {
      if (i!=0&&j!=0) {
	gdImageStringFT(im, &brect[0], 
			gdTrueColorAlpha(10, 10, 10, 0),
			(char*)fnt.c_str(), pt, 0, xx+i, yy+j, (char*)txt.c_str());
      }
    }
  }

  gdImageStringFT(im, &brect[0], 
		  gdTrueColorAlpha(255, colr, 0, 0),
		  (char*)fnt.c_str(), pt, 0, xx, yy, (char*)txt.c_str());

  // we've gone off the left; wrap
  if (brect[2]<0) {
    first = now;
  }

  return &idest;
}



