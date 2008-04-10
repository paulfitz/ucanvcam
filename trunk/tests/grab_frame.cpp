
#include <stdio.h>

#include "ShmemBus.h"

#include <yarp/os/all.h>
#include <yarp/sig/all.h>
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::sig::file;

int main(int argc, char* argv[])
{
  Network yarp;

  ShmemBus bus;
  bus.init();
  if (argc>1) {
    for (int i=1; i<argc; i++ ) {
      char *txt = argv[i];
      printf("Writing [%s]\n", txt);
      bool ok = bus.beginWrite();
      if (!ok) { printf("Failure to write!\n"); exit(1); }
      strcpy((char*)bus.buffer(),txt);
      bus.endWrite();
      Time::delay(1);
    }
  } else {
    printf("Reading...\n");
    bool ok = bus.beginRead();
    if (!ok) { printf("Failure to read!\n"); exit(1); }
    long int tot = 0;
    for (int i=0; i<bus.size(); i++) {
      tot += i*(((unsigned char*)bus.buffer())[i]);
    }
    printf("Read [%ld]\n", tot);
    ImageOf<PixelRgb> cache;
    cache.setQuantum(1);
    cache.setExternal(bus.buffer(),320,240);
    write(cache,"test.ppm");
    //printf("Read [%s]\n", (char*)bus.buffer());
    bus.endRead();
  }
  bus.fini();

  return 0;
}
