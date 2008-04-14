
#include "Vcam.h"

#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/dev/all.h>

using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;

#include "drivers.h"

#include "Effect.h"

#include "modules/ISourceLister.h"

class VcamWinStandalone : public Vcam {
private:
  Network yarp;
  DriverCollection dev;
  PolyDriver source;
  IFrameGrabberImage *grabber;
  ISourceLister *lister;
  ImageOf<PixelRgb> cache, proc;
  Bottle sources;
public:
  VcamWinStandalone() {
    getList();
    open("test");
  }

  bool getList() {
    printf("Getting list\n");  fflush(stdout);
    sources.clear();
    sources.addString("test");
    Property pSource;
    pSource.put("device","vdub");
    pSource.put("passive","1");
    printf("Trying to open\n");  fflush(stdout);
    bool ok = source.open(pSource);
    lister = NULL;
    source.view(lister);
    if (lister!=NULL) {
      printf("Checking sources... (%ld)\n", (long int) lister);  fflush(stdout);
      //lister->getSources();
      sources.append(lister->getSources());
      printf("Done Checking sources... (%ld)\n", (long int) lister);  fflush(stdout);
    }
    source.close();
    return true;
    
  }

  bool open(const char *name) {
    grabber = NULL;
    source.close();
    Property pSource;
    pSource.put("device","vdub");
    
    pSource.put("source",name);
    //pSource.put("v4l",1);
    //pSource.put("v4ldevice","/dev/video0");
    //pSource.put("v4ldevice","/dev/video2");
    pSource.put("width",320);
    pSource.put("height",240);
    pSource.put("w",320);
    pSource.put("h",240);
    pSource.put("mode","ball");
    //pSource.put("source","/scratch/camera/dcim/135canon/mvi_3549.avi");
    bool ok = false;
    if (ConstString("test")!=name) {
      ok = source.open(pSource);
    }
    if (!ok) {
        pSource.put("device","test_grabber");
        pSource.put("width",320);
        pSource.put("height",240);
        pSource.put("mode","ball");
        source.open(pSource);
    }
    source.view(grabber);    
    source.view(lister);
    return true;
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
    Effect::apply(cache,proc);
    return &proc;
  }

  virtual bool getImage(ImageOf<PixelRgb>& img) {
    bool result = grabber->getImage(cache);
    if (!result) return false;
    Effect::apply(cache,img);
  }

  virtual yarp::os::Bottle getSources() {
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
  return new VcamWinStandalone;
}


