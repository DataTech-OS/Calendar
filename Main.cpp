#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <windowsx.h>
#include <ShlObj_core.h>

#include "resource.h"

#define ID_FILE_EXIT 9001
#define ID_FILL 9002
#define ID_ADD 9003
#define ID_MODIFY_ORD 9004
#define BUTTON 800

const char g_szClassName[] = "myWindowClass";
COLORREF g_rgbText = RGB(0, 0, 0);
HANDLE ord, ext;
BOOL showMenu = FALSE;
const char *month[] = { "Gennaio", "Febbraio", "Marzo", "Aprile", "Maggio", "Giugno", "Luglio", "Agosto", "Settembre",
						"Ottobre", "Novembre", "Dicembre" };
const char *days[] = {"Lunedi", "Martedi", "Mercoledi", "Giovedi", "Venerdi", "Sabato", "Domenica", NULL};
const char *hours[] = {"8-9: ", "9-10: ", "10-11: ", "11-12: ", "12-13: ", "13-14: ", "14-15: ", "15-16: ", 
						"16-17: ", "17-18: ", "18-19: ", "19-20: ", "20-21: ", "21-22: ", "22-23: ", "23-24: ", NULL};
int counter = 0, day = -2;
COLORREF backgroundColor = RGB(204, 229, 255);
HWND hwndButtonM, hwndButtonO1, hwndButtonO2, hwndButtonO3;
POINT p;
BOOL present = FALSE;
char path[MAX_PATH], extr[MAX_PATH], ordi[MAX_PATH];

BOOL WriteToOrdinary(char *buf, int len, int hour, int flag)
{
	DWORD size, ph, pp;
	size = GetFileSize(ord, &ph);

	if (!buf && flag)
	{
		CloseHandle(ord);
		ord = CreateFileA(ordi, FILE_ALL_ACCESS, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
							CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		return TRUE;
	}
	if (size > 0)
	{
		if (buf)
		{
			char *buffer = (PCHAR)GlobalAlloc(GPTR, size + 1 + len);
			if (ReadFile(ord, buffer, size, &ph, NULL))
			{
				char *start = strstr(buffer, days[day]);
				char *target = strstr(start, hours[hour]) + strlen(hours[hour]);

				char *strToCpy = strstr(target, hours[hour + 1]) - 1;
				char *placeholder = (PCHAR)GlobalAlloc(GPTR, size - (strToCpy - buffer) + 1);

				strcpy(placeholder, strToCpy);
				strcpy(target, buf);
				strcpy(target + len - 1, placeholder);

				GlobalFree(placeholder);
			}
			SetFilePointer(ord, 0, 0, FILE_BEGIN);
			WriteFile(ord, buffer, strlen(buffer), &pp, NULL);
			GlobalFree(buffer);
		}
	}
	else
	{
		for (int i = 0; days[i] != NULL; i++)
		{
			WriteFile(ord, days[i], strlen(days[i]), &pp, NULL);
			WriteFile(ord, "\n", 1, &pp, NULL);
			for (int j = 0; hours[j] != NULL; j++)
			{
				WriteFile(ord, hours[j], strlen(hours[j]), &pp, NULL);
				if (hour == j && day == i)
				{
					WriteFile(ord, buf, len - 1, &pp, NULL);
				}
				WriteFile(ord, "\n", 1, &pp, NULL);
			}
		}
	}

	FlushFileBuffers(ord);
	SetFilePointer(ord, 0, 0, FILE_BEGIN);

	return TRUE;
}

BOOL CALLBACK ModDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
	{
		int cell = p.y / 42;
		p.y = cell * 42 + 8;
		p.x = 795 + 50 + (350 - 300) / 2;
		SetWindowPos(hwnd, HWND_TOP, p.x, p.y, 300, 30, 0);
	}
	break;
	case WM_COMMAND:
	{
		if (wParam == IDOK)
		{
			int len = GetWindowTextLength(GetDlgItem(hwnd, IDC_MOD));
			if (len > 0)
			{
				char *buf;

				buf = (char*)GlobalAlloc(GPTR, len + 1);
				GetDlgItemText(hwnd, IDC_MOD, buf, len + 1);

				FlushFileBuffers(ord);
				SetFilePointer(ord, 0, 0, FILE_BEGIN);
				WriteToOrdinary(buf, len + 1, p.y / 42, FALSE);

				GlobalFree((HANDLE)buf);
			}
			EndDialog(hwnd, 0);
		}
	}
	break;
	default:
		return FALSE;
	}
	return TRUE;
}

BOOL CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hwnd, IDC_DAY, days[counter]);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_ADD:
		{
			day = counter;
			int fl = 0;
			for (int i = 0; i < 16; i++)
			{
				int len = GetWindowTextLength(GetDlgItem(hwnd, IDC_A + i));
				if (len > 0)
				{
					fl = 1;
					int j;
					char *buf;

					buf = (char *)GlobalAlloc(GPTR, len + 1);
					GetDlgItemText(hwnd, IDC_A + i, buf, len + 1);

					WriteToOrdinary(buf, len + 1, i, FALSE);

					GlobalFree((HANDLE)buf);
				}
			}
			if(!fl)
				WriteToOrdinary(NULL, 0, 0, FALSE);
			SetDlgItemText(hwnd, IDC_DAY, days[++counter]);
			for (int i = 0; i < 16; i++)
			{
				SetDlgItemText(hwnd, IDC_A + i, "");
			}
			if (counter == 7)
			{
				day = -2;
				EndDialog(hwnd, 0);
				counter = 0;
			}
		}
		break;
		case IDC_CLEAR:
		{
			for (int i = 0; i < 16; i++)
			{
				SetDlgItemText(hwnd, IDC_A + i, "");
			}
		}
		break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hwnd, 0);
		counter = 0;
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK windowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		DWORD size, ph, pp;
		size = GetFileSize(ord, &ph);

		char *buff = (PCHAR)GlobalAlloc(GPTR, size + 1);

		if (ReadFile(ord, buff, size, &ph, NULL))
		{
			SetBkMode(hdc, OPAQUE);
			SetBkColor(hdc, backgroundColor);
			SetTextColor(hdc, g_rgbText);

			RECT prc;
			GetClientRect(hwnd, &prc);

			int len;
			SYSTEMTIME sysTime;
			GetSystemTime(&sysTime);

			if (day == -2)
			{
				day = sysTime.wDayOfWeek - 1;
				if (day == -1)
					day = 6;
			}
			char *start = strstr(buff, days[day]) + strlen(days[day]) + 1;
			if (day != 6)
			{
				char *end = strstr(buff, days[day + 1]);
				len = end - start;
			}
			else
				len = size - (start - buff);

			// TEXT FORMATTING
			HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
			HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
			MoveToEx(hdc, 50, 0, NULL);
			LineTo(hdc, 50, 675);
			MoveToEx(hdc, 390, 0, NULL);
			LineTo(hdc, 390, 675);
			SelectObject(hdc, hOldPen);
			DeleteObject(hPen);

			HFONT hf;
			long lfHeight;

			lfHeight = -MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72);
			hf = CreateFontA(lfHeight, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Calibri");
			HFONT hfOld = (HFONT)SelectObject(hdc, hf);

			RECT cust = prc;
			char bff[5] = { 0 };

			if (day == 2 || day == 6)
				cust.left -= 7;
			cust.left += 414;
			cust.top += 90;
			DrawTextA(hdc, days[day], -1, &cust, DT_WORDBREAK);

			lfHeight = -MulDiv(12, GetDeviceCaps(hdc, LOGPIXELSY), 72);
			hf = CreateFontA(lfHeight, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Calibri");
			hfOld = (HFONT)SelectObject(hdc, hf);

			prc.top += 12;
			int count = len, f = 44;
			char *next;
			while (count > 0)
			{
				next = strstr(start, "\n") + 1;
				count -= next - start;
				
				RECT sec = prc;
				sec.left += 60;
				char *second = strstr(start, " ") + 1;
				DrawTextA(hdc, start, second - start, &prc, DT_WORDBREAK);
				
				if (second[0] == '.')
				{
					SetBkColor(hdc, RGB(255, 0, 0));
					second++;
				}
				DrawTextA(hdc, second, next - second, &sec, DT_WORDBREAK);
				SetBkColor(hdc, backgroundColor);

				start = next;
				prc.top += 42;
				HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
				HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
				MoveToEx(hdc, 0, f, NULL);
				LineTo(hdc, 390, f);
				SelectObject(hdc, hOldPen);
				DeleteObject(hPen);
				f += 42;
			}
			SelectObject(hdc, hfOld);
		}
		SetFilePointer(ord, 0, 0, FILE_BEGIN);
		EndPaint(hwnd, &ps);
	}
	break;
	case WM_CREATE:
	{
		if (showMenu)
			DialogBoxParamA(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAIN), hwnd, (DLGPROC)AboutDlgProc, 0);

		HMENU hMenu, hSubMenu;
		HICON hIcon, hIconSm;

		hMenu = CreateMenu();

		hSubMenu = CreatePopupMenu();
		AppendMenuA(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "File");
		AppendMenuA(hSubMenu, MF_STRING, ID_FILE_EXIT, "Esci");

		hSubMenu = CreatePopupMenu();
		AppendMenuA(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, "Options");
		AppendMenuA(hSubMenu, MF_STRING, ID_ADD, "Aggiungi attività straordinarie");
		AppendMenuA(hSubMenu, MF_STRING, ID_MODIFY_ORD, "Riscrivi il calendario ordinario");

		SetMenu(hwnd, hMenu);

		HWND hwndButtonF = CreateWindowA("BUTTON", ">", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBOX,
									410, 250, 70, 70, hwnd, (HMENU)BUTTON, (HINSTANCE)GetWindowLongA(hwnd, GWLP_HINSTANCE), NULL);
		HWND hwndButtonB = CreateWindowA("BUTTON", "<", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBOX,
									410, 320, 70, 70, hwnd, (HMENU)(BUTTON+1), (HINSTANCE)GetWindowLongA(hwnd, GWLP_HINSTANCE), NULL);
	}
	break;
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case ID_FILE_EXIT:
			PostMessageA(hwnd, WM_CLOSE, 0, 0);
			break;
		case ID_ADD:
		{
			//TODO
			//PostMessageA(hwnd, WM_COMMAND, ID_FILL, 0);
		}
		break;
		case ID_MODIFY_ORD:
		{
			SetFilePointer(ord, 0, 0, FILE_BEGIN);
			WriteToOrdinary(NULL, 0, 0, TRUE);
			DialogBoxParamA(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MAIN), hwnd, (DLGPROC)AboutDlgProc, 0);
			
			RECT prc;
			GetClientRect(hwnd, &prc);

			InvalidateRect(hwnd, &prc, TRUE);
			PostMessageA(hwnd, WM_PAINT, NULL, NULL);
		}
		break;
		case BUTTON:
		{
			if (day == 6)
				day = 0;
			else
				day += 1;

			RECT prc;
			GetClientRect(hwnd, &prc);

			InvalidateRect(hwnd, &prc, TRUE);
			PostMessageA(hwnd, WM_PAINT, NULL, NULL);
		}
		break;
		case BUTTON + 1:
		{
			if (day == 0)
				day = 6;
			else
				day -= 1;

			RECT prc;
			GetClientRect(hwnd, &prc);

			InvalidateRect(hwnd, &prc, TRUE);
			PostMessageA(hwnd, WM_PAINT, NULL, NULL);
		}
		break;
		case BUTTON + 2:
		{
			DestroyWindow(hwndButtonM);
			DialogBoxParamA(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MOD), hwnd, (DLGPROC)ModDlgProc, NULL);

			RECT prc;
			GetClientRect(hwnd, &prc);

			InvalidateRect(hwnd, &prc, TRUE);
			PostMessageA(hwnd, WM_PAINT, NULL, NULL);
		}
		break;
		case BUTTON + 3:
		{
			PostMessageA(hwnd, WM_COMMAND, ID_ADD, NULL);
			DestroyWindow(hwndButtonO1);
			DestroyWindow(hwndButtonO2);
			DestroyWindow(hwndButtonO3);
			present = FALSE;
		}
		break;
		case BUTTON + 4:
		{
			PostMessageA(hwnd, WM_COMMAND, ID_MODIFY_ORD, NULL);
			DestroyWindow(hwndButtonO1);
			DestroyWindow(hwndButtonO2);
			DestroyWindow(hwndButtonO3);
			present = FALSE;
		}
		break;
		case BUTTON + 5:
		{
			PostMessageA(hwnd, WM_COMMAND, ID_FILE_EXIT, NULL);
			DestroyWindow(hwndButtonO1);
			DestroyWindow(hwndButtonO2);
			DestroyWindow(hwndButtonO3);
			present = FALSE;
		}
		break;
		}
	}
	break;
	case WM_CLOSE:
	{
		CloseHandle(ord);
		CloseHandle(ext);
		DestroyWindow(hwnd);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
	break;
	case WM_RBUTTONDOWN:
	{
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);

		if (xPos >= 390)
		{
			if (yPos <= 320 && yPos >= 250)
			{
				if (xPos <= 410 || xPos >= 480) 
				{
					if (!present)
					{
						HFONT hf;
						long lfHeight;
						HDC hdc = GetDC(hwnd);

						lfHeight = -MulDiv(12, GetDeviceCaps(hdc, LOGPIXELSY), 72);
						hf = CreateFontA(lfHeight, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Calibri");

						hwndButtonO1 = CreateWindowA("BUTTON", "Aggiungi attività straordinarie", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
							xPos - 200, yPos, 200, 20, hwnd, (HMENU)(BUTTON + 3), (HINSTANCE)GetWindowLongA(hwnd, GWLP_HINSTANCE), NULL);
						hwndButtonO2 = CreateWindowA("BUTTON", "Riscrivi il calendario ordinario", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
							xPos - 200, yPos + 20, 200, 20, hwnd, (HMENU)(BUTTON + 4), (HINSTANCE)GetWindowLongA(hwnd, GWLP_HINSTANCE), NULL);
						hwndButtonO3 = CreateWindowA("BUTTON", "Exit application", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
							xPos - 200, yPos + 40, 200, 20, hwnd, (HMENU)(BUTTON + 5), (HINSTANCE)GetWindowLongA(hwnd, GWLP_HINSTANCE), NULL);
						present = TRUE;

						SendMessageA(hwndButtonO1, WM_SETFONT, WPARAM(hf), TRUE);
						SendMessageA(hwndButtonO2, WM_SETFONT, WPARAM(hf), TRUE);
						SendMessageA(hwndButtonO3, WM_SETFONT, WPARAM(hf), TRUE);
					}
				}
			}
			else
			{
				if (!present)
				{
					HFONT hf;
					long lfHeight;
					HDC hdc = GetDC(hwnd);

					lfHeight = -MulDiv(12, GetDeviceCaps(hdc, LOGPIXELSY), 72);
					hf = CreateFontA(lfHeight, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "Calibri");

					hwndButtonO1 = CreateWindowA("BUTTON", "Aggiungi attività straordinarie", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
						xPos - 200, yPos, 200, 20, hwnd, (HMENU)(BUTTON + 3), (HINSTANCE)GetWindowLongA(hwnd, GWLP_HINSTANCE), NULL);
					hwndButtonO2 = CreateWindowA("BUTTON", "Riscrivi calendario ordinario", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
						xPos - 200, yPos + 20, 200, 20, hwnd, (HMENU)(BUTTON + 4), (HINSTANCE)GetWindowLongA(hwnd, GWLP_HINSTANCE), NULL);
					hwndButtonO3 = CreateWindowA("BUTTON", "Esci dal calendario", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
						xPos - 200, yPos + 40, 200, 20, hwnd, (HMENU)(BUTTON + 5), (HINSTANCE)GetWindowLongA(hwnd, GWLP_HINSTANCE), NULL);
					present = TRUE;

					SendMessageA(hwndButtonO1, WM_SETFONT, WPARAM(hf), TRUE);
					SendMessageA(hwndButtonO2, WM_SETFONT, WPARAM(hf), TRUE);
					SendMessageA(hwndButtonO3, WM_SETFONT, WPARAM(hf), TRUE);
				}
			}
		}
		else if(xPos >= 50)
		{
			p.x = xPos;
			p.y = yPos;
			hwndButtonM = CreateWindowA("BUTTON", "Modifica", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBOX,
				xPos, yPos, 60, 20, hwnd, (HMENU)(BUTTON + 2), (HINSTANCE)GetWindowLongA(hwnd, GWLP_HINSTANCE), NULL);
		}
	}
	break;
	case WM_LBUTTONDOWN:
	{
		DestroyWindow(hwndButtonO1);
		DestroyWindow(hwndButtonO2);
		DestroyWindow(hwndButtonO3);
		present = FALSE;
	}
	break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HMODULE current, HMODULE previous, LPSTR cmdLine, int cmdShow)
{
	// Check for Calendar database directory
	SHGetFolderPathA(NULL, CSIDL_DESKTOP, NULL, SHGFP_TYPE_CURRENT, path);

	strcat(path, "\\Calendar");
	strcpy(extr, path);
	strcpy(ordi, path);
	strcat(ordi, "\\Ordinary.txt");
	strcat(extr, "\\Extraordinary.txt");

	DWORD attr = GetFileAttributesA(path);
	if (attr == INVALID_FILE_ATTRIBUTES)
	{
		CreateDirectoryA(path, NULL);
		ord = CreateFileA(ordi, FILE_ALL_ACCESS, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL, NULL);
		ext = CreateFileA(extr, FILE_ALL_ACCESS, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL, NULL);

		showMenu = TRUE;
	}
	else
	{
		ord = CreateFileA(ordi, FILE_ALL_ACCESS, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL);
		ext = CreateFileA(extr, FILE_ALL_ACCESS, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL);
		if (GetFileSize(ord, NULL) == 0)
			showMenu = TRUE;
	}

	WNDCLASSEX wc;
	HWND hwnd;
	MSG msg;

	// Registering the Window Class
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = windowProcedure;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = current;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)CreateSolidBrush(backgroundColor);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = g_szClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassExA(&wc))
	{
		MessageBoxA(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	// Creating the Window
	hwnd = CreateWindowExA(WS_EX_CLIENTEDGE, g_szClassName, "The title of my window", 
		WS_OVERLAPPEDWINDOW & WS_MAXIMIZEBOX & WS_MINIMIZEBOX, 789, -52, 500, 744, NULL, NULL, current, NULL);

	if (hwnd == NULL)
	{
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	ShowWindow(hwnd, cmdShow);
	UpdateWindow(hwnd);

	// The Message Loop
	while (GetMessageA(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	return msg.wParam;
}