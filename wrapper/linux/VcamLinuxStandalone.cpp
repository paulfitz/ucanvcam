
#include "Vcam.h"

#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/dev/all.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;

#include "drivers.h"

#include "yarpy.h"

class VcamLinuxStandalone : public Vcam {
private:
  Network yarp;
  DriverCollection dev;
  PolyDriver source;
  IFrameGrabberImage *grabber;
  ImageOf<PixelRgb> cache, proc;
  Bottle sources;
public:
  VcamLinuxStandalone() {
    open("/dev/video0");
  }

  bool open(const char *name) {
    grabber = NULL;
    source.close();
    Property pSource;
    pSource.put("device","ffmpeg_grabber");
    pSource.put("v4l",1);
    //pSource.put("v4ldevice","/dev/video0");
    pSource.put("v4ldevice",name);
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

  virtual bool close() {
    source.close();
  }

  virtual yarp::os::Bottle getSources() {
    sources.clear();
    sources.addString("test");
    for (int i=0; i<10; i++) {
      char buf[256];
      sprintf(buf,"/dev/video%d", i);
      struct stat s;
      int r = stat(buf,&s);
      if (r>=0) {
	sources.addString(buf);
	printf("adding source %s\n", buf);
      } else {
	printf("do not have %s\n", buf);
      }
    }
    return sources;
  }

  virtual void setSource(const char *name) {
    if (sources.size()<1) {
      getSources();
    }
    printf("Should switch source to %s\n", name);
    open(name);
  }
};



Vcam *getVcam() {
  return new VcamLinuxStandalone;
}


