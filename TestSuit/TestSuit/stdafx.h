// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once
#undef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#define _CRT_SECURE_NO_WARNINGS


//#include "targetver.h"
#pragma region windows支持
#include <WinSock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#define INIT_WIN_SOCK()   WORD ver = MAKEWORD(2, 2);  \
                        WSADATA dat;                \
                        WSAStartup(ver, &dat)
#pragma endregion

#pragma region C++标准库
#include "iostream"
#pragma endregion

#include <stdio.h>
#include <tchar.h>
#include"base/exportbase.h"

// TODO: 在此处引用程序需要的其他头文件
