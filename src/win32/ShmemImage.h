#ifndef SHMEM_IMAGE
#define SHMEM_IMAGE

#include "ShmemBus.h"

#ifndef SKIP_YARP

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


#else

// stripped down version to avoid YARP dependency

enum {
  IMGHDR_TICK,
  IMGHDR_W,
  IMGHDR_H,
};

#define SIZE_IMGHDR (4*20)

class ShmemImageHeader {
public:
  unsigned char raw[SIZE_IMGHDR];

  int get(int key) {
    int offset = key*4;
    unsigned int s = 256;
    return raw[offset]+s*(raw[offset+1]+s*(raw[offset+2]+s*raw[offset+3]));
  }
};

class ShmemImage {
public:
  ShmemImage(ShmemBus& bus) : bus(&bus) {
  }

  /**
   * Only valid between beginRead/endRead or beginWrite/endWrite
   */
  char *getImage() {
    if (bus->buffer()==NULL) return NULL;
    return (char*)(bus->buffer())+SIZE_IMGHDR;
  }

  bool getHeader(ShmemImageHeader& hdr) {
    memcpy((void*)hdr.raw,(void*)bus->buffer(),SIZE_IMGHDR);
    return true;
  }

private:
  ShmemBus *bus;
};

#endif

#endif
