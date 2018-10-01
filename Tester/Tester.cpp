// Tester.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <functional>
using namespace std;

int main()
{
	using startFunction = void __stdcall(const wchar_t* moduleName);

	auto module = L"C:\\Users\\urieg\\source\\repos\\HttpServer\\Server\\bin\\Release\\netcoreapp2.1\\win10-x64\\publish\\Server.dll";
	auto handle = LoadLibraryA("CoreHoster.dll");
	auto proc = GetProcAddress(handle, "Start");
	auto start = (startFunction*)proc;
	start(module);
	std::cout << "Hello World!\n"; 
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
