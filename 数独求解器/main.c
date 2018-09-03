#include <windows.h>
#include <process.h>
#include "resource.h"

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")

/*全局变量区*/
HINSTANCE hInst;
HWND hMainDlg;
HANDLE hThread;
int s[9][9];
int a[9][9];
COORD t[81];
int cnt;
int constant;
int r, c;
int flag;

/*对话框过程*/
BOOL CALLBACK MainDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK SelectDlgProc(HWND, UINT, WPARAM, LPARAM);
/*线程函数*/
VOID Thread(PVOID pvoid);
/*数独求解函数*/
void PreProc(void);
int Judge(int r, int c, int n);
void Next(void);
void Last(void);
void SolveSuduku(void);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	hInst = hInstance;
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc);
	return 0;
}

BOOL CALLBACK MainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rect;
	TCHAR buf[1024];
	int select;
	int i, j;

	switch (message)
	{
	case WM_INITDIALOG:
		SendMessage(hDlg, WM_SETICON, 0, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1)));
		hMainDlg = hDlg;
		EnableWindow(GetDlgItem(hDlg, IDB_ENDSOLVE), FALSE);
		/*让对话框显示在显示屏中央*/
		GetWindowRect(hDlg, &rect);
		SetWindowPos(hDlg, NULL,
			(GetSystemMetrics(SM_CXSCREEN) - rect.right + rect.left) / 2,
			(GetSystemMetrics(SM_CYSCREEN) - rect.bottom + rect.top) / 2,
			0, 0, SWP_NOZORDER | SWP_NOSIZE);
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) >= IDB_1 && LOWORD(wParam) <= IDB_81)
		{
			r = (LOWORD(wParam) - IDB_1) / 9;
			c = (LOWORD(wParam) - IDB_1) % 9;
			select = DialogBox(hInst, MAKEINTRESOURCE(IDD_SELECT), hDlg, SelectDlgProc);
			if (select == 10)
			{
				wsprintf(buf, TEXT(""));
				s[r][c] = 0;
				a[r][c] = 0;
			}
				
			else if (select != 0)
			{
				wsprintf(buf, TEXT("%d"), select);
				s[r][c] = select;
				a[r][c] = select;
			}
			else
				break;
			SetWindowText(GetDlgItem(hDlg, LOWORD(wParam)), buf);
			break;
		}
		switch (LOWORD(wParam))
		{
		case IDB_SOLVE:
			for (i = 0; i < 9; ++i)
			{
				for (j = 0; j < 9; ++j)
				{
					EnableWindow(GetDlgItem(hDlg, IDB_1 + i * 9 + j), FALSE);
				}
			}
			EnableWindow(GetDlgItem(hDlg, IDB_SOLVE), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDB_CLEAR), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDB_ENDSOLVE), TRUE);
			SetWindowText(GetDlgItem(hDlg, IDB_SOLVE), TEXT("正在求解"));
			hThread = _beginthread(Thread, 0, NULL);
			break;
		case IDB_ENDSOLVE:
			TerminateThread(hThread, 0);
			for (i = 0; i < 9; ++i)
			{
				for (j = 0; j < 9; ++j)
				{
					a[i][j] = s[i][j];
					EnableWindow(GetDlgItem(hDlg, IDB_1 + i * 9 + j), TRUE);
				}
			}
			EnableWindow(GetDlgItem(hMainDlg, IDB_SOLVE), TRUE);
			EnableWindow(GetDlgItem(hMainDlg, IDB_CLEAR), TRUE);
			EnableWindow(GetDlgItem(hMainDlg, IDB_ENDSOLVE), FALSE);
			SetWindowText(GetDlgItem(hMainDlg, IDB_SOLVE), TEXT("求解"));
			break;
		case IDB_CLEAR:
			for (i = 0; i < 9; ++i)
			{
				for (j = 0; j < 9; ++j)
				{
					s[i][j] = a[i][j] = 0;
					SetWindowText(GetDlgItem(hDlg, IDB_1 + i * 9 + j), TEXT(""));
				}
			}
			break;
		default:
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	default:
		break;
	}
	return FALSE;
}

BOOL CALLBACK SelectDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int i;

	switch (message)
	{
	case WM_INITDIALOG:
		/*禁用按钮*/
		for (i = 1; i <= 9; ++i)
		{
			if (!Judge(r, c, i))
				EnableWindow(GetDlgItem(hDlg, IDB_NUM1 + i - 1), FALSE);
		}
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) >= IDB_NUM1 && LOWORD(wParam) <= IDB_NUM9)
		{
			EndDialog(hDlg, LOWORD(wParam)-IDB_NUM1+1);
			return TRUE;
		}
		switch (LOWORD(wParam))
		{
		case IDB_DELETE:
			EndDialog(hDlg, 10);
			return TRUE;
		default:
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	default:
		break;
	}
	return FALSE;
}

VOID Thread(PVOID pvoid)
{
	int i, j;
	TCHAR buf[1024];

	r = c = 0;
	flag = 0;
	PreProc();
	cnt = -1;
	Next();
	SolveSuduku();

	for (i = 0; i < 9; ++i)
	{
		for (j = 0; j < 9; ++j)
		{
			s[i][j] = a[i][j];
			wsprintf(buf, TEXT("%d"), a[i][j]);
			SetWindowText(GetDlgItem(hMainDlg, IDB_1 + i * 9 + j), buf);
			EnableWindow(GetDlgItem(hMainDlg, IDB_1 + i * 9 + j), TRUE);
		}
	}
	EnableWindow(GetDlgItem(hMainDlg, IDB_SOLVE), TRUE);
	EnableWindow(GetDlgItem(hMainDlg, IDB_CLEAR), TRUE);
	EnableWindow(GetDlgItem(hMainDlg, IDB_ENDSOLVE), FALSE);
	SetWindowText(GetDlgItem(hMainDlg, IDB_SOLVE), TEXT("求解"));

	_endthread();
}


































void PreProc(void)
{
	cnt = 0;
	int i, j;
	for (i = 0; i < 9; ++i)
	{
		for (j = 0; j < 9; ++j)
		{
			if (s[i][j] == 0)
			{
				t[cnt].X = i;
				t[cnt].Y = j;
				cnt++;
			}
		}
	}
	constant = cnt - 1;
}

int Judge(int r, int c, int n)
{
	int i, j;

	/*判断同行有无重复*/
	for (j = 0; j < 9; ++j)
	{
		if (a[r][j] == n)
			return 0;
	}

	/*判断同列有无重复*/
	for (i = 0; i < 9; ++i)
	{
		if (a[i][c] == n)
			return 0;
	}

	/*判断同宫有无重复*/
	switch (r % 3)
	{
	case 0:
		switch (c % 3)
		{
		case 0:
			if (a[r + 1][c + 1] == n || a[r + 1][c + 2] == n || a[r + 2][c + 1] == n || a[r + 2][c + 2] == n)
				return 0;
			break;
		case 1:
			if (a[r + 1][c - 1] == n || a[r + 1][c + 1] == n || a[r + 2][c - 1] == n || a[r + 2][c + 1] == n)
				return 0;
			break;
		case 2:
			if (a[r + 1][c - 2] == n || a[r + 1][c - 1] == n || a[r + 2][c - 2] == n || a[r + 2][c - 1] == n)
				return 0;
			break;
		}
		break;
	case 1:
		switch (c % 3)
		{
		case 0:
			if (a[r - 1][c + 1] == n || a[r - 1][c + 2] == n || a[r + 1][c + 1] == n || a[r + 1][c + 2] == n)
				return 0;
			break;
		case 1:
			if (a[r - 1][c - 1] == n || a[r - 1][c + 1] == n || a[r + 1][c - 1] == n || a[r + 1][c + 1] == n)
				return 0;
			break;
		case 2:
			if (a[r - 1][c - 2] == n || a[r - 1][c - 1] == n || a[r + 1][c - 2] == n || a[r + 1][c - 1] == n)
				return 0;
			break;
		}
		break;
	case 2:
		switch (c % 3)
		{
		case 0:
			if (a[r - 1][c + 1] == n || a[r - 1][c + 2] == n || a[r - 2][c + 1] == n || a[r - 2][c + 2] == n)
				return 0;
			break;
		case 1:
			if (a[r - 1][c - 1] == n || a[r - 1][c + 1] == n || a[r - 2][c - 1] == n || a[r - 2][c + 1] == n)
				return 0;
			break;
		case 2:
			if (a[r - 1][c - 2] == n || a[r - 1][c - 1] == n || a[r - 2][c - 2] == n || a[r - 2][c - 1] == n)
				return 0;
			break;
		}
		break;
	}

	return 1;
}

void Next(void)
{
	cnt++;
	r = t[cnt].X;
	c = t[cnt].Y;
}

void Last(void)
{
	cnt--;
	r = t[cnt].X;
	c = t[cnt].Y;
}

void SolveSuduku(void)
{
	if (flag)
		return;

	if (cnt == constant + 1)
	{
		flag = 1;
		return;
	}

	int i;
	for (i = 1; i <= 9; ++i)
	{
		if (Judge(r, c, i))
		{
			a[r][c] = i;
			Next();
			SolveSuduku();
			if (flag)
				return;
		}
	}

	a[r][c] = 0;
	Last();

	return;
}
