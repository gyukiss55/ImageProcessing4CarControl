// LoadFile.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "LoadFile.h"
#include "PictureData.h"
#include <CommDlg.h>

bool LoadFile (HWND hWnd)
{
	OPENFILENAMEA ofn;       // common dialog box structure
	char path[512];
	char szFileRd[512];

	GetCurrentDirectoryA (sizeof (path), path);
	ZeroMemory (szFileRd, sizeof(szFileRd));
	strcpy_s (szFileRd, sizeof (szFileRd), "*.JPG");

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFile = szFileRd;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = 512;
	ofn.lpstrFilter = "*.jpg\0\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrTitle = "Read picture file";
	ofn.lpstrInitialDir = path;
	ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST | OFN_READONLY | OFN_FILEMUSTEXIST;
	if (GetOpenFileNameA(&ofn) == TRUE && strlen (szFileRd) > 0) {
		PictureData *pictureData = PictureData::GetSingleton ();
		pictureData->Read (szFileRd);
		pictureData->Show (hWnd);
		return true;
	}
	return false;
}
