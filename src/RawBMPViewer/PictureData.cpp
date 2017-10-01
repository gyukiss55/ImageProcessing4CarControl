// LoadFile.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "PictureData.h"

PictureData*		PictureData::singleton;

PictureData::PictureData ()
	:	type (NotDefinedType),
		rawDataPtr (nullptr),
		rawDataSize (0),
		hSize (0),
		vSize (0),
		bitOfColor (0)
{
}


PictureData::~PictureData ()
{
	Reset ();
}


bool PictureData::Read (const char* filename)
{
	Reset ();
	HANDLE hFileRd = CreateFileA(filename,
								GENERIC_READ | GENERIC_WRITE,
								0,    // must be opened with exclusive-access
								NULL, // no security attributes
								OPEN_EXISTING, // must use OPEN_EXISTING
								0,    // not overlapped I/O
								NULL  // hTemplate must be NULL for comm devices
								);
	if (hFileRd != INVALID_HANDLE_VALUE) {
		Reset ();
		BOOL ret = TRUE;

		LARGE_INTEGER lFileSize;
		if (GetFileSizeEx (hFileRd, &lFileSize) && lFileSize.u.HighPart == 0) {
			rawDataSize = lFileSize.u.LowPart;
			rawDataPtr = new unsigned char [rawDataSize + 1];
			if (rawDataPtr != nullptr) {
				ZeroMemory (rawDataPtr, rawDataSize + 1);
				ReadFile (hFileRd, rawDataPtr, rawDataSize, nullptr, nullptr);
				hSize = 2592;
				vSize = 1944;
				type = JPGRawType;
				bitOfColor = 10;
				if (rawDataPtr[0] == 'B') {
					if (rawDataPtr[0] != 'B' || rawDataPtr[1] != 'M' || *(DWORD *)&rawDataPtr[2] != rawDataSize) {
						Reset ();
						return false;
					}
					type = BMPType;
					DWORD pixelPtrOffset = *(DWORD *)&rawDataPtr[10];
					DWORD sizeOfHeader = *(DWORD *)&rawDataPtr[0x0e];
					hSize = *(DWORD *)&rawDataPtr[0x12];
					vSize = *(DWORD *)&rawDataPtr[0x16];
				}
			}
		}
		CloseHandle (hFileRd);
		return true;
	}
	return false;
}

void PictureData::Show (HWND hWnd) const
{
	InvalidateRect (hWnd, nullptr, TRUE);
}

void PictureData::Reset()
{
	if (rawDataPtr != nullptr)
		delete rawDataPtr;
	type = (NotDefinedType);
	rawDataPtr = (nullptr),
	rawDataSize = (0),
	hSize = (0);
	vSize = (0);
	bitOfColor = (0);
}

bool PictureData::Draw (HWND hWnd, HDC hDC) const
{
	if (rawDataPtr != nullptr) {
		switch (type) {
			case JPGRawType:
			{
				return DrawJPG (hWnd, hDC);
			}
			case BMPType:
			{
				return DrawBMP (hWnd, hDC);
			}
		}
	}
	return false;
}


PictureData* PictureData::GetSingleton ()
{
	if (singleton == nullptr)
		singleton = new PictureData;
	return singleton;
}


void PictureData::DestroySingleton ()
{
	if (singleton != nullptr)
		delete singleton;
}

/*

2592 x 1944
2592 + 2 = 2594  = 162.125 * 16
1944 + 2 = 1946  = 121.625 * 16

163 * 16 * 10 / 8 = 203.75 * 16 = 3260
204 * 16 = 3264

122 * 16 = 1952
3264 * 1952 = 6371328

*/

DWORD PictureData::Align16 (DWORD val)
{
	DWORD tmp = val % 16;
	if (tmp == 0) {
		return val;
	} else {
		return ((val / 16) + 1) * 16;
	}
}

bool PictureData::DrawJPG (HWND hWnd, HDC hDC) const
{
	const DWORD Align16HSize = Align16 (hSize + 2);
	const DWORD Align16VSize = Align16 (vSize + 2);
	const DWORD Align16HTotalSize = Align16 (Align16HSize * bitOfColor / 8);
	const DWORD currRawDataSize = Align16HTotalSize * Align16VSize;
	if (rawDataSize < currRawDataSize)
		return false;
	DWORD xBegin = 2;
	DWORD yBegin = vSize / 4;
	for (DWORD y = yBegin; y < vSize * 3 / 4; ++y) {
		for (DWORD x = xBegin; x < hSize * 3 / 4; ++x) {
			short r = 0;
			short g = 0;
			short b = 0;
			const unsigned char * ptrByte = rawDataPtr + rawDataSize - currRawDataSize + (y * Align16HTotalSize) + (x * 3 * bitOfColor / 8);
			DWORD shift = x * 3 * bitOfColor % 8;
			long long valRGB = *(long long *)ptrByte;
			valRGB = valRGB >> (shift + bitOfColor - 8);
			b = valRGB & 0xFF;
			valRGB = valRGB >> bitOfColor;
			g = valRGB & 0xFF;
			valRGB = valRGB >> bitOfColor;
			r = valRGB & 0xFF;
			SetPixel (hDC, x - xBegin, y - yBegin, RGB (r,g,b));
		}
	}
	return true;
}

bool PictureData::DrawBMP (HWND hWnd, HDC hDC) const
{
	if (rawDataPtr[0] != 'B' || rawDataPtr[1] != 'M' || *(DWORD *)&rawDataPtr[2] != rawDataSize)
		return false;
	DWORD pixelPtrOffset = *(DWORD *)&rawDataPtr[10];
	DWORD sizeOfHeader = *(DWORD *)&rawDataPtr[0x0e];
	short numOfColorPlane = *(short *)&rawDataPtr[0x1a]; // ==1
	short numOfBitPerPixel = *(short *)&rawDataPtr[0x1c]; // ==1, 4, 8, 16, 24, 32...
	DWORD compressMode = *(DWORD *)&rawDataPtr[0x1e]; // 	BI_RGB = 0 // non compression
	if (numOfColorPlane != 1 || numOfBitPerPixel != 0x18 || compressMode != 0)
		return false;

	DWORD rowSize = (numOfBitPerPixel * hSize + 31) / 32 * 4;
	DWORD pixelArraySize = rowSize * vSize;
	DWORD xBegin = 0;
	DWORD yBegin = 0;
	DWORD xEnd = hSize;
	DWORD yEnd = vSize;
	for (DWORD y = yBegin; y < yEnd; ++y) {
		unsigned char * ptrByte = rawDataPtr + pixelPtrOffset + (y * rowSize);
		for (DWORD x = xBegin; x < xEnd; ++x) {
			unsigned char b = *(ptrByte++);
			unsigned char g = *(ptrByte++);
			unsigned char r = *(ptrByte++);
			SetPixel (hDC, x - xBegin, yEnd - y, RGB (r, g, b));
		}
	}

	DWORD xOffs = 0;
	DWORD yOffs = 0;
	DWORD i = 0;
	for (auto it = rawDataList.begin (); it != rawDataList.end (); ++it) {
		if (i%2 == 0) {
			xOffs += hSize;
		} else {
			yOffs += vSize;
		}
		for (DWORD y = 0; y < vSize; ++y) {
			for (DWORD x = 0; x < hSize; ++x) {
				unsigned char v = it->GetPixel (x, y);
				SetPixel (hDC, x + xOffs, y + yOffs, RGB (v, v, v));
			}
		}
	}
	return false;
}


bool PictureData::Anal (int analID, int index)
{
	if (rawDataPtr == nullptr || type != BMPType)
		return false;
	int si = index;
	int i = rawDataList.size ();
	if (i == 0) {
		rawDataList.push_back (RawData ());
		RawData& rawData = rawDataList.back ();
		i = rawDataList.size ();

		DWORD pixelPtrOffset = *(DWORD *)&rawDataPtr[10];
		short numOfBitPerPixel = *(short *)&rawDataPtr[0x1c]; // ==1, 4, 8, 16, 24, 32...
		DWORD rowSize = (numOfBitPerPixel * hSize + 31) / 32 * 4;
		const unsigned char *	src =  rawDataPtr + pixelPtrOffset;
		rawData.Set (src, rowSize * vSize, hSize, vSize, 8, RawData::BMPSource);

		si = 0;
	}
	if (index >= i)
		return false;
	int k = 0;
	auto it =  rawDataList.cbegin ();
	for (; k != index && it != rawDataList.cend (); ++it) {}

	int j = rawDataList.size ();
	rawDataList.push_back (RawData ());
	RawData& rawData = rawDataList.back ();
	rawData.Filter (*it, analID);
	return true;
}


bool Anal(HWND hWnd, int analID, int index)
{
	PictureData *pictureData = PictureData::GetSingleton ();
	if (pictureData->Anal (analID, index)) {
		InvalidateRect (hWnd, NULL, TRUE);
		return true;
	}
	return false;
}
