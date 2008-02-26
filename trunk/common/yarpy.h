
#include <yarp/sig/Image.h>

extern bool yarpy_stop();
extern void yarpy_init(yarp::sig::Image& in);

extern bool yarpy_apply(yarp::sig::ImageOf<yarp::sig::PixelRgb>& in, 
			yarp::sig::ImageOf<yarp::sig::PixelRgb>& out);

extern bool yarpy_apply(yarp::sig::ImageOf<yarp::sig::PixelBgr>& in, 
			yarp::sig::ImageOf<yarp::sig::PixelBgr>& out);

extern bool yarpy_apply(yarp::sig::ImageOf<yarp::sig::PixelRgb>& in, 
			yarp::sig::ImageOf<yarp::sig::PixelBgr>& out);




