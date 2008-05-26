#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "Effect.h"

#include <yarp/sig/Image.h>
#include <yarp/sig/ImageDraw.h>
#include <yarp/os/Time.h>

#include <string>

using namespace yarp::os;
using namespace yarp::sig;

using namespace std;


#include <ImageLoader.h>

#include "location.h"

/**
 *
 * An effect that places a still image over the video stream, 
 * respecting transparency.
 *
 */
class OverlayEffect : public Effect {
public:
  OverlayEffect() {
    options.put("overlay",
		(getResourceLocation()+"/"+"example.png").c_str());
    needRead = false;
    reconfigure(options);
  }

  virtual bool draw(ImageOf<PixelRgb>& src2, ImageOf<PixelRgb>& dest2);

  virtual std::string getName() {
    return "OverlayTV";
  }

  virtual bool reconfigure(yarp::os::Searchable& config) {
    options.fromString(config.toString());
    if (options.check("overlay")) {
      workOverlay = options.check("overlay",Value("")).toString().c_str();
    }
    needRead = true;
    return true;
  }

  virtual Property getConfiguration() {
    return options;
  }

  void readEffectData();

private:
  string workOverlay, readOverlay;
  Property options;
  ImageLoader img;
  bool needRead;
};

Effect *overlayRegister() {
  return new OverlayEffect();
}

bool OverlayEffect::draw(ImageOf<PixelRgb>& src2, ImageOf<PixelRgb>& dest2) {
  if (needRead) {
    readEffectData();
    needRead = false;
  }
  dest2 = src2;

  if (img.isValid()) {
    for (int x=0; x<dest2.width()&&x<img.width(); x++) {
      for (int y=0; y<dest2.height()&&y<img.height(); y++) {
	PixelBgra v = img.pixel(x,y);
	if (v.a==0) {
	  dest2(x,y) = PixelRgb(v.r,v.g,v.b);
	} else if (v.a>=127) {
	  // do nothing, leave copied value
	} else {
	  PixelRgb& o = dest2(x,y);
	  float f = v.a/127.0;
	  if (f>1) f = 1;
	  int r = (int)((1-f)*v.r+f*o.r);
	  int g = (int)((1-f)*v.g+f*o.g);
	  int b = (int)((1-f)*v.b+f*o.b);
	  dest2(x,y) = PixelRgb(r,g,b);
	}
      }
    }
  }


  return true;
}

void OverlayEffect::readEffectData() {
  if (workOverlay!="") {
    if (workOverlay!=readOverlay) {
      img.load(workOverlay.c_str());
      printf("overlay size %dx%d\n", img.width(), img.height());
      readOverlay = workOverlay;
    }
  }
}
