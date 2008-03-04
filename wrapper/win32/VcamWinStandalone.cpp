
#include "Vcam.h"

#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/dev/all.h>

using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;

#include "drivers.h"

#include "yarpy.h"

class VcamWinStandalone : public Vcam {
private:
  Network yarp;
  DriverCollection dev;
  PolyDriver source;
  IFrameGrabberImage *grabber;
  ImageOf<PixelRgb> cache, proc;
public:
  VcamWinStandalone() {
    Property pSource;
    pSource.put("device","ffmpeg_grabber");
    pSource.put("v4l",1);
    //pSource.put("v4ldevice","/dev/video0");
    pSource.put("v4ldevice","/dev/video2");
    pSource.put("width",320);
    pSource.put("height",240);
    pSource.put("w",320);
    pSource.put("h",240);
    //pSource.put("source","/scratch/camera/dcim/135canon/mvi_3549.avi");
    bool ok = source.open(pSource);
    if (!ok) {
        pSource.put("device","test_grabber");
        pSource.put("width",320);
        pSource.put("height",240);
        pSource.put("mode","ball");
        source.open(pSource);
    }
    source.view(grabber);
  }

  virtual bool isImage() {
    if (grabber==NULL) {
      return false;
    }
    return true; // not quite true...
  }

  virtual yarp::sig::Image *getImage() {
    bool result = grabber->getImage(cache);
    if (!result) return NULL;
    yarpy_apply(cache,proc);
    return &proc;
  }

  virtual bool getImage(ImageOf<PixelRgb>& img) {
    bool result = grabber->getImage(cache);
    if (!result) return false;
    yarpy_apply(cache,img);
  }
};



static VcamWinStandalone my_vcam;

Vcam& getVcam() {
    return my_vcam;
}


