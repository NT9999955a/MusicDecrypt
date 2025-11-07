#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shlobj.h>
#include <tchar.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(linker, "/subsystem:windows")

#define ID_EDIT_INPUT 1001
#define ID_EDIT_OUTPUT 1002
#define ID_BUTTON_START 1003
#define ID_BUTTON_BROWSE_IN 1004
#define ID_BUTTON_BROWSE_OUT 1005

// 函数声明
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void CreateControls(HWND hwnd);
void BrowseFolder(HWND hwndEdit);
BOOL StartDecryption(HWND hwnd);

// DLL函数类型定义
typedef void (*DecryptFunc)(const char*, const char*);

// 全局变量
HINSTANCE hInstance;
HWND hEditInput, hEditOutput;

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow) {
    hInstance = hInst;
    
    // 初始化通用控件
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);
    
    // 注册窗口类
    WNDCLASSEX wc;
    memset(&wc, 0, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = TEXT("DecryptWindow");
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    
    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, TEXT("Window registration failed!"), TEXT("Error"), MB_ICONERROR);
        return 0;
    }
    
    // 创建窗口
    HWND hwnd = CreateWindow(
        TEXT("DecryptWindow"),
        TEXT("Music File Decryption"),
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        500, 350,
        NULL, NULL, hInstance, NULL
    );
    
    if (!hwnd) {
        MessageBox(NULL, TEXT("Window creation failed!"), TEXT("Error"), MB_ICONERROR);
        return 0;
    }
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            CreateControls(hwnd);
            break;
            
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_BUTTON_BROWSE_IN:
                    BrowseFolder(hEditInput);
                    break;
                    
                case ID_BUTTON_BROWSE_OUT:
                    BrowseFolder(hEditOutput);
                    break;
                    
                case ID_BUTTON_START:
                    StartDecryption(hwnd);
                    break;
            }
            break;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
            
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void CreateControls(HWND hwnd) {
    // 创建标题
    CreateWindow(TEXT("STATIC"), TEXT("Music File Decryption"),
                 WS_VISIBLE | WS_CHILD | SS_CENTER,
                 20, 20, 440, 30,
                 hwnd, NULL, hInstance, NULL);
    
    // 设置标题字体
    HFONT hFont = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, 
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
                            DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("MS Shell Dlg"));
    HWND hTitle = GetDlgItem(hwnd, 0);
    if (hTitle) SendMessage(hTitle, WM_SETFONT, (WPARAM)hFont, TRUE);
    
    // 创建格式选择标签
    CreateWindow(TEXT("STATIC"), TEXT("Select Format:"),
                 WS_VISIBLE | WS_CHILD,
                 20, 60, 100, 20,
                 hwnd, NULL, hInstance, NULL);
    
    // 创建格式选择（目前只有KWM）
    CreateWindow(TEXT("BUTTON"), TEXT("KWM Format"),
                 WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,
                 20, 85, 100, 20,
                 hwnd, NULL, hInstance, NULL);
    
    // 输入文件夹标签
    CreateWindow(TEXT("STATIC"), TEXT("Input Folder:"),
                 WS_VISIBLE | WS_CHILD,
                 20, 120, 120, 20,
                 hwnd, NULL, hInstance, NULL);
    
    // 输入文件夹编辑框
    hEditInput = CreateWindow(TEXT("EDIT"), TEXT(""),
                             WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                             20, 145, 350, 25,
                             hwnd, (HMENU)ID_EDIT_INPUT, hInstance, NULL);
    
    // 输入文件夹浏览按钮
    CreateWindow(TEXT("BUTTON"), TEXT("Browse"),
                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 380, 145, 60, 25,
                 hwnd, (HMENU)ID_BUTTON_BROWSE_IN, hInstance, NULL);
    
    // 输出文件夹标签
    CreateWindow(TEXT("STATIC"), TEXT("Output Folder:"),
                 WS_VISIBLE | WS_CHILD,
                 20, 180, 120, 20,
                 hwnd, NULL, hInstance, NULL);
    
    // 输出文件夹编辑框
    hEditOutput = CreateWindow(TEXT("EDIT"), TEXT(""),
                              WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                              20, 205, 350, 25,
                              hwnd, (HMENU)ID_EDIT_OUTPUT, hInstance, NULL);
    
    // 输出文件夹浏览按钮
    CreateWindow(TEXT("BUTTON"), TEXT("Browse"),
                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                 380, 205, 60, 25,
                 hwnd, (HMENU)ID_BUTTON_BROWSE_OUT, hInstance, NULL);
    
    // 开始按钮
    CreateWindow(TEXT("BUTTON"), TEXT("Start Decryption"),
                 WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_CENTER,
                 200, 250, 100, 40,
                 hwnd, (HMENU)ID_BUTTON_START, hInstance, NULL);
}

void BrowseFolder(HWND hwndEdit) {
    BROWSEINFO bi;
    memset(&bi, 0, sizeof(BROWSEINFO));
    bi.hwndOwner = GetParent(hwndEdit);
    bi.lpszTitle = TEXT("Select Folder");
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    
    if (pidl != NULL) {
        TCHAR path[MAX_PATH];
        if (SHGetPathFromIDList(pidl, path)) {
            SetWindowText(hwndEdit, path);
        }
        CoTaskMemFree(pidl);
    }
}

BOOL StartDecryption(HWND hwnd) {
    TCHAR inputPath[MAX_PATH];
    TCHAR outputPath[MAX_PATH];
    
    GetWindowText(hEditInput, inputPath, MAX_PATH);
    GetWindowText(hEditOutput, outputPath, MAX_PATH);
    
    // 使用 lstrlen 而不是 _tcslen
    if (lstrlen(inputPath) == 0 || lstrlen(outputPath) == 0) {
        MessageBox(hwnd, TEXT("Please select both input and output folders!"), TEXT("Error"), MB_ICONWARNING);
        return FALSE;
    }
    
    // 检查输入文件夹是否存在
    DWORD attrib = GetFileAttributes(inputPath);
    if (attrib == INVALID_FILE_ATTRIBUTES || !(attrib & FILE_ATTRIBUTE_DIRECTORY)) {
        MessageBox(hwnd, TEXT("Input folder does not exist!"), TEXT("Error"), MB_ICONERROR);
        return FALSE;
    }
    
    // 创建输出文件夹（如果不存在）
    CreateDirectory(outputPath, NULL);
    
    // 加载DLL
    HINSTANCE hDll = LoadLibrary(TEXT("kwm.dll"));
    if (!hDll) {
        MessageBox(hwnd, TEXT("Cannot load kwm.dll!"), TEXT("Error"), MB_ICONERROR);
        return FALSE;
    }
    
    // 获取解密函数
    DecryptFunc decryptFunc = (DecryptFunc)GetProcAddress(hDll, "DecryptKWMFolder");
    if (!decryptFunc) {
        MessageBox(hwnd, TEXT("Cannot find decryption function in DLL!"), TEXT("Error"), MB_ICONERROR);
        FreeLibrary(hDll);
        return FALSE;
    }
    
    // 转换路径为多字节（DLL使用char*）
    char inputPathA[MAX_PATH];
    char outputPathA[MAX_PATH];
    
#ifdef UNICODE
    WideCharToMultiByte(CP_ACP, 0, inputPath, -1, inputPathA, MAX_PATH, NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, outputPath, -1, outputPathA, MAX_PATH, NULL, NULL);
#else
    lstrcpyA(inputPathA, inputPath);
    lstrcpyA(outputPathA, outputPath);
#endif
    
    // 调用DLL函数
    decryptFunc(inputPathA, outputPathA);
    
    FreeLibrary(hDll);
    return TRUE;
}
