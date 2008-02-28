
#include <stdio.h>

#include "Vcam.h"

#include <yarp/sig/Image.h>

using namespace yarp::sig;

class VcamDummy {
private:
  ImageOf<PixelRgb> img;
  int ct;

public:
  VcamDummy() {
    ct = 0;
  }

  virtual bool isImage() {
    return true;
  }

  virtual yarp::sig::Image *getImage() {
    if (img.width()==0) {
      img.resize(320,240);
      img.setQuantum(1);
      img.zero();
    }
    img.zero();
    for (int x=0; x<img.width(); x++) {
      img(x,ct%img.height()).r = 255;
    }
    ct++;
    return &img;
  }
};

static VcamDummy my_vcam;

Vcam& getVcam() {
    return my_vcam;
}


