//	VirtualDub - Video processing and capture application
//	A/V interface library
//	Copyright (C) 1998-2004 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef f_VD2_RIZA_BITMAP_H
#define f_VD2_RIZA_BITMAP_H

#include <windows.h>
#include <vd2/system/vdstl.h>

struct VDPixmap;

///////////////////////////////////////////////////////////////////////////
//
//	pixmap <-> bitmap format converters
//
//	pixmap format:	describes channel layout, color space, subsampling
//	variant:		describes variations in layout and FOURCC that may
//					correspond to the same pixmap format
//	bitmap format:	Win32 BITMAPINFOHEADER
//
struct VDPixmapLayout;

int VDGetPixmapToBitmapVariants(int format);
int VDBitmapFormatToPixmapFormat(const BITMAPINFOHEADER& hdr);
int VDBitmapFormatToPixmapFormat(const BITMAPINFOHEADER& hdr, int& variant);
bool VDMakeBitmapFormatFromPixmapFormat(vdstructex<BITMAPINFOHEADER>& dst, const vdstructex<BITMAPINFOHEADER>& src, int format, int variant);
bool VDMakeBitmapFormatFromPixmapFormat(vdstructex<BITMAPINFOHEADER>& dst, const vdstructex<BITMAPINFOHEADER>& src, int format, int variant, uint32 w, uint32 h);
bool VDMakeBitmapFormatFromPixmapFormat(vdstructex<BITMAPINFOHEADER>& dst, int format, int variant, uint32 w, uint32 h);
uint32 VDMakeBitmapCompatiblePixmapLayout(VDPixmapLayout& layout, sint32 w, sint32 h, int format, int variant);
VDPixmap VDGetPixmapForBitmap(const BITMAPINFOHEADER& hdr, const void *data);

#endif
