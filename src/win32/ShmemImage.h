#ifndef SHMEM_IMAGE
#define SHMEM_IMAGE

#include "ShmemBus.h"
#include <yarp/sig/Image.h>

#include <yarp/os/begin_pack_for_net.h>
class ShmemImageHeader {
public:
  yarp::os::NetInt32 tick;
  yarp::os::NetInt32 w, h;
  yarp::os::NetInt32 blank[17];
} PACKED_FOR_NET;
#include <yarp/os/end_pack_for_net.h>

class ShmemImage {
public:
  ShmemImage(ShmemBus& bus) : bus(&bus) {
  }

  /**
   * Only valid between beginRead/endRead or beginWrite/endWrite
   */
  yarp::sig::ImageOf<yarp::sig::PixelRgb>& getImage() {
    if (bus->buffer()!=cp.getRawImage()) {
      cp.setQuantum(1);
      cp.setExternal((char*)(bus->buffer())+sizeof(ShmemImageHeader),320,240);
    }
    return cp;
  }

  ShmemImageHeader& getHeader() {
    return *((ShmemImageHeader*)((void*)bus->buffer()));
  }

private:
  yarp::sig::ImageOf<yarp::sig::PixelRgb> cp;
  ShmemBus *bus;
};


#endif
