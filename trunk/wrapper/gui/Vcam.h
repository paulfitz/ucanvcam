
#ifndef VCAM_INC
#define VCAM_INC

#include <yarp/sig/Image.h>
#include <yarp/os/Bottle.h>

#include "Effect.h"

class Vcam {
public:
  virtual ~Vcam() {}

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
  
  //virtual bool close() {}

  virtual yarp::os::Bottle getSources() {
    return yarp::os::Bottle();
  }

  virtual void setSource(const char *name) {
    printf("Cannot switch source to %s or anything else\n", name);
  }

  virtual yarp::os::ConstString guessSource() {
    return "";
  }
};

#endif


