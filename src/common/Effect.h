#ifndef EFFECT_INC
#define EFFECT_INC

#include <yarp/sig/Image.h>
#include <yarp/os/Bottle.h>
#include <yarp/os/Property.h>
#include <yarp/dev/DeviceDriver.h>

#include <string>
#include <vector>

class Effect : public yarp::dev::DeviceDriver {
public:
  virtual bool open(yarp::os::Searchable& config) {
    return true;
  }

  virtual bool close() {
    return true;
  }

  virtual bool reconfigure(yarp::os::Searchable& config) {
    return true;
  }

  virtual yarp::os::Property getConfiguration() {
    return yarp::os::Property();
  }

  virtual bool draw(yarp::sig::ImageOf<yarp::sig::PixelRgb>& src,
		    yarp::sig::ImageOf<yarp::sig::PixelRgb>& dest) {
    return false;
  }

  virtual yarp::sig::Image *pdraw(yarp::sig::Image& src,
				  yarp::sig::Image& dest) {
    yarp::sig::ImageOf<yarp::sig::PixelRgb> src2;
    yarp::sig::ImageOf<yarp::sig::PixelRgb> dest2;
    bool cpIn = (src.getPixelCode()!=src2.getPixelCode());
    bool cpOut = (dest.getPixelCode()!=dest2.getPixelCode());
    if (cpIn) {
      src2.copy(src);
    } else {
      src2.wrapIplImage(src.getIplImage());
    }
    if (!cpOut) {
      dest.resize(src);
      dest2.wrapIplImage(dest.getIplImage());
    }
    if (!draw(src2,dest2)) {
      return NULL;
    }
    if (cpOut) {
      dest.copy(dest2);
    }
    return &dest;
  }

  /*
  virtual bool draw(yarp::sig::ImageOf<yarp::sig::PixelRgba>& src,
		    yarp::sig::ImageOf<yarp::sig::PixelRgba>& dest) {
    return false;
  }
  */

  virtual std::string getName() {
    return "anon";
  }

  virtual ~Effect() {
  }

  virtual bool start() {
  }

  virtual bool stop() {
  }
};

#endif




