/*
The Texture Viewer Project, http://imagetools.itch.io/texview
Copyright (c) 2011-2024 Ilya Lyutin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#pragma once


class CFileList
{
public:

	CFileList(void);
	~CFileList();

	HWND GetWnd(void);

	void Init(void);

	void Clear(void);
	void SetNumItems(int n);
	void AddItem(const wchar_t* pszFileName, int iFile);
	void UpdateList(void);

	//void UnselAll(void);
	void SetSel(int iFile);
	bool IsMarked(int i);
	void SetMarked(int i, bool bMarked);
	void SetSelectionMarked(bool bMarked);
	void ToggleSelectionMarked(void);
	void SetAllMarked(bool bMarked);
	void MarkInvert(void);
	void SelectMarked(void);
	int CountMarked( void );

	static LRESULT CALLBACK ListWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	
	HWND m_hWnd;
	HWND m_hList;
	HWND m_hMenuButton;
	HMENU m_hMarkedGroupMenu;
	HMENU m_hItemMenu;
	HMENU m_hPop;

	bool CheckCanPerformFileOperation(void);
	void InvokeShellCommand(char* pszCmd);
	void ShellCopy(void);
	void ShellCut(void);
	void ShellDelete(void);
	void ShellProperties(void);

	void CopyName(void);
	void ExportList(bool bMarked, bool bUnicode);
	void RemoveMarkedFromList(void);

	int Create(HWND hWnd, CREATESTRUCT *pcs);
	void Destroy(void);
	void Size(int x, int y, int flags);
	void RButtonUp(int fKeys, int x, int y);
	void Command(int iCtrl, int eNotifyCode);
	void Notify(int iCtrl, NMHDR* pnmhdr);
	void Close(void);
};


extern CFileList* g_pFileList;

void InitFileList(HWND hWndParent);

