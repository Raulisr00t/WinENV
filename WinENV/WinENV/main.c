#include <windows.h>
#include <stdio.h>
#include <wchar.h>  
#include "structs.h"

void LoadEnv(LPCSTR variable) {
    HMODULE hNtDll = GetModuleHandleA("ntdll.dll");
    if (!hNtDll) {
        printf("[!] Failed to load ntdll.dll\n");
        return;
    }

    pNtQueryInformationProcess NtQueryInformationProcess = (pNtQueryInformationProcess)GetProcAddress(hNtDll, "NtQueryInformationProcess");
    if (!NtQueryInformationProcess) {
        printf("[!] Failed to get NtQueryInformationProcess.\n");
        return;
    }

    ULONG requiredLength = 48;  
    PROCESS_BASIC_INFORMATION pbi = { 0 };
    NTSTATUS status = NtQueryInformationProcess(GetCurrentProcess(), ProcessBasicInformation, &pbi, requiredLength, &requiredLength);

    if (status != 0) {
        printf("[!] Failed to query process information. Error: 0x%X\n", status);
        return;
    }

    PPEB peb = pbi.PebBaseAddress;
    if (!peb) {
        printf("[!] Failed to access PEB.\n");
        return;
    }

    PRTL_USER_PROCESS_PARAMETERS params = (PRTL_USER_PROCESS_PARAMETERS)peb->ProcessParameters;
    if (!params) {
        printf("[!] Failed to access ProcessParameters.\n");
        return;
    }

    PWCHAR env = (PWCHAR)params->Environment;
    if (!env) {
        printf("[!] Failed to access environment variables.\n");
        return;
    }

    if (variable == NULL) {
        printf("[+] Loading Full Environment Variables From PEB [+]\n");
        while (*env) {
            wprintf(L"%s\n", env);
            env += wcslen(env) + 1; 
        }
    }
    else {
        
        WCHAR wVariable[256];
        MultiByteToWideChar(CP_ACP, 0, variable, -1, wVariable, 256);

        while (*env) {
            PWCHAR equalsSign = wcschr(env, L'=');
            if (equalsSign) {
                size_t nameLen = equalsSign - env;  
                if (_wcsnicmp(env, wVariable, nameLen) == 0 && wVariable[nameLen] == L'\0') {
                    wprintf(L"[+] Found: %s\n", env);
                    return;
                }
            }

            env += wcslen(env) + 1;
        }

        printf("[!] Environment Variable %s not found in PEB\n", variable);
    
    }
}

int main(int argc, const char* argv[]) {
    if (argc == 1) {
        LoadEnv(NULL);
    }

    else if (argc == 2) {
        LoadEnv(argv[1]);
    }
    
    else {
        printf("[!] USAGE: WinENV.exe\n");
        printf("[!] USAGE: WinENV.exe <variable>\n");
        return -1;
    }
    
    return 0;
}
