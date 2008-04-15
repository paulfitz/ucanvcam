#ifndef EFFECTS_INC
#define EFFECTS_INC

#include "EffectGroup.h"

/**
 *
 * Standard interface to the available effects.
 * The Effects::apply method applies an effect selected using Effects::setEffect.
 * The apply method can be called with various image formats.
 *
 */
class Effects {
private:
public:
  Effects();

  virtual ~Effects() {
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
   * Get the name of the currently active effect.
   *
   */
  yarp::os::ConstString getCurrentEffect();

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

