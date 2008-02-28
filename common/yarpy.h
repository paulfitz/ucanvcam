
#include <yarp/sig/Image.h>

#include "yeffects.h"

extern bool yarpy_stop();
extern void yarpy_init(yarp::sig::Image& in);

extern bool yarpy_apply(yarp::sig::ImageOf<yarp::sig::PixelRgb>& in, 
			yarp::sig::ImageOf<yarp::sig::PixelRgb>& out);

extern bool yarpy_apply(yarp::sig::ImageOf<yarp::sig::PixelBgr>& in, 
			yarp::sig::ImageOf<yarp::sig::PixelBgr>& out);

extern bool yarpy_apply(yarp::sig::ImageOf<yarp::sig::PixelRgb>& in, 
			yarp::sig::ImageOf<yarp::sig::PixelBgr>& out);


extern YarpEffect *yarpy_get_effect();

extern YarpEffect *yarpy_take_effect();

extern bool yarpy_set_effect(YarpEffect *next);



