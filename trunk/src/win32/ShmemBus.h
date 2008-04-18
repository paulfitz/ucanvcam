#ifndef SHMEMBUS_INC
#define SHMEMBUS_INC

#include <iostream>
#include <windows.h>
#include <tchar.h>
#define SHMEM_SIZE (1024*1024)
#define SHMEM_TIMEOUT 100
#define SHMEM_MAX_READERS 3


class ShmemBus {
public:
  
  bool init() {
    writeOk = false;
    readOk = false;
    readIndex = 0;
    for (int i=0; i<SHMEM_MAX_READERS; i++) {
      readEvent[i] = NULL;
    }
    writeEvent = NULL;
    shmemHandle = NULL;
    shmemBuffer = NULL;
    TCHAR readEventName[] = _T("ucanvcam_read");
    TCHAR writeEventName[] = _T("ucanvcam_write");
    TCHAR shmemName[] = _T("ucanvcam_shmem");

    TCHAR buf[256];
    for (int i=0; i<SHMEM_MAX_READERS; i++) {
      _stprintf(buf, _T("%s_%d"), readEventName, i);
      readEvent[i] = CreateEvent(NULL,false,true,buf);
      if (readEvent[i]==NULL) return false;
    }
    
    writeEvent = CreateEvent(NULL,false,true,writeEventName);
    if (writeEvent==NULL) return false;

    shmemHandle = CreateFileMapping(INVALID_HANDLE_VALUE,
				    NULL,
				    PAGE_READWRITE,
				    0,
				    SHMEM_SIZE,
				    shmemName);
     
    if (shmemHandle == NULL || shmemHandle == INVALID_HANDLE_VALUE) {
      return false;
    }

    shmemBuffer = MapViewOfFile(shmemHandle,
				FILE_MAP_ALL_ACCESS,
				0,
				0,
				SHMEM_SIZE);    
    if (shmemBuffer==NULL) return false;
  }

  bool fini() {
    for (int i=0; i<SHMEM_MAX_READERS; i++) {
      CloseHandle(readEvent[i]);
    }
    CloseHandle(writeEvent);
    UnmapViewOfFile(shmemBuffer);
    CloseHandle(shmemHandle);
  }

  bool beginRead() {
    bool done = false;
    readOk = false;
    readIndex = 0;
    while (!done) {
      DWORD res = WaitForSingleObject(writeEvent,INFINITE);
      if (res==WAIT_OBJECT_0) {
	for (int i=0; i<SHMEM_MAX_READERS; i++) {
	  res = WaitForSingleObject(readEvent[i], SHMEM_TIMEOUT);
	  if (res==WAIT_OBJECT_0) {
	    SetEvent(writeEvent);
	    // should now read ...
	    readOk = true;
	    readIndex = i;
	    done = true;
	    break;
	  }
	}
	if (!readOk) {
	  SetEvent(writeEvent);
	  Sleep(SHMEM_TIMEOUT);
	}
      }
    }
    return readOk;
  }

  bool endRead() {
    if (readOk) {
      readOk = false;
      SetEvent(readEvent[readIndex]);
      return true;
    }
    return false;
  }

  bool beginWrite() {
    writeOk = false;
    DWORD res = WaitForSingleObject(writeEvent,INFINITE);
    if (res==WAIT_OBJECT_0) {
      res = WaitForMultipleObjects(SHMEM_MAX_READERS,
				   readEvent,
				   TRUE,
				   INFINITE);
      if (res==WAIT_OBJECT_0) {
	writeOk = true;
      }
    }
    return writeOk;
  }

  bool endWrite() {
    if (writeOk) {
      writeOk = false;
      SetEvent(writeEvent);
      for (int i=0; i<SHMEM_MAX_READERS; i++) {
	SetEvent(readEvent[i]);
      }
      return true;
    }
    return false;
  }

  void *buffer() {
    return shmemBuffer;
  }

  int size() {
    return SHMEM_SIZE;
  }

private:
  HANDLE readEvent[SHMEM_MAX_READERS];
  HANDLE writeEvent;
  HANDLE shmemHandle;
  LPVOID shmemBuffer;
  bool readOk;
  bool writeOk;
  int readIndex;
};


#endif

