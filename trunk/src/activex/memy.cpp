
#ifndef POINTER_64
#define POINTER_64 __ptr64
#endif
#include <windows.h>

#include "YarpOut.h"
#include "memy.h"


#if 1
void *operator new(size_t size) //throw(std::bad_alloc) 
{ 
  YarpOut yarp_out;
  yarp_out.init("new",true);
  yarp_out.say("new!",size,0);
 
  void *p = LocalAlloc(LPTR,size); //malloc(size); 

   if (!p) 
   { 
     //throw std::bad_alloc(); 
   } 

   return p; 
} 

void operator delete(void *p) throw() 
{ 
  LocalFree(p);
  //free(p); 
} 

void *operator new(size_t size, const std::nothrow_t &) throw() 
{ 
  YarpOut yarp_out;
  yarp_out.init("new",true);
  yarp_out.say("new 2!",size,0);
  return LocalAlloc(LPTR,size); //malloc(size); 
} 

void operator delete(void *p, const std::nothrow_t &) throw() 
{ 
  LocalFree(p);
  //free(p); 
} 

void *operator new[](size_t size) //throw(std::bad_alloc) 
{ 
  YarpOut yarp_out;
  yarp_out.init("new",true);
  yarp_out.say("new 3!",size,0);

  void *p = LocalAlloc(LPTR,size); //malloc(size); 

   if (!p) 
   { 
     //throw std::bad_alloc(); 
   } 

   return p; 
} 

void operator delete[](void *p) throw() 
{ 
  LocalFree(p);
  //free(p); 
} 

void *operator new[](size_t size, const std::nothrow_t &) throw() 
{ 
  return LocalAlloc(LPTR,size); //malloc(size); 
} 

void operator delete[](void *p, const std::nothrow_t &) throw() 
{ 
  LocalFree(p);
  //free(p); 
}

#endif

