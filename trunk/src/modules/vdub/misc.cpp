
#include <windows.h>
#include <mmsystem.h>

#include <vd2/system/vdtypes.h>

#include <math.h>

uint32 VDGetAccurateTick() {
	return timeGetTime();
}

int VDRoundToInt(double x) {
	return (int)floor(x + 0.5);
}
