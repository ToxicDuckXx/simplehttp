#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <process.h>
#include <dirent.h>

#include <winsock2.h>
#include <windows.h>
#include "httpserver.h"

typedef struct {
    const char* url;
    UrlHandlerFunc handler;
} UrlHandler;
static UrlHandler url_handlers[25];
static int url_handler_count = 0;
static const char* static_folder = NULL;
static char static_files[50][MAX_PATH];
static int static_file_count = 0;

unsigned int __stdcall thread_handler(void* data) {

    int client_socket = *(int*)data;
    char rec_buffer[1024];
    int bytes_received;

    bytes_received = recv(client_socket, rec_buffer, sizeof(rec_buffer), 0);
    char requested_url[256];
    if (sscanf(rec_buffer,  "GET %255s", requested_url) != 1) {
        printf("Failed to parse request\n");
        closesocket(client_socket);
        return 1;
    }

    for (int i = 0; i < url_handler_count; i++) {
        if (strcmp(requested_url, url_handlers[i].url) == 0) {
            url_handlers[i].handler(client_socket);
            break;
        }
    }
    int len = strlen(requested_url);
    for (int t = 1; t < len; t++) {
        requested_url[t - 1] = requested_url[t];
    }
    requested_url[len - 1] = '\0'; 
    for (int o = 0; o < static_file_count; o++) {
        if (strcmp(requested_url, static_files[o]) == 0) {
            len = strlen(static_files[o]);
            char temp_str[len + 7];
            strcpy(temp_str, "imgs/");
            strcpy(temp_str + 5, static_files[o]);
            serve_static_blob(client_socket, temp_str, "image/png");
        }
    }
    _endthreadex(0);
    return 0;
}

void start_server(const char* host, int port)
{

    //load static files
    if (!(static_folder == NULL)) {
        WIN32_FIND_DATA FindFileData;
        char search_path[MAX_PATH];
        HANDLE hFind; 
        snprintf(search_path, sizeof(search_path), "%s\\*", static_folder);
        hFind =FindFirstFile(search_path, &FindFileData);
        if (hFind == INVALID_HANDLE_VALUE) {
            printf("Error opening static DIR");
            return;
        }
        if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

            strcpy(static_files[static_file_count], FindFileData.cFileName);
            static_file_count += 1;
        }

        // Loop through all other files in the directory
        while (FindNextFile(hFind, &FindFileData)) {
            if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                strcpy(static_files[static_file_count], FindFileData.cFileName);
                static_file_count += 1;
            }
        }
        FindClose(hFind);
    } 
    

    WSADATA wsa;
    SOCKET s;
    char c = 0;

    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0)
    {
        printf("Failed. Error Code : %d.\nPress a key to exit...", WSAGetLastError());
        c = getch();
        return;
    }
    int server_socket;
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0))==INVALID_SOCKET) {
        printf("Failed. Error Code : %d",WSAGetLastError());
        int c = getch();
        return;
    }

    // define the address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(host);
    bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));

    listen(server_socket, 5);

    int client_socket;
    while(1) {
        client_socket = accept(server_socket, NULL, NULL);
        uintptr_t thread_id;
        _beginthreadex(NULL, 0, &thread_handler, &client_socket, 0, (unsigned int*)&thread_id);
        
    }
}

void register_url(const char* url, UrlHandlerFunc handler)
{
    if (url_handler_count < sizeof(url_handlers) / sizeof(url_handlers[0])) {
        url_handlers[url_handler_count].url = url;
        url_handlers[url_handler_count].handler = handler;
        url_handler_count++;
    } else {
        printf("URL Registered Overflow Error");
    }
}


void serve_tempate(int client_socket, const char* path) 
{
    char* http_response;
        FILE* html_file = fopen(path, "rb");
        if (html_file == NULL) {
            const char* response_content = "<p>500 - Internal Server Error: Could not open CSS file</p>";
            asprintf(&http_response, "HTTP/1.1 500 Internal Server Error\r\n\r\n%s", response_content);
        } else {
            fseek(html_file, 0, SEEK_END);
            long file_size = ftell(html_file);
            fseek(html_file, 0, SEEK_SET);

            if (file_size <= 0) {
                const char* response_content = "<p>500 - Internal Server Error: Empty CSS file</p>";
                asprintf(&http_response, "HTTP/1.1 500 Internal Server Error\r\n\r\n%s", response_content);
            } else {
                char* html_content = (char*)malloc(file_size + 1);
                fread(html_content, 1, file_size, html_file);
                html_content[file_size] = '\0';

                fclose(html_file);

                asprintf(&http_response, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n%s", html_content);
                free(html_content);
            }
            send(client_socket, http_response, strlen(http_response), 0);
            
            free(http_response);
        }
    shutdown(client_socket, SD_SEND);
    closesocket(client_socket);
}

void serve_static_text(int client_socket, const char* path, const char* type) 
{
    char* http_response;
        FILE* html_file = fopen(path, "rb");
        if (html_file == NULL) {
            const char* response_content = "<p>500 - Internal Server Error: Could not open CSS file</p>";
            asprintf(&http_response, "HTTP/1.1 500 Internal Server Error\r\n\r\n%s", response_content);
        } else {
            fseek(html_file, 0, SEEK_END);
            long file_size = ftell(html_file);
            fseek(html_file, 0, SEEK_SET);

            if (file_size <= 0) {
                const char* response_content = "<p>500 - Internal Server Error: Empty CSS file</p>";
                asprintf(&http_response, "HTTP/1.1 500 Internal Server Error\r\n\r\n%s", response_content);
            } else {
                char* html_content = (char*)malloc(file_size + 1);
                fread(html_content, 1, file_size, html_file);
                
                html_content[file_size] = '\0';
                
                
                fclose(html_file);
                char headers[1024];
                strcat(headers, "HTTP/1.1 200 OK\r\nContent-Type: ");
                strcat(headers, type);
                strcat(headers, "\r\n\r\n%s");
                asprintf(&http_response, headers, html_content);
                free(html_content);
            }
            send(client_socket, http_response, strlen(http_response), 0);
            
            free(http_response);
        }
    shutdown(client_socket, SD_SEND);
    closesocket(client_socket);
}

void serve_static_blob(int client_socket, const char* path, const char* type)
{
    char* http_response = NULL;
    FILE* file = fopen(path, "rb");
    
    if (file == NULL) {
        const char* response_content = "<p>500 - Internal Server Error: Could not open file</p>";
        asprintf(&http_response, "HTTP/1.1 500 Internal Server Error\r\n\r\n%s", response_content);
    } else {
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        if (file_size <= 0) {
            const char* response_content = "<p>500 - Internal Server Error: Empty file</p>";
            asprintf(&http_response, "HTTP/1.1 500 Internal Server Error\r\n\r\n%s", response_content);
        } else {
            char* file_content = (char*)malloc(file_size);
            fread(file_content, 1, file_size, file);

            fclose(file);
            
            // Concatenate type before \r\n\r\n in headers
            char headers[1028];
            
            asprintf(&http_response, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", type);
            
            // Send headers
            send(client_socket, http_response, strlen(http_response), 0);
            
            // Send file content
            send(client_socket, file_content, file_size, 0);
            
            free(file_content);
        }

        free(http_response);
    }
    
    shutdown(client_socket, SD_SEND);
    closesocket(client_socket);
}

void set_static_folder(const char* path)
{
    static_folder = path;
}