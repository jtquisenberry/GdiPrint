// GdiPrint_OpenPrinter.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>
#include <Commdlg.h>
#include <stdio.h> 
#include <commdlg.h>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <time.h>
#include <stdlib.h> 
#include <iostream>
#include <chrono>
#include <ctime>

void DrawStuff(HDC hdc, int i);
void RunPrintJob(LPDEVMODE devmode, wchar_t* printerName);

/*===============================*/
/* Initialize DOCINFO structure */
/* ==============================*/
void InitPrintJobDoc(DOCINFO* di, wchar_t* docname)
{
	memset(di, 0, sizeof(DOCINFO));
	/* Fill in the required members. */
	di->cbSize = sizeof(DOCINFO);
	di->lpszDocName = docname;
}

std::string current_time_to_stringA()
{
	struct tm newtime;
	__time64_t long_time;
	errno_t err;

	// Get current time as 64-bit integer.
	_time64(&long_time);

	// Convert to local time.
	err = _localtime64_s(&newtime, &long_time);

	char output[30];
	strftime(output, 30, "%Y %m-%d %H-%M-%S", &newtime);

	// Convert to an ASCII representation.
	//err = asctime_s(timebuf, 26, &newtime);

	return output;
}

std::wstring current_time_to_stringW()
{
	struct tm newtime;
	//char am_pm[] = "AM";
	__time64_t long_time;
	// char timebuf[26];
	errno_t err;

	// Get current time as 64-bit integer.
	_time64(&long_time);

	// Convert to local time.
	err = _localtime64_s(&newtime, &long_time);

	wchar_t output[30];
	wcsftime(output, 30, L"%Y %m-%d %H-%M-%S", &newtime);

	// Convert to an ASCII representation.
	//err = asctime_s(timebuf, 26, &newtime);

	return output;
}

LPDEVMODE GetLandscapeDevMode(HWND hWnd, wchar_t* pDevice)
{
	HANDLE hPrinter;
	LPDEVMODE pDevMode;
	DWORD dwNeeded, dwRet;

	/* Start by opening the printer */
	if (!OpenPrinter(pDevice, &hPrinter, NULL))
		return NULL;

	/*
	* Step 1:
	* Allocate a buffer of the correct size.
	*/
	dwNeeded = DocumentProperties(hWnd,
		hPrinter, /* Handle to our printer. */
		pDevice, /* Name of the printer. */
		NULL, /* Asking for size, so */
		NULL, /* these are not used. */
		0); /* Zero returns buffer size. */
	pDevMode = (LPDEVMODE)malloc(dwNeeded);

	/*
	* Step 2:
	* Get the default DevMode for the printer and
	* modify it for your needs.
	*/
	dwRet = DocumentProperties(hWnd,
		hPrinter,
		pDevice,
		pDevMode, /* The address of the buffer to fill. */
		NULL, /* Not using the input buffer. */
		DM_OUT_BUFFER); /* Have the output buffer filled. */
	if (dwRet != IDOK)
	{
		/* If failure, cleanup and return failure. */
		free(pDevMode);
		ClosePrinter(hPrinter);
		return NULL;
	}

	///* Make changes to the DevMode which are supported.
	//*/
	//if (pDevMode->dmFields & DM_ORIENTATION)
	//{
	//	/* If the printer supports paper orientation, set it.*/
	//	pDevMode->dmOrientation = DMORIENT_LANDSCAPE;
	//}
	//
	//
	//
	//if (pDevMode->dmFields & DM_DUPLEX)
	//{
	//	/* If it supports duplex printing, use it. */
	//	pDevMode->dmDuplex = DMDUP_HORIZONTAL;
	//}
	//
	///*
	//* Step 3:
	//* Merge the new settings with the old.
	//* This gives the driver an opportunity to update any private
	//* portions of the DevMode structure.
	//*/
	//dwRet = DocumentProperties(hWnd,
	//	hPrinter,
	//	pDevice,
	//	pDevMode, /* Reuse our buffer for output. */
	//	pDevMode, /* Pass the driver our changes. */
	//	DM_IN_BUFFER | /* Commands to Merge our changes and */
	//	DM_OUT_BUFFER); /* write the result. */
	//
	///* Finished with the printer */
	//ClosePrinter(hPrinter);
	//
	//if (dwRet != IDOK)
	//{
	//	/* If failure, cleanup and return failure. */
	//	free(pDevMode);
	//	return NULL;
	//}

	/* Return the modified DevMode structure. */
	return pDevMode;
}






/*===============================*/
/* Drawing on the DC on Page */
/* ==============================*/
void DrawPage(HDC hdc, UINT Page)
{
	wchar_t line[50];
	int nWidth, nHeight;

	nWidth = GetDeviceCaps(hdc, HORZRES);
	nHeight = GetDeviceCaps(hdc, VERTRES);

	SelectObject(hdc, CreatePen(PS_SOLID, 2, RGB(255, 0, 0)));
	Rectangle(hdc, 0, 0, nWidth - 4, nHeight - 2);
	swprintf_s(line,
		L"Test Printing, page#%u width x height (%dx%d)",
		Page,
		nWidth,
		nHeight);
	SetBkMode(hdc, TRANSPARENT);
	TextOut(hdc, 4, 4, line, lstrlen(line));
}

/*===============================*/
/* The Abort Procudure */
/* ==============================*/
BOOL CALLBACK AbortProc(HDC hDC, int Error)
{
	MSG   msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return TRUE;
}
/*===============================*/
/* Obtain printer device context */
/* ==============================*/
HDC GetPrinterDC(void)
{
	PRINTDLG pdlg;

	/* Initialize the PRINTDLG structure. */
	memset(&pdlg, 0, sizeof(PRINTDLG));
	pdlg.lStructSize = sizeof(PRINTDLG);
	/* Set the flag to return printer DC. */
	pdlg.Flags = PD_RETURNDC;

	/* Invoke the printer dialog box. */
	PrintDlg(&pdlg);

	/* hDC member of the PRINTDLG structure contains the printer DC. */
	return pdlg.hDC;
}

/*==============================================*/
/* Sample code : Typical printing process */
/* =============================================*/
void RunPrintJob(LPDEVMODE devmode, wchar_t* printerName)
{
	HDC        hDC;
	DOCINFO    di;

	hDC = CreateDC(NULL, printerName, NULL, devmode);

	/* Did you get a good DC? */
	if (!hDC)
	{
		MessageBox(NULL,
			L"Error creating DC",
			L"Error",
			MB_APPLMODAL | MB_OK);
		return;
	}

	/* You always have to use an AbortProc(). */
	if (SetAbortProc(hDC, AbortProc) == SP_ERROR)
	{
		MessageBox(NULL,
			L"Error setting up AbortProc",
			L"Error",
			MB_APPLMODAL | MB_OK);
		return;
	}

	/* Init the DOCINFO and start the document. */
	InitPrintJobDoc(&di, (LPWSTR)L"MyDoc");
	StartDoc(hDC, &di);

	/* Print 1st page. */
	StartPage(hDC);
	DrawStuff(hDC, 1);
	EndPage(hDC);

	/* Indicate end of document.*/
	EndDoc(hDC);
	/* Clean up */
	DeleteDC(hDC);
}


void DrawStuff(HDC hdc, int i)
{
	// Get current date and time as Unicode string
	std::wstring timeString = current_time_to_stringW();

	// Initialize a RECT structure.
	RECT rect = { 600, 600, 700, 700 };

	// Output string
	std::wstring outString_temp1 = L"ExtTextOut ";
	std::wstring outString1 = outString_temp1 + timeString;
	std::wstring outString_temp2 = L"TextOut ";
	std::wstring outString2 = outString_temp2 + timeString;

	ExtTextOut(hdc, 300, 300, 0, &rect, outString1.c_str(), outString1.length(), NULL);
	TextOut(hdc, 600, 600, outString2.c_str(), outString2.length());
	return;
}


/* Print DC demo */
int APIENTRY main(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow)
{
	std::wstring timeString = current_time_to_stringW();

	wchar_t* printerName = const_cast<wchar_t*>(L"hp LaserJet 1320 PCL 6");
	LPDEVMODE devmode = GetLandscapeDevMode(NULL, printerName);
	RunPrintJob(devmode, printerName);

	return 0;
}

