#include "kwm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// 内部函数声明
static dl getFileSize(FILE *fp);
static int cmpCharArray(const char *a1, int a1Len, const char *a2, int a2Len);
static int IsKWMFile(const char* filename);

// 获取文件大小
dl getFileSize(FILE *fp) {
    dl sz;
    fseek(fp, 0L, SEEK_END);
    sz = (dl)ftell(fp);
    if (sz == -1) {
        sz = (dl)ftell(fp);
    }
    fseek(fp, 0L, SEEK_SET);
    return sz;
}

// 比较字符数组
int cmpCharArray(const char *a1, int a1Len, const char *a2, int a2Len) {
    int i;
    if (a1Len != a2Len) return 0;
    for (i = 0; i < a1Len; ++i) {
        if (a1[i] != a2[i]) return 0;
    }
    return 1;
}

// 检查文件扩展名
int IsKWMFile(const char* filename) {
    const char* ext = strrchr(filename, '.');
    return (ext && _stricmp(ext, ".kwm") == 0);
}

// 解密单个KWM文件
__declspec(dllexport) void DecryptKWMFile(const char* inputFile, const char* outputFile) {
    int haveFoundKey = 0;
    FILE *fp = NULL, *fpO = NULL;
    int i, j, k, l;
    char key[32] = {0}, old_key[32] = {0};
    dl fS;
    dl a;
    usi b;
    char buf[1024] = {0};
    char *newKey = NULL;
    
    // 打开输入文件
    if ((fp = fopen(inputFile, "rb")) == NULL) {
        char msg[256];
        sprintf(msg, "Cannot open input file: %s", inputFile);
        MessageBoxA(NULL, msg, "Error", MB_ICONERROR);
        return;
    }
    
    // 打开输出文件
    if ((fpO = fopen(outputFile, "wb")) == NULL) {
        char msg[256];
        sprintf(msg, "Cannot create output file: %s", outputFile);
        MessageBoxA(NULL, msg, "Error", MB_ICONERROR);
        fclose(fp);
        return;
    }
    
    fS = getFileSize(fp);
    a = fS / 1024;
    b = (usi)(fS % 1024);
    
    // 跳过前1024字节
    fseek(fp, 1024L, SEEK_SET);
    
    // 查找密钥
    for (i = 0; i < MAX_FIND_KEY_TIME; ++i) {
        if (fread(key, 32, 1, fp) != 1) {
            break;
        }
        int cmpR = cmpCharArray(key, 32, old_key, 32);
        if (cmpR) {
            haveFoundKey = 1;
            break;
        }
        for (j = 0; j < 32; ++j) {
            old_key[j] = key[j];
        }
    }
    
    // 如果没有找到密钥，尝试交换密钥
    if (!haveFoundKey) {
        newKey = (char *)malloc(32);
        for (i = 0; i < 16; ++i) {
            newKey[i] = key[i + 16];
        }
        for (j = 16; j < 32; ++j) {
            newKey[j] = key[j - 16];
        }
        for (k = 0; k < 32; ++k) {
            key[k] = newKey[k];
        }
        free(newKey);
    }
    
    // 回到数据开始位置
    fseek(fp, 1024L, SEEK_SET);
    
    // 解密数据
    for (l = 1; l < a; ++l) {
        if (fread(buf, 1024, 1, fp) != 1) {
            break;
        }
        for (i = 0; i < 1024; ++i) {
            buf[i] ^= key[i & 31];
        }
        fwrite(buf, 1024, 1, fpO);
    }
    
    // 处理剩余字节
    if (b) {
        if (fread(buf, b, 1, fp) == 1) {
            for (i = 0; i < b; ++i) {
                buf[i] ^= key[i & 31];
            }
            fwrite(buf, b, 1, fpO);
        }
    }
    
    fclose(fp);
    fclose(fpO);
}

// 解密整个文件夹中的KWM文件
__declspec(dllexport) void DecryptKWMFolder(const char* inputFolder, const char* outputFolder) {
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind;
    char searchPath[MAX_PATH];
    char inputFile[MAX_PATH];
    char outputFile[MAX_PATH];
    int fileCount = 0;
    int successCount = 0;
    char outputName[MAX_PATH];
    char* dot;
    
    // 构建搜索路径
    sprintf(searchPath, "%s\\*.kwm", inputFolder);
    
    // 检查输入文件夹是否存在
    DWORD attrib = GetFileAttributesA(inputFolder);
    if (attrib == INVALID_FILE_ATTRIBUTES || !(attrib & FILE_ATTRIBUTE_DIRECTORY)) {
        MessageBoxA(NULL, "Input folder does not exist or cannot be accessed!", "Error", MB_ICONERROR);
        return;
    }
    
    // 创建输出文件夹（如果不存在）
    CreateDirectoryA(outputFolder, NULL);
    
    // 查找KWM文件
    hFind = FindFirstFileA(searchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        MessageBoxA(NULL, "No KWM files found in input folder!", "Info", MB_ICONINFORMATION);
        return;
    }
    
    do {
        if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            fileCount++;
            
            // 构建完整的文件路径
            sprintf(inputFile, "%s\\%s", inputFolder, findFileData.cFileName);
            
            // 构建输出文件名（将.kwm改为.mp3）
            strncpy(outputName, findFileData.cFileName, sizeof(outputName));
            dot = strrchr(outputName, '.');
            if (dot) {
                strcpy(dot, ".flac");
            } else {
                strcat(outputName, ".flac");
            }
            
            sprintf(outputFile, "%s\\%s", outputFolder, outputName);
            
            // 解密文件
            DecryptKWMFile(inputFile, outputFile);
            successCount++;
        }
    } while (FindNextFileA(hFind, &findFileData) != 0);
    
    FindClose(hFind);
    
    // 显示结果
    char message[256];
    if (fileCount == 0) {
        MessageBoxA(NULL, "No KWM files found in the specified folder!", "Info", MB_ICONINFORMATION);
    } else {
        sprintf(message, 
                "Decryption completed!\nTotal files: %d\nSuccessfully decrypted: %d", 
                fileCount, successCount);
        MessageBoxA(NULL, message, "Complete", MB_ICONINFORMATION);
    }
}

// DLL入口点
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    switch (reason) {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
