
#pragma once

class RawData {
public:
	enum RowSourceEnum {
		Undefined = 0,
		BMPSource
	};
public:
	RawData ();
	RawData (const RawData& src);
	~RawData ();

	RawData& operator=(const RawData& src);

	bool Set (const unsigned char *	src, DWORD size, DWORD w, DWORD h, int nBit, RowSourceEnum srcType);
	bool Filter (const RawData& src, int filterID);
	bool ApplyFilter3x3 (const RawData& src, short filter[3][3]);
	void Reset ();
	unsigned char GetPixel (DWORD x, DWORD y) const;

private:
	unsigned char *			rawDataPtr;
	DWORD					rawDataSize;
	DWORD					width;
	DWORD					height;
	int						bitOfPixel;
};
