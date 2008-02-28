
#ifndef VCAM_INC
#define VCAM_INC

#include <yarp/sig/Image.h>

#include "Effect.h"

class Vcam {
public:
  virtual bool isImage() = 0;

  virtual yarp::sig::Image *getImage() = 0;

  virtual bool getImage(yarp::sig::ImageOf<yarp::sig::PixelRgb>& img) {
    yarp::sig::Image *src = getImage();
    if (src!=0) {
      img.copy(*src);
      return true;
    }
    return false;
  }
};

#endif


