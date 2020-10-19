#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include <Windows.h>
#include <iostream>
#include <map>
#include <regex>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#include "MurMurHash3.h"
#pragma comment(lib, "User32.lib")

void DisplayError(LPCTSTR lpszFunction);

int _tmain(int argc, TCHAR *argv[])
{
	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize[2];
	TCHAR szDir[2][MAX_PATH], szDirWithWildcard[MAX_PATH], szFileNameWithPath[MAX_PATH];
	size_t lengthFile[2];
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;

	// If the directory paths are not specified as a command-line argument, print usage.
	if (argc != 3)
	{
		_tprintf(_T("Usage: %s \"<1st directory path>\" \"<2nd directory path>\"\n"), argv[0]);
		return (-1);
	}

	// Check that the input paths plus 3 are not longer than MAX_PATH.
	// Three characters are for the "\*" plus NULL appended below.
	for (int counter = 0; counter < 2; counter++)
	{
		StringCchLength(argv[counter + 1], MAX_PATH, &lengthFile[counter]);
		if (lengthFile[counter] > (MAX_PATH - 3))
		{
			_tprintf(_T("Directory path is too long.\n"));
			return (-1);
		}
		StringCchCopy(szDir[counter], MAX_PATH, argv[counter + 1]);
	}

	_tprintf(_T("1st target directory is '%s'\n"), szDir[0]);
	_tprintf(_T("2nd target directory is '%s'\n"), szDir[1]);

	// Create dictionaries with hashes of file contents and file names.
	std::map<std::wstring, std::vector<std::wstring>> dictionaries[2];
	for (int counter = 0; counter < 2; counter++)
	{
		_tprintf(_T("Creating hashes for directory '%s'\n"), szDir[counter]);

		StringCchCopy(szDirWithWildcard, MAX_PATH, szDir[counter]);
		StringCchCat(szDirWithWildcard, MAX_PATH, _T("\\*"));

		hFind = FindFirstFile(szDirWithWildcard, &ffd);
		if (INVALID_HANDLE_VALUE == hFind)
		{
			LPCTSTR text = _T("FindFirstFile");
			DisplayError(text);
			return dwError;
		}

		do
		{
			if ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == false)
			{
				// Concatenate file path and name.
				StringCchCopy(szFileNameWithPath, MAX_PATH, argv[counter + 1]);
				StringCchCat(szFileNameWithPath, MAX_PATH, _T("\\"));
				StringCchCat(szFileNameWithPath, MAX_PATH, ffd.cFileName);
				
				// Create file.
				HANDLE hFile = CreateFile(szFileNameWithPath, 
					GENERIC_READ,          
					FILE_SHARE_READ,       
					NULL,                  
					OPEN_EXISTING,         
					FILE_ATTRIBUTE_NORMAL ,
					NULL);                 

				// Read file.
				DWORD dwFileSize;
				DWORD dwBytesRead = 0;
				LPOVERLAPPED ol = { 0 };
				dwFileSize = GetFileSize(hFile, NULL);
				LPBYTE data = new BYTE[dwFileSize + 1];

				if (FALSE == ReadFile(hFile, data, dwFileSize, &dwBytesRead, NULL))
				{
					DisplayError(_T("ReadFile"));
					printf("Terminal failure: Unable to read from file.\n GetLastError=%08x\n", GetLastError());
					CloseHandle(hFile);
					return -1;
				}

				// Make file hash.
				uint64_t seed = 1;
				uint64_t hashParts[2];
				MurmurHash3_x64_128(data, (uint64_t)dwBytesRead, seed, hashParts);
				
				std::wstring hash = std::to_wstring(hashParts[0]);
				hash += std::to_wstring(hashParts[1]);

				// Put file name into dictionary.
				if (dictionaries[counter].find(hash) != dictionaries[counter].end())
					dictionaries[counter][hash].push_back(ffd.cFileName);
				else
				{
					std::vector<std::wstring> filePaths = std::vector<std::wstring>();
					filePaths.push_back(ffd.cFileName);
					dictionaries[counter].insert(std::make_pair(hash, filePaths));
				}
				CloseHandle(hFile);
				delete[] data;
			}
		} while (FindNextFile(hFind, &ffd) != 0);

		dwError = GetLastError();
		if (dwError != ERROR_NO_MORE_FILES)
		{
			DisplayError(L"FindFirstFile");
		}

		FindClose(hFind);
	}

	// Compare files - iterate through one dictionary and find matches in the other by comparing hashes, perform this for both dictionaries.
	for (int counter = 0; counter < 2; counter++)
	{
		_tprintf(_T("\nSearching for content matches of files in directory '%s'\n"), szDir[counter]);

		std::map<std::wstring, std::vector<std::wstring>>::iterator itDictionary;
		for (itDictionary = dictionaries[counter].begin(); itDictionary != dictionaries[counter].end(); itDictionary++)
		{
			std::wcout << _T("File ");

			std::vector<std::wstring>::iterator itFile;
			for (itFile = itDictionary->second.begin(); itFile != itDictionary->second.end(); ++itFile)
				std::wcout << *itFile << _T(" ");

			std::wcout << _T(" matches:") << std::endl;

			int otherDictIndex = 0;
			if (counter == 0)
				otherDictIndex = 1;

			std::wstring key = itDictionary->first;
			bool found = false;
			if (dictionaries[otherDictIndex].find(key) != dictionaries[otherDictIndex].end())
			{
				std::vector<std::wstring> matchedFiles = dictionaries[otherDictIndex][key];
				std::vector<std::wstring>::iterator itMatchedFile;
				for (itMatchedFile = matchedFiles.begin(); itMatchedFile != matchedFiles.end(); ++itMatchedFile)
					std::wcout << _T("     ") << *itMatchedFile << std::endl;

				found = true;
			}
			if (found == false)
				std::wcout << _T("     -") << std::endl;
		}
	}

	return dwError;
}


void DisplayError(LPCTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code.
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and clean up.
	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen(lpszFunction) + 40) * sizeof(TCHAR));

	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		_T("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);

	std::wcout << L"Error: " << (LPCTSTR)lpDisplayBuf << std::endl;

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}