//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2004 Avery Lee, All Rights Reserved.
//
//	Beginning with 1.6.0, the VirtualDub system library is licensed
//	differently than the remainder of VirtualDub.  This particular file is
//	thus licensed as follows (the "zlib" license):
//
//	This software is provided 'as-is', without any express or implied
//	warranty.  In no event will the authors be held liable for any
//	damages arising from the use of this software.
//
//	Permission is granted to anyone to use this software for any purpose,
//	including commercial applications, and to alter it and redistribute it
//	freely, subject to the following restrictions:
//
//	1.	The origin of this software must not be misrepresented; you must
//		not claim that you wrote the original software. If you use this
//		software in a product, an acknowledgment in the product
//		documentation would be appreciated but is not required.
//	2.	Altered source versions must be plainly marked as such, and must
//		not be misrepresented as being the original software.
//	3.	This notice may not be removed or altered from any source
//		distribution.

#ifndef VD2_SYSTEM_VDSTL_H
#define VD2_SYSTEM_VDSTL_H

#ifdef _MSC_VER
	#pragma once
#endif

#include <limits>

#ifndef MAX_INT
#define MAX_INT    0x7ffffff
#define length_error string("length error")
#endif

#include <vd2/system/vdtypes.h>
#include <vd2/system/memory.h>

///////////////////////////////////////////////////////////////////////////
//
//	glue
//
///////////////////////////////////////////////////////////////////////////

template<class Category, class T, class Distance = ptrdiff_t, class Pointer = T*, class Reference = T&>
struct vditerator {
#if defined(VD_COMPILER_MSVC) && (VD_COMPILER_MSVC < 1310 || (defined(VD_COMPILER_MSVC_VC8_PSDK) || defined(VD_COMPILER_MSVC_VC8_DDK)))
	typedef std::iterator<Category, T, Distance> type;
#else
	typedef std::iterator<Category, T, Distance, Pointer, Reference> type;
#endif
};

template<class Iterator, class T>
struct vdreverse_iterator {
#if defined(VD_COMPILER_MSVC) && (VD_COMPILER_MSVC < 1310 || (defined(VD_COMPILER_MSVC_VC8_PSDK) || defined(VD_COMPILER_MSVC_VC8_DDK)))
	typedef std::reverse_iterator<Iterator, T> type;
#else
	typedef std::reverse_iterator<Iterator> type;
#endif
};

///////////////////////////////////////////////////////////////////////////

template<class T, unsigned kDeadZone = 16>
class vddebug_alloc {
public:
	typedef	size_t		size_type;
	typedef	ptrdiff_t	difference_type;
	typedef	T*			pointer;
	typedef	const T*	const_pointer;
	typedef	T&			reference;
	typedef	const T&	const_reference;
	typedef	T			value_type;

	template<class U> struct rebind { typedef vddebug_alloc<U, kDeadZone> other; };

	pointer			address(reference x) const			{ return &x; }
	const_pointer	address(const_reference x) const	{ return &x; }

	pointer allocate(size_type n, void *p_close = 0) {
		pointer p = (pointer)VDAlignedMalloc(n*sizeof(T) + 2*kDeadZone, 16);

		if (!p)
			return p;

		memset((char *)p, 0xa9, kDeadZone);
		memset((char *)p + kDeadZone + n*sizeof(T), 0xa9, kDeadZone);

		return (pointer)((char *)p + kDeadZone);
	}

	void deallocate(pointer p, size_type n) {
		char *p1 = (char *)p - kDeadZone;
		char *p2 = (char *)p + n*sizeof(T);

		for(uint32 i=0; i<kDeadZone; ++i) {
			VDASSERT(p1[i] == (char)0xa9);
			VDASSERT(p2[i] == (char)0xa9);
		}

		VDAlignedFree(p1);
	}

	size_type		max_size() const throw()			{ return MAX_INT - 2*kDeadZone; }

	void			construct(pointer p, const T& val)	{ new((void *)p) T(val); }
	void			destroy(pointer p)					{ ((T*)p)->~T(); }

#if defined(_MSC_VER) && _MSC_VER < 1300
	char *			_Charalloc(size_type n)				{ return rebind<char>::other::allocate(n); }
#endif
};

///////////////////////////////////////////////////////////////////////////

template<class T, unsigned kAlignment = 16>
class vdaligned_alloc {
public:
	typedef	size_t		size_type;
	typedef	ptrdiff_t	difference_type;
	typedef	T*			pointer;
	typedef	const T*	const_pointer;
	typedef	T&			reference;
	typedef	const T&	const_reference;
	typedef	T			value_type;

	template<class U> struct rebind { typedef vdaligned_alloc<U, kAlignment> other; };

	pointer			address(reference x) const			{ return &x; }
	const_pointer	address(const_reference x) const	{ return &x; }

	pointer			allocate(size_type n, void *p = 0)	{ return (pointer)VDAlignedMalloc(n*sizeof(T), kAlignment); }
	void			deallocate(pointer p, size_type n)	{ VDAlignedFree(p); }
	size_type		max_size() const throw()			{ return MAX_INT; }

	void			construct(pointer p, const T& val)	{ new((void *)p) T(val); }
	void			destroy(pointer p)					{ ((T*)p)->~T(); }

#if defined(_MSC_VER) && _MSC_VER < 1300
	char *			_Charalloc(size_type n)				{ return rebind<char>::other::allocate(n); }
#endif
};

///////////////////////////////////////////////////////////////////////////
//
//	vdblock
//
//	vdblock<T> is similar to vector<T>, except:
//
//	1) May only be used with POD types.
//	2) No construction or destruction of elements is performed.
//	3) Capacity is always equal to size, and reallocation is performed
//	   whenever the size changes.
//	4) Contents are undefined after a reallocation.
//	5) No insertion or deletion operations are provided.
//
///////////////////////////////////////////////////////////////////////////

template<class T, class A = std::allocator<T> >
class vdblock : protected A {
public:
	typedef	T									value_type;
	typedef	typename A::pointer					pointer;
	typedef	typename A::const_pointer			const_pointer;
	typedef	typename A::reference				reference;
	typedef	typename A::const_reference			const_reference;
	typedef	size_t								size_type;
	typedef	ptrdiff_t							difference_type;
	typedef	pointer								iterator;
	typedef	const_pointer						const_iterator;
	typedef typename vdreverse_iterator<iterator, T>::type			reverse_iterator;
	typedef typename vdreverse_iterator<const_iterator, const T>::type	const_reverse_iterator;

	vdblock(const A& alloc = A()) : A(alloc), mpBlock(NULL), mSize(0) {}
	vdblock(size_type s, const A& alloc = A()) : A(alloc), mpBlock(A::allocate(s, 0)), mSize(s) {}
	~vdblock() {
		if (mpBlock)
			A::deallocate(mpBlock, mSize);
	}

	reference				operator[](size_type n)			{ return mpBlock[n]; }
	const_reference			operator[](size_type n) const	{ return mpBlock[n]; }
	reference				at(size_type n)					{ return n < mSize ? mpBlock[n] : throw std::length_error; }
	const_reference			at(size_type n) const			{ return n < mSize ? mpBlock[n] : throw std::length_error; }
	reference				front()							{ return *mpBlock; }
	const_reference			front() const					{ return *mpBlock; }
	reference				back()							{ return mpBlock[mSize-1]; }
	const_reference			back() const					{ return mpBlock[mSize-1]; }

	const_pointer			data() const	{ return mpBlock; }
	pointer					data()			{ return mpBlock; }

	const_iterator			begin() const	{ return mpBlock; }
	iterator				begin()			{ return mpBlock; }
	const_iterator			end() const		{ return mpBlock + mSize; }
	iterator				end()			{ return mpBlock + mSize; }

	const_reverse_iterator	rbegin() const	{ return const_reverse_iterator(end()); }
	reverse_iterator		rbegin()		{ return reverse_iterator(end()); }
	const_reverse_iterator	rend() const	{ return const_reverse_iterator(begin()); }
	reverse_iterator		rend()			{ return reverse_iterator(begin()); }

	bool					empty() const		{ return !mSize; }
	size_type				size() const		{ return mSize; }
	size_type				capacity() const	{ return mSize; }

	void clear() {
		if (mpBlock)
			A::deallocate(mpBlock, mSize);
		mpBlock = NULL;
		mSize = 0;
	}

	void resize(size_type s) {
		if (s != mSize) {
			if (mpBlock) {
				A::deallocate(mpBlock, mSize);
				mpBlock = NULL;
			}
			mSize = s;
			if (s)
				mpBlock = A::allocate(mSize, 0);
		}
	}

	void resize(size_type s, const T& value) {
		if (s != mSize) {
			if (mpBlock) {
				A::deallocate(mpBlock, mSize);
				mpBlock = NULL;
			}
			mSize = s;
			if (s) {
				mpBlock = A::allocate(mSize, 0);
				std::fill(mpBlock, mpBlock+s, value);
			}
		}
	}

	void swap(vdblock& x) {
		std::swap(mpBlock, x.mpBlock);
		std::swap(mSize, x.mSize);
	}

protected:
	typename A::pointer		mpBlock;
	typename A::size_type	mSize;

	union PODType {
		T x;
	};
};

///////////////////////////////////////////////////////////////////////////
//
//	vdstructex
//
//	vdstructex describes an extensible format structure, such as
//	BITMAPINFOHEADER or WAVEFORMATEX, without the pain-in-the-butt
//	casting normally associated with one.
//
///////////////////////////////////////////////////////////////////////////

template<class T>
class vdstructex {
public:
	typedef size_t			size_type;
	typedef T				value_type;

	vdstructex() : mpMemory(NULL), mSize(0) {}

	explicit vdstructex(size_t len) : mpMemory(NULL), mSize(0) {
		resize(len);
	}

	vdstructex(const T *pStruct, size_t len) : mSize(len), mpMemory((T*)malloc(len)) {
		memcpy(mpMemory, pStruct, len);
	}

	vdstructex(const vdstructex<T>& src) : mSize(src.mSize), mpMemory((T*)malloc(src.mSize)) {
		memcpy(mpMemory, src.mpMemory, mSize);
	}

	~vdstructex() {
		free(mpMemory);
	}

	bool		empty() const		{ return !mpMemory; }
	size_type	size() const		{ return mSize; }
	T*			data() const		{ return mpMemory; }

	T&	operator *() const	{ return *(T *)mpMemory; }
	T*	operator->() const	{ return (T *)mpMemory; }

	bool operator==(const vdstructex& x) const {
		return mSize == x.mSize && (!mSize || !memcmp(mpMemory, x.mpMemory, mSize));
	}

	bool operator!=(const vdstructex& x) const {
		return mSize != x.mSize || (mSize && memcmp(mpMemory, x.mpMemory, mSize));
	}

	vdstructex<T>& operator=(const vdstructex<T>& src) {
		assign(src.mpMemory, src.mSize);
		return *this;
	}

	void assign(const T *pStruct, size_type len) {
		if (mSize != len)
			resize(len);

		memcpy(mpMemory, pStruct, len);
	}

	void clear() {
		free(mpMemory);
		mpMemory = NULL;
		mSize = 0;
	}

	void resize(size_type len) {
		if (mSize != len)
			mpMemory = (T *)realloc(mpMemory, mSize = len);
	}

protected:
	size_type	mSize;
	T *mpMemory;
};

///////////////////////////////////////////////////////////////////////////
//
//	vdlist
//
//	vdlist<T> is similar to list<T*>, except:
//
//	1) The node structure must be embedded as a superclass of T.
//     Thus, the client is in full control of allocation.
//	2) Node pointers may be converted back into iterators in O(1).
//
///////////////////////////////////////////////////////////////////////////

struct vdlist_node {
	vdlist_node *mListNodeNext, *mListNodePrev;
};

template<class T, class T_Nonconst>
class vdlist_iterator : public vditerator<std::bidirectional_iterator_tag, T, ptrdiff_t>::type {
public:
	vdlist_iterator() {}
	vdlist_iterator(vdlist_node *p) : mp(p) {}
	vdlist_iterator(const vdlist_iterator<T_Nonconst, T_Nonconst>& src) : mp(src.mp) {}

	T* operator *() const {
		return static_cast<T*>(mp);
	}

	bool operator==(const vdlist_iterator<T, T_Nonconst>& x) const {
		return mp == x.mp;
	}

	bool operator!=(const vdlist_iterator<T, T_Nonconst>& x) const {
		return mp != x.mp;
	}

	vdlist_iterator& operator++() {
		mp = mp->mListNodeNext;
		return *this;
	}

	vdlist_iterator& operator--() {
		mp = mp->mListNodePrev;
		return *this;
	}

	vdlist_iterator operator++(int) {
		vdlist_iterator tmp(*this);
		mp = mp->mListNodeNext;
		return tmp;
	}

	vdlist_iterator& operator--(int) {
		vdlist_iterator tmp(*this);
		mp = mp->mListNodePrev;
		return tmp;
	}

	vdlist_node *mp;
};

class vdlist_base {
public:
	typedef	vdlist_node						node;
	typedef	size_t							size_type;
	typedef	ptrdiff_t						difference_type;

	bool empty() const {
		return mAnchor.mListNodeNext == &mAnchor;
	}

	size_type size() const {
		node *p = { mAnchor.mListNodeNext };
		size_type s = 0;

		if (p != &mAnchor)
			do {
				++s;
				p = p->mListNodeNext;
			} while(p != &mAnchor);

		return s;
	}

	void clear() {
		mAnchor.mListNodePrev	= &mAnchor;
		mAnchor.mListNodeNext	= &mAnchor;
	}

	void pop_front() {
		mAnchor.mListNodeNext = mAnchor.mListNodeNext->mListNodeNext;
		mAnchor.mListNodeNext->mListNodePrev = &mAnchor;
	}

	void pop_back() {
		mAnchor.mListNodePrev = mAnchor.mListNodePrev->mListNodePrev;
		mAnchor.mListNodePrev->mListNodeNext = &mAnchor;
	}

	static void unlink(vdlist_node& node) {
		vdlist_node& n1 = *node.mListNodePrev;
		vdlist_node& n2 = *node.mListNodeNext;

		n1.mListNodeNext = &n2;
		n2.mListNodePrev = &n1;
	}

protected:
	node	mAnchor;
};

template<class T>
class vdlist : public vdlist_base {
public:
	typedef	T*								value_type;
	typedef	T**								pointer;
	typedef	const T**						const_pointer;
	typedef	T*&								reference;
	typedef	const T*&						const_reference;
	typedef	vdlist_iterator<T, T>						iterator;
	typedef vdlist_iterator<const T, T>					const_iterator;
	typedef typename vdreverse_iterator<iterator, T>::type			reverse_iterator;
	typedef typename vdreverse_iterator<const_iterator, const T>::type	const_reverse_iterator;

	vdlist() {
		mAnchor.mListNodePrev	= &mAnchor;
		mAnchor.mListNodeNext	= &mAnchor;
	}

	iterator begin() {
		iterator it(mAnchor.mListNodeNext);
		return it;
	}

	const_iterator begin() const {
		const_iterator it(mAnchor.mListNodeNext);
		return it;
	}

	iterator end() {
		iterator it(&mAnchor);
		return it;
	}

	const_iterator end() const {
		const_iterator it(&mAnchor);
		return it;
	}

	reverse_iterator rbegin() {
		return reverse_iterator(begin());
	}

	const_reverse_iterator rbegin() const {
		return const_reverse_iterator(begin());
	}

	reverse_iterator rend() {
		return reverse_iterator(end);
	}

	const_reverse_iterator rend() const {
		return const_reverse_iterator(end());
	}

	const value_type front() const {
		return static_cast<T *>(mAnchor.mListNodeNext);
	}

	const value_type back() const {
		return static_cast<T *>(mAnchor.mListNodePrev);
	}

	iterator find(T *p) {
		iterator it(mAnchor.mListNodeNext);

		if (it.mp != &mAnchor)
			do {
				if (it.mp == static_cast<node *>(p))
					break;

				it.mp = it.mp->mListNodeNext;
			} while(it.mp != &mAnchor);

		return it;
	}

	const_iterator find(T *p) const {
		const_iterator it(mAnchor.mListNodeNext);

		if (it.mp != &mAnchor)
			do {
				if (it.mp == static_cast<node *>(p))
					break;

				it.mp = it.mp->mListNodeNext;
			} while(it.mp != &mAnchor);

		return it;
	}

	iterator fast_find(T *p) {
		iterator it(p);
		return it;
	}

	const_iterator fast_find(T *p) const {
		iterator it(p);
	}

	void push_front(T *p) {
		node& n = *p;
		n.mListNodePrev = &mAnchor;
		n.mListNodeNext = mAnchor.mListNodeNext;
		n.mListNodeNext->mListNodePrev = &n;
		mAnchor.mListNodeNext = &n;
	}

	void push_back(T *p) {
		node& n = *p;
		n.mListNodeNext = &mAnchor;
		n.mListNodePrev = mAnchor.mListNodePrev;
		n.mListNodePrev->mListNodeNext = &n;
		mAnchor.mListNodePrev = &n;
	}

	iterator erase(T *p) {
		return erase(fast_find(p));
	}

	iterator erase(iterator it) {
		node& n = *it.mp;

		n.mListNodePrev->mListNodeNext = n.mListNodeNext;
		n.mListNodeNext->mListNodePrev = n.mListNodePrev;

		it.mp = n.mListNodeNext;
		return it;
	}

	iterator erase(iterator i1, iterator i2) {
		node& np = *i1.mp->mListNodePrev;
		node& nn = *i2.mp;

		np.mListNodeNext = &nn;
		nn.mListNodePrev = &np;

		return i2;
	}

	void insert(iterator dst, T *src) {
		node& ns = *src;
		node& nd = *dst.mp;

		ns.mListNodeNext = &nd;
		ns.mListNodePrev = nd.mListNodePrev;
		nd.mListNodePrev->mListNodeNext = &ns;
		nd.mListNodePrev = &ns;
	}

	void insert(iterator dst, iterator i1, iterator i2) {
		if (i1 != i2) {
			node& np = *dst.mp->mListNodePrev;
			node& nn = *dst.mp;
			node& n1 = *i1.mp;
			node& n2 = *i2.mp->mListNodePrev;

			np.mListNodeNext = &n1;
			n1.mListNodePrev = &np;
			n2.mListNodeNext = &nn;
			nn.mListNodePrev = &n2;
		}
	}

	void splice(iterator dst, vdlist<T>& srclist) {
		insert(dst, srclist.begin(), srclist.end());
		srclist.clear();
	}

	void splice(iterator dst, vdlist<T>& srclist, iterator src) {
		T *v = *src;
		srclist.erase(src);
		insert(dst, v);
	}

	void splice(iterator dst, vdlist<T>& srclist, iterator i1, iterator i2) {
		if (dst.mp != i1.mp && dst.mp != i2.mp) {
			srclist.erase(i1, i2);
			insert(dst, i1, i2);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////

template<class T, class A = std::allocator<T> >
class vdfastvector {
public:
	typedef	T					value_type;
	typedef	T*					pointer;
	typedef	const T*			const_pointer;
	typedef	T&					reference;
	typedef	const T&			const_reference;
	typedef	size_t				size_type;
	typedef	ptrdiff_t			difference_type;
	typedef	pointer				iterator;
	typedef const_pointer		const_iterator;
	typedef typename vdreverse_iterator<iterator, T>::type			reverse_iterator;
	typedef typename vdreverse_iterator<const_iterator, const T>::type	const_reverse_iterator;

public:
	vdfastvector() {
		m.begin = NULL;
		m.end = NULL;
		m.eos = NULL;
	}

	vdfastvector(size_type len) {
		m.begin = m.allocate(len, NULL);
		m.end = m.begin + len;
		m.eos = m.begin + len;
	}

	vdfastvector(size_type len, const T& fill) {
		m.begin = m.allocate(len, NULL);
		m.end = m.begin + len;
		m.eos = m.begin + len;

		std::fill(m.begin, m.end, fill);
	}

	vdfastvector(const vdfastvector& x) {
		size_type n = x.m.end - x.m.begin;
		m.begin = m.allocate(n, NULL);
		m.end = m.begin + n;
		m.eos = m.end;
		memcpy(m.begin, x.m.begin, sizeof(T) * n);
	}

	vdfastvector(const value_type *p, const value_type *q) {
		m.begin = NULL;
		m.end = NULL;
		m.eos = NULL;

		assign(p, q);
	}

	~vdfastvector() {
		if (m.begin)
			m.deallocate(m.begin, m.eos - m.begin);
	}

	vdfastvector& operator=(const vdfastvector& x) {
		if (this != &x) {
			m.end = m.begin;
			resize(x.m.end - x.m.begin);
			memcpy(m.begin, x.m.begin, (char *)x.m.end - (char *)x.m.begin);
		}
		return *this;
	}

public:
	bool		empty() const		{ return m.begin == m.end; }
	size_type	size() const		{ return size_type(m.end - m.begin); }
	size_type	capacity() const	{ return size_type(m.eos - m.begin); }

	pointer			data()			{ return m.begin; }
	const_pointer	data() const	{ return m.begin; }

	iterator		begin()			{ return m.begin; }
	const_iterator	begin() const	{ return m.begin; }
	iterator		end()			{ return m.end; }
	const_iterator	end() const		{ return m.end; }

	reverse_iterator		rbegin()		{ return reverse_iterator(m.begin); }
	const_reverse_iterator	rbegin() const	{ return const_reverse_iterator(m.begin); }
	reverse_iterator		rend()			{ return reverse_iterator(m.end); }
	const_reverse_iterator	rend() const	{ return const_reverse_iterator(m.end); }

	reference		front()			{ return *m.begin; }
	const_reference	front() const	{ return *m.begin; }
	reference		back()			{ VDASSERT(m.begin != m.end); return m.end[-1]; }
	const_reference	back() const	{ VDASSERT(m.begin != m.end); return m.end[-1]; }

	reference			operator[](size_type n)			{ VDASSERT(n < size_type(m.end - m.begin)); return m.begin[n]; }
	const_reference		operator[](size_type n) const	{ VDASSERT(n < size_type(m.end - m.begin)); return m.begin[n]; }

public:
	T *alloc(size_type n) {
		size_type offset = (size_type)(m.end - m.begin);
		resize(offset + n);
		return m.begin + offset;
	}

	void assign(const T *p1, const T *p2) {
		if (m.begin) {
			m.deallocate(m.begin, m.eos - m.begin);
			m.begin = NULL;
			m.end = NULL;
			m.eos = NULL;
		}

		resize(p2 - p1);
		memcpy(m.begin, p1, (char *)p2 - (char *)p1);
	}

	void clear() {
		m.end = m.begin;
	}

	iterator erase(iterator it) {
		VDASSERT(it - m.begin < m.end - m.begin);

		memmove(it, it+1, (char *)m.end - (char *)(it+1));

		--m.end;

		return it;
	}

	iterator erase(iterator it1, iterator it2) {
		VDASSERT(it1 - m.begin <= m.end - m.begin);
		VDASSERT(it2 - m.begin <= m.end - m.begin);
		VDASSERT(it1 <= it2);

		memmove(it1, it2, (char *)m.end - (char *)it2);

		m.end -= (it2 - it1);

		return it1;
	}

	iterator insert(iterator it, const T& value) {
		const T temp(value);		// copy in case value is inside container.

		if (m.end == m.eos) {
			difference_type delta = it - m.begin;
			_reserve_always_add_one();
			it = m.begin + delta;
		}

		memmove(it+1, it, sizeof(T) * (m.end - it));
		*it = temp;
		++m.end;
		VDASSERT(m.end <= m.eos);

		return it;
	}

	iterator insert(iterator it, size_type n, const T& value) {
		const T temp(value);		// copy in case value is inside container.

		ptrdiff_t bytesToInsert = n * sizeof(T);

		if ((char *)m.eos - (char *)m.end < bytesToInsert) {
			difference_type delta = it - m.begin;
			_reserve_always_add(bytesToInsert);
			it = m.begin + delta;
		}

		memmove((char *)it + bytesToInsert, it, (char *)m.end - (char *)it);
		for(size_t i=0; i<n; ++i)
			*it++ = temp;
		m.end += n;
		VDASSERT(m.end <= m.eos);
		return it;
	}

	iterator insert(iterator it, const T *p1, const T *p2) {
		ptrdiff_t elementsToCopy = p2 - p1;
		ptrdiff_t bytesToCopy = (char *)p2 - (char *)p1;

		if ((char *)m.eos - (char *)m.end < bytesToCopy) {
			difference_type delta = it - m.begin;
			_reserve_always_add(bytesToCopy);
			it = m.begin + delta;
		}

		memmove((char *)it + bytesToCopy, it, (char *)m.end - (char *)it);
		memcpy(it, p1, bytesToCopy);
		m.end += elementsToCopy;
		VDASSERT(m.end <= m.eos);
		return it;
	}

	void push_back() {
		if (m.end == m.eos)
			_reserve_always_add_one();

		++m.end;
	}

	void push_back(const T& value) {
		const T temp(value);		// copy in case value is inside container.

		if (m.end == m.eos)
			_reserve_always_add_one();

		*m.end++ = temp;
	}

	void pop_back() {
		VDASSERT(m.begin != m.end);
		--m.end;
	}

	void resize(size_type n) {
		if (n*sizeof(T) > size_type((char *)m.eos - (char *)m.begin))
			_reserve_always_amortized(n);

		m.end = m.begin + n;
	}

	void resize(size_type n, const T& value) {
		const T temp(value);

		if (n*sizeof(T) > size_type((char *)m.eos - (char *)m.begin)) {
			_reserve_always_amortized(n);
		}

		const iterator newEnd(m.begin + n);
		if (newEnd > m.end)
			std::fill(m.end, newEnd, temp);
		m.end = newEnd;
	}

	void reserve(size_type n) {
		if (n*sizeof(T) > size_type((char *)m.eos - (char *)m.begin))
			_reserve_always(n);
	}

	void swap(vdfastvector& x) {
		T *p;

		p = m.begin;		m.begin = x.m.begin;		x.m.begin = p;
		p = m.end;			m.end = x.m.end;			x.m.end = p;
		p = m.eos;			m.eos = x.m.eos;			x.m.eos = p;
	}

protected:
#ifdef _MSC_VER
	__declspec(noinline)
#endif
	void _reserve_always_add_one() {
		_reserve_always((m.eos - m.begin) * 2 + 1);
	}

#ifdef _MSC_VER
	__declspec(noinline)
#endif
	void _reserve_always_add(size_type n) {
		_reserve_always((m.eos - m.begin) * 2 + n);
	}

#ifdef _MSC_VER
	__declspec(noinline)
#endif
	void _reserve_always(size_type n) {
		size_type oldSize = m.end - m.begin;
		T *oldStorage = m.begin;
		T *newStorage = m.allocate(n, NULL);

		memcpy(newStorage, m.begin, (char *)m.end - (char *)m.begin);
		m.deallocate(oldStorage, m.eos - m.begin);
		m.begin = newStorage;
		m.end = newStorage + oldSize;
		m.eos = newStorage + n;
	}

#ifdef _MSC_VER
	__declspec(noinline)
#endif
	void _reserve_always_amortized(size_type n) {
		size_type nextCapacity = (size_type)((m.eos - m.begin)*2);

		if (nextCapacity < n)
			nextCapacity = n;

		_reserve_always(nextCapacity);
	}

	struct : A {
		T *begin;
		T *end;
		T *eos;
	} m;

	union TrivialObjectConstraint {
		T m;
	};
};


#endif
