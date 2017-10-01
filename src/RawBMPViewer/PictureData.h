
#pragma once

#include <list>

#include "RawData.h"


class PictureData {
public:
	enum Type {
		NotDefinedType = 0,
		JPGRawType,
		BMPType
	};

public:
	virtual ~PictureData ();
	bool Read (const char* szFileRd);
	void Show (HWND hWnd) const;
	bool Draw (HWND hWnd, HDC hDC) const;
	void Reset ();
	bool Anal (int analID, int index);

	static PictureData* GetSingleton ();
	static void DestroySingleton ();

protected:
	PictureData ();
	bool DrawJPG (HWND hWnd, HDC hDC) const;
	bool DrawBMP (HWND hWnd, HDC hDC) const;

private:
	Type					type;
	unsigned char *			rawDataPtr;
	DWORD					rawDataSize;
	DWORD					hSize;
	DWORD					vSize;
	int						bitOfColor;

	std::list<RawData>		rawDataList;

	static DWORD			Align16 (DWORD val);

	static PictureData*		singleton;
};


bool Anal(HWND hWnd, int analID, int index);
