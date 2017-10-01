// RawData.cpp : Defines the entry point for the application.
//

#include "stdafx.h"

#include "RawData.h"

RawData::RawData ()
	:	rawDataPtr (nullptr),
		rawDataSize (0),
		width (0),
		height (0),
		bitOfPixel (0)
{
}


RawData::RawData (const RawData& src)
	:	rawDataPtr (nullptr),
		rawDataSize (0),
		width (0),
		height (0),
		bitOfPixel (0)
{
	this->operator= (src);
}


RawData::~RawData ()
{
	Reset ();
}


RawData& RawData::operator=(const RawData& src)
{
	Reset ();
	if (src.rawDataPtr != nullptr) {
		rawDataPtr  = new unsigned char[src.rawDataSize];
		memcpy (rawDataPtr, src.rawDataPtr, src.rawDataSize);
		rawDataSize = src.rawDataSize;
		width = src.width;
		height = src.height;
		bitOfPixel = src.bitOfPixel;
	}
	return *this;
}


bool RawData::Set (const unsigned char *	src, DWORD size, DWORD w, DWORD h, int nBit, RowSourceEnum srcType)
{
	Reset ();
	rawDataPtr = new unsigned char [w * h];
	if (rawDataPtr == nullptr)
		return false;
	width = w;
	height = h;
	bitOfPixel = 8;
	for (DWORD y = 0; y < h; ++y) {
		const unsigned char * ptrByte = src + (y * w);
		for (DWORD x = 0; x < w; ++x) {
			unsigned char r = *(unsigned char *)(ptrByte+2);
			rawDataPtr[x + (w * (h - y - 1))] = r;
			ptrByte += 3;
		}
	}
	rawDataSize = width * height;
	return true;
}


bool RawData::Filter (const RawData& src, int filterID)
{
	if (src.rawDataPtr == nullptr)
		return false;
	Reset ();
	this->operator= (src);
	memset (rawDataPtr, rawDataSize, 0);
	switch (filterID) {
	case 1:
		{
			short filter [3][3] = {1,2,1,2,-8,2,1,2,1};
			ApplyFilter3x3 (src, filter);
		}
		break;
	}
	return true;
}


bool RawData::ApplyFilter3x3 (const RawData& src, short filter[3][3])
{
	for (DWORD y = 1; y < height - 1; ++y) {
		for (DWORD x = 1; x < width - 1; ++x) {
			short v = 0;
			v+= filter[0][0] * (short)src.rawDataPtr[(y - 1) * width + x - 1];
			v+= filter[0][1] * (short)src.rawDataPtr[(y - 1) * width + x];
			v+= filter[0][2] * (short)src.rawDataPtr[(y - 1) * width + x + 1];
			v+= filter[1][0] * (short)src.rawDataPtr[(y) * width + x - 1];
			v+= filter[1][1] * (short)src.rawDataPtr[(y) * width + x];
			v+= filter[1][2] * (short)src.rawDataPtr[(y) * width + x + 1];
			v+= filter[2][0] * (short)src.rawDataPtr[(y + 1) * width + x - 1];
			v+= filter[2][1] * (short)src.rawDataPtr[(y + 1) * width + x];
			v+= filter[2][2] * (short)src.rawDataPtr[(y + 1) * width + x + 1];
			src.rawDataPtr[(y) * width + x] = (unsigned char)(v >> 4);
		}
	}
	return true;
}

void RawData::Reset ()
{
	if (rawDataPtr != nullptr)
		delete rawDataPtr;
	rawDataPtr = nullptr;
	rawDataSize = 0;
	width = 0;
	height = 0;
	bitOfPixel = 0;
}


unsigned char RawData::GetPixel (DWORD x, DWORD y) const
{
	if (rawDataPtr == nullptr || x >= width || y >= height)
		return 0;
	return rawDataPtr[y * width + x];
}


