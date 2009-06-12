#include <stdio.h>
#include <windows.h>

#include "ShmemBus.h"

/*

To send an image:

Modify write_test to put data in the buffer in this format:

  A four byte little-endian integer holding a count that increments
  A four byte little-endian integer holding the number 320 (width)
  A four byte little-endian integer holding the number 240 (height)
  A zero-filled 17*4 byte sequence
  A 320*240*3 byte sequence holding your image in RGBRGBRGBRGB... format

*/

void write_test(const char *msg) {
  ShmemBus bus;
  bus.init();
  bus.beginWrite();
  char *base = (char *)bus.buffer();
  int len = bus.size();
  if (strlen(msg)>=len) {
    printf("Message too long\n");
  } else {
    sprintf(base,"%s",msg);
    printf("Writing %s\n", msg);
  }
  bus.endWrite();
  bus.fini();
}

void read_test() {
  ShmemBus bus;
  bus.init();
  for (int i=0; i<100; i++) {
    bus.beginRead();
    char *base = (char *)bus.buffer();
    printf("Read [%s]\n", base);
    bus.endRead();
    Sleep(1000);
  }
  bus.fini();
}

int main(int argc, char *argv[]) {
  if (argc<2) {
    read_test();
  } else {
    write_test(argv[1]);
  }
  return 0;
}
