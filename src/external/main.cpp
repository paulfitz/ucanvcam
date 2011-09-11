#include <stdio.h>
#include <windows.h>

#include <string>

#include "ShmemBus.h"

///////////////////////////////////////////////////////////////////////
//
// Basic test of shared memory bus - read/write strings
//

int write_test_string(const char *msg) {
  ShmemBus bus;
  bus.init();
  bus.beginWrite();
  char *base = (char *)bus.buffer();
  int len = bus.size();
  if ((int)strlen(msg)>=len) {
    printf("Message too long\n");
  } else {
    sprintf(base,"%s",msg);
    printf("Writing %s\n", msg);
  }
  bus.endWrite();
  bus.fini();
  return 0;
}

int read_test_string() {
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
  return 0;
}

///////////////////////////////////////////////////////////////////////
//
// Here's the code needed to write an image. 

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

// send 10 test images of dimension w * h
int write_test_image(int w, int h) {
  int tck = 0;
  ShmemBus bus;
  bus.init();
  for (int k=0; k<10; k++) {
	  bus.beginWrite();
	  char *base = (char *)bus.buffer();
	  ShmemImageHeader *header = (ShmemImageHeader *)base;
	  header->tick = tck;
	  tck++;
	  header->w = w;
	  header->h = h;
	  for (int i=0; i<17; i++) {
		header->blank[i] = 0;
	  }
	  PixelRgb *image = (PixelRgb *)(base+sizeof(ShmemImageHeader));
	  for (int y=0; y<h; y++) {
	    for (int x=0; x<w; x++) {
		  image->r = (x/(w/10))*25;
		  image->g = (y/(h/10))*25;
		  image->b = k*25;
		  image++;
		}
	  }
	  if (k==0) {
		  FILE *f = fopen("target.ppm","wb");
		  fprintf (f, "P6\n%d %d\n255\n", header->w, header->h);
		  fwrite (base+sizeof(ShmemImageHeader), header->w * 3 * header->h, 1, f);
		  fclose(f);
	  }
	  bus.endWrite();
	  Sleep(50);
  }

  bus.fini();
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc>=2) {
	std::string cmd = argv[1];
	if (cmd=="read") {
		return read_test_string();
	}
	if (cmd=="write"&&argc==3) {
	    return write_test_string(argv[2]);
	}
	if (cmd=="image"&&argc==4) {
	    return write_test_image(atoi(argv[2]),atoi(argv[3]));
	}
  }
  printf("Call as:\n");
  printf("testbus.exe read\n");
  printf("testbus.exe write \"string to write\"\n");
  printf("testbus.exe image 320 240\n");
  return 1;
}
