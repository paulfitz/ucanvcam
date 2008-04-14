#ifndef EFFECT_INC
#define EFFECT_INC

// put an API around all the messy code, which should then
// get cleaned up step-by-step

#include "yeffects.h"

/**
 *
 * Standard interface to the available effects.
 * The Effect::apply method applies an effect selected using Effect::setEffect.
 * The apply method can be called with various image formats.
 *
 */
class Effect {
private:
public:
  Effect();

  virtual ~Effect() {
    stop();
  }

  /**
   *
   * Get a list of the available effects.
   *
   */
  yarp::os::Bottle getEffects();

  /**
   *
   * Choose a named effect.
   *
   */
  bool setEffect(const char *name);

  /**
   *
   * Apply the current effect to an image, RGB version.
   *
   */
  static bool apply(yarp::sig::ImageOf<yarp::sig::PixelRgb>& in, 
		    yarp::sig::ImageOf<yarp::sig::PixelRgb>& out);

  /**
   *
   * Apply the current effect to an image, BGR version.
   *
   */
  static bool apply(yarp::sig::ImageOf<yarp::sig::PixelBgr>& in, 
		    yarp::sig::ImageOf<yarp::sig::PixelBgr>& out);

  /**
   *
   * Apply the current effect to an image, RGB to BGR version.
   *
   */
  static bool apply(yarp::sig::ImageOf<yarp::sig::PixelRgb>& in, 
		    yarp::sig::ImageOf<yarp::sig::PixelBgr>& out);
  
  /**
   *
   * Get the configuration of the current effect.
   *
   */
  yarp::os::Property getConfiguration();

  /**
   *
   * Reconfigure the current effect.
   *
   */
  void setConfiguration(yarp::os::Property& prop);

private:
  void stop();
};

#endif

