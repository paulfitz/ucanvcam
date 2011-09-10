#ifndef EFFECT_INC
#define EFFECT_INC

#include <yarp/sig/Image.h>
#include <yarp/os/Bottle.h>
#include <yarp/os/Property.h>
#include <yarp/dev/DeviceDriver.h>

#include <string>
#include <vector>

/**
 *
 * Base class for all effects.  For convenience, we treat effects
 * as DeviceDrivers in YARP -- entities that can be created
 * by name, and passed configuration information at runtime.
 *
 */
class Effect : public yarp::dev::DeviceDriver {
public:

  /**
   *
   * Configure an effect, and prepare it to run.
   *
   */
  virtual bool open(yarp::os::Searchable& config) {
    return true;
  }

  /**
   * Shut an effect down.
   */
  virtual bool close() {
    return true;
  }

  /**
   * Reconfigure an effect while it is running.
   */
  virtual bool reconfigure(yarp::os::Searchable& config) {
    return true;
  }

  /**
   * Recover the configuration of an effect.
   */
  virtual yarp::os::Property getConfiguration() {
    return yarp::os::Property();
  }

  /**
   * Modify an image in an effect-specific way.  This is an RGB-to-RGB
   * specialization of Effect::pdraw - it should return false if there
   * is no such specialization.
   */
  virtual bool draw(yarp::sig::ImageOf<yarp::sig::PixelRgb>& src,
		    yarp::sig::ImageOf<yarp::sig::PixelRgb>& dest) {
    return false;
  }

  /**
   * Modify an image in any format in an effect-specific way, producing
   * an output in any format.
   */
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
    //if (!draw(src2,dest2)) {
    //return NULL;
    //}
    draw(src2,dest2);
    if (cpOut) {
      dest.copy(dest2);
    }
    return &dest;
  }

  /**
   * Get the name of an effect.
   */
  virtual std::string getName() {
    return "anon";
  }

  virtual ~Effect() {
  }

  /**
   * Prepare the effect for operation.
   */
  virtual bool start() {
	  return true;
  }

  /**
   * Prepare the effect to cease operation.  It may be started again without
   * a close/open cycle.
   */
  virtual bool stop() {
	  return true;
  }
};

#endif




