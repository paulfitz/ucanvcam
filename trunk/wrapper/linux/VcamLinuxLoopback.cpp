
#include "Vcam.h"


class VcamLinuxLoopback : public Vcam {
public:
  virtual bool isImage();

  virtual yarp::sig::Image *getImage();
};



static VcamLinuxLoopback my_vcam;

Vcam& getVcam() {
    return my_vcam;
}

