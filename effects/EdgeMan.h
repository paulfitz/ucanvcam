#ifndef EDGEMAN_INC
#define EDGEMAN_INC

#include <yarp/sig/Image.h>

class EdgeMan {
public:
  void Decorate(yarp::sig::ImageOf<yarp::sig::PixelRgb>& src,
		yarp::sig::ImageOf<yarp::sig::PixelRgb>& dest);
};

#endif
