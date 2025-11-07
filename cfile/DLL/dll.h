#ifndef KWM_H
#define KWM_H

#include <windows.h>

// 类型定义
typedef long long dl;
typedef unsigned short usi;

#define ARR_len(arr) (sizeof(arr) / sizeof((arr)[0]))
#define MAX_FIND_KEY_TIME 468

// 函数声明
#ifdef __cplusplus
extern "C" {
#endif

// 导出函数 - 这些是DLL的主要接口
__declspec(dllexport) void DecryptKWMFolder(const char* inputFolder, const char* outputFolder);
__declspec(dllexport) void DecryptKWMFile(const char* inputFile, const char* outputFile);

#ifdef __cplusplus
}
#endif

#endif // KWM_H
