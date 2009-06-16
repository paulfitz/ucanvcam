#include <stdio.h>
#include <windows.h>

#include "ShmemBus.h"

///////////////////////////////////////////////////////////////////////
//
// Basic test of shared memory bus - read/write strings
//

void write_test_string(const char *msg) {
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

void read_test_string() {
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

///////////////////////////////////////////////////////////////////////
//
// Here's the code needed to write an image. 
// Not tested though!  (you can't even call it in this test program).
// The code uses C-style casting that would be tricky in other
// languages.  No problem, just make sure the same bytes get written
// in the order given here.
//

// Use packing to make sure no gaps are inserted in structure 
// during optimization
#pragma warning (disable:4103)
#pragma pack(push, 1)
class ShmemImageHeader {
public:
  DWORD tick;
  DWORD w, h;
  DWORD blank[17];
};
class PixelRgb {
public:
  unsigned char r, g, b;
};
#pragma pack(pop)

void write_test_image() {
  ShmemBus bus;
  bus.init();
  bus.beginWrite();
  char *base = (char *)bus.buffer();
  ShmemImageHeader *header = (ShmemImageHeader *)base;
  header->tick = 0; // to work, this will need to increment, or at least change
  header->w = 320;
  header->h = 240;
  for (int i=0; i<17; i++) {
    header->blank[i] = 0;
  }
  PixelRgb *image = (PixelRgb *)(base+sizeof(ShmemImageHeader));
  for (int w=0; w<320; w++) {
    for (int h=0; h<240; h++) {
      image->r = w % 256;
      image->g = h % 256;
      image->b = (w+h) % 256;
      image++;
    }
  }

  FILE *f = fopen("target.ppm","wb");
  fprintf (f, "P6\n%d %d\n255\n", header->w, header->h);
  fwrite (base+sizeof(ShmemImageHeader), header->w * 3 * header->h, 1, f);
  fclose(f);

  bus.endWrite();
  bus.fini();
}

int main(int argc, char *argv[]) {
  if (argc<2) {
    read_test_string();
  } else if (argc==2) {
    write_test_string(argv[1]);
  } else {
    write_test_image();
  }
  return 0;
}
