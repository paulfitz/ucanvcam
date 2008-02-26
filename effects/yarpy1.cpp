#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "yarpy1.h"
#include "yeffects.h"

#include <yarp/sig/Image.h>
#include <yarp/sig/ImageDraw.h>
#include <yarp/os/Time.h>

using namespace yarp::os;
using namespace yarp::sig;

#include "EdgeMan.h"

//#include <yarp/begin_pack_for_net.h>
//class YarpPixelRGBA {
//public:
//  char b, g, r, a;
//};
//#include <yarp/end_pack_for_net.h>

class Yarpy1Effect : public YarpEffect {
  virtual bool draw(ImageOf<PixelRgb>& src2, ImageOf<PixelRgb>& dest2);

  virtual std::string getName() {
    return "EngageTV";
  }
};

YarpEffect *ypaulfitzRegister() {
  return new Yarpy1Effect();
}

bool Yarpy1Effect::draw(ImageOf<PixelRgb>& src2, ImageOf<PixelRgb>& dest2) {
  static EdgeMan edgy;
  edgy.Decorate(src2,dest2);
  return true;
}

extern "C" void yarpy(void *src, void *dest,
		      int w, int h, 
		      int area, int pix_size) {
  static double now = Time::now();
  //memcpy(dest, src, area * pix_size);
  //printf("%g %d %d\n", now, sizeof(YarpPixelRGBA), pix_size);
  /*
  YARPImageOf<YarpPixelRGBA> img;
  img.UncountedRefer(src,w,h);
  static int index = 0;
  for (int i=0; i<w; i++) {
    img.SafePixel(i,index).r = 255;
  }
  index++;
  if (index>=h) {
    index = 0;
  }
  */
  ImageOf<PixelRgba> src1,dest1;
  src1.setExternal(src,w,h);
  dest1.setExternal(dest,w,h);
  static ImageOf<PixelRgb> src2,dest2;
  src2.resize(src1);
  dest2.resize(dest1);
  IMGFOR(src2,x,y) {
    PixelRgba& pix1 = src1(x,y);
    PixelRgb& pix2 = src2(x,y);
    pix2.r = pix1.r;
    pix2.g = pix1.g;
    pix2.b = pix1.b;
  }
  static EdgeMan edgy;
  edgy.Decorate(src2,dest2);
  IMGFOR(dest2,x,y) {
    PixelRgb& pix1 = dest2(x,y);
    PixelRgba& pix2 = dest1(x,y);
    pix2.r = pix1.r;
    pix2.g = pix1.g;
    pix2.b = pix1.b;
  }
}

