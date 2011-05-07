#ifndef MEMY_INC
#define MEMY_INC

#include <new> 
#include <stdlib.h>

void *operator new(size_t size); //throw(std::bad_alloc) 

void operator delete(void *p) throw();

void *operator new(size_t size, const std::nothrow_t &) throw();

void operator delete(void *p, const std::nothrow_t &) throw() ;

void *operator new[](size_t size); // throw(std::bad_alloc)

void operator delete[](void *p) throw();

void *operator new[](size_t size, const std::nothrow_t &) throw();

void operator delete[](void *p, const std::nothrow_t &) throw();


#endif
