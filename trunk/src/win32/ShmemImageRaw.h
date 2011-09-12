#ifndef SHMEM_IMAGE_INC
#define SHMEM_IMAGE_INC

#include "ShmemBus.h"

enum {
	IMGHDR_TICK = 0,
	IMGHDR_W = 1,
	IMGHDR_H = 2,
};

#define SIZE_IMGHDR (4*20)

#pragma warning (disable:4103)
#pragma pack(push, 1)
class ShmemImageHeader {
public:
	unsigned char raw[SIZE_IMGHDR];

	int get(int key) {
		int offset = key*4;
		unsigned int s = 256;
		return raw[offset]+s*(raw[offset+1]+s*(raw[offset+2]+s*raw[offset+3]));
	}
};
#pragma pack(pop)

class ShmemImageRaw {
public:
	ShmemImageRaw(ShmemBus& bus) : bus(&bus) {
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
