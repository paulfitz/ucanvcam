

///////////////////////////////////////////////////////////////////////////
//
//	fast semaphore
//
///////////////////////////////////////////////////////////////////////////

class VDAtomicInt {
protected:
	volatile int n;

public:
	VDAtomicInt() {}
	VDAtomicInt(int v) : n(v) {}

	bool operator!() const { return !n; }
	bool operator!=(volatile int v) const  { return n!=v; }
	bool operator==(volatile int v) const { return n==v; }
	bool operator<=(volatile int v) const { return n<=v; }
	bool operator>=(volatile int v) const { return n>=v; }
	bool operator<(volatile int v) const { return n<v; }
	bool operator>(volatile int v) const { return n>v; }

	///////////////////////////////

	/// Atomically exchanges a value with an integer in memory.
	static inline int staticExchange(volatile int *dst, int v) {
	  int old = *dst;
	  *dst = v;
	  return old;
	  //return (int)_InterlockedExchange((volatile long *)dst, v);
	  
	}

	/// Atomically adds one to an integer in memory.
	static inline void staticIncrement(volatile int *dst) {
	  //_InterlockedExchangeAdd((volatile long *)dst, 1);
	  (*dst)++;
	}

	/// Atomically subtracts one from an integer in memory.
	static inline void staticDecrement(volatile int *dst) {
	  //_InterlockedExchangeAdd((volatile long *)dst, -1);
	  (*dst)--;
	}

	/// Atomically subtracts one from an integer in memory and returns
	/// true if the result is zero.
	static inline bool staticDecrementTestZero(volatile int *dst) {
	  //return 1 == _InterlockedExchangeAdd((volatile long *)dst, -1);
	  (*dst)--;
	  return (*dst)==0;
	}

	/// Atomically adds a value to an integer in memory and returns the
	/// result.
	static inline int staticAdd(volatile int *dst, int v) {
	  //return (int)_InterlockedExchangeAdd((volatile long *)dst, v) + v;
	  *dst += v;
	  return *dst;
	}

	/// Atomically adds a value to an integer in memory and returns the
	/// old result (post-add).
	static inline int staticExchangeAdd(volatile int *dst, int v) {
	  //return _InterlockedExchangeAdd((volatile long *)dst, v);
	  int old = *dst;
	  *dst += v;
	  return old;
	}

	/// Atomically compares an integer in memory to a compare value and
	/// swaps the memory location with a second value if the compare
	/// succeeds. The return value is the memory value prior to the swap.
  //static inline int staticCompareExchange(volatile int *dst, int v, int compare) {
  //return _InterlockedCompareExchange((volatile long *)dst, v, compare);
  //}

	///////////////////////////////

	int operator=(int v) { return n = v; }

	int operator++()		{ return staticAdd(&n, 1); }
	int operator--()		{ return staticAdd(&n, -1); }
	int operator++(int)		{ return staticExchangeAdd(&n, 1); }
	int operator--(int)		{ return staticExchangeAdd(&n, -1); }
	int operator+=(int v)	{ return staticAdd(&n, v); }
	int operator-=(int v)	{ return staticAdd(&n, -v); }

  /*
#if _MSC_VER >= 1310
	void operator&=(int v)	{ _InterlockedAnd((volatile long *)&n, v); }	///< Atomic bitwise AND.
	void operator|=(int v)	{ _InterlockedOr((volatile long *)&n, v); }		///< Atomic bitwise OR.
	void operator^=(int v)	{ _InterlockedXor((volatile long *)&n, v); }	///< Atomic bitwise XOR.
#else
	/// Atomic bitwise AND.
	void operator&=(int v) {
		__asm mov eax,v
		__asm mov ecx,this
		__asm lock and dword ptr [ecx],eax
	}

	/// Atomic bitwise OR.
	void operator|=(int v) {
		__asm mov eax,v
		__asm mov ecx,this
		__asm lock or dword ptr [ecx],eax
	}

	/// Atomic bitwise XOR.
	void operator^=(int v) {
		__asm mov eax,v
		__asm mov ecx,this
		__asm lock xor dword ptr [ecx],eax
	}
#endif
  */
	operator int() const {
		return n;
	}

	/// Atomic exchange.
	int xchg(int v) {
		return staticExchange(&n, v);
	}

	// 486 only, but much nicer.  They return the actual result.

	int inc()			{ return operator++(); }				///< Atomic increment.
	int dec()			{ return operator--(); }				///< Atomic decrement.
	int add(int v)		{ return operator+=(v); }				///< Atomic add.

	// These return the result before the operation, which is more inline with
	// what XADD allows us to do.

	int postinc()		{ return operator++(0); }				///< Atomic post-increment.
	int postdec()		{ return operator--(0); }				///< Atomic post-decrement.
	int postadd(int v)	{ return staticExchangeAdd(&n, v); }	///< Atomic post-add.

};



template<typename T>
class VDAtomicPtr {
protected:
	T *volatile ptr;

public:
	VDAtomicPtr() {}
	VDAtomicPtr(T *p) : ptr(p) { }

	operator T*() const { return ptr; }
	T* operator->() const { return ptr; }

	T* operator=(T* p) {
		return ptr = p;
	}

	/// Atomic pointer exchange.
	T *xchg(T* p) {
	  if (ptr==p) { return p; }
	  T *tmp;
	  tmp = ptr;
	  ptr = p;
	  return tmp;
	    /*
#ifdef _M_AMD64
		return ptr == p ? p : (T *)_InterlockedExchangePointer((void *volatile *)&ptr, p);
#else
		return ptr == p ? p : (T *)_InterlockedExchange((volatile long *)&ptr, (long)p);
#endif
	    */
	}
};

// VDGetAccurateTick: Reads a timer with good precision and accuracy, in
// milliseconds. On Win9x, it has 1ms precision; on WinNT, it may have anywhere
// from 1ms to 10-15ms, although 1ms can be forced with timeBeginPeriod().
uint32 VDGetAccurateTick();

