#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <process.h>

#include <winsock2.h>
#include <windows.h>
#include "httpserver.h"

typedef struct {
    const char* url;
    UrlHandlerFunc handler;
} UrlHandler;
static const char* error_handler_path;
static UrlHandler url_handlers[MAX_URLS];
static int url_handler_count = 0;
static const char* static_folder = NULL;
static char static_files[MAX_STATIC_FILES][MAX_PATH];
static int static_file_count = 0;

//Eventuall should probably do this from a file instead of just a lot of if statements. 
static const char* file_to_mime_type(const char* path) {
    
    const char* last_dot = strrchr(path, '.');
    if (strcmp(last_dot + 1, "png")==0) {return "image/png";}
    else if (strcmp(last_dot + 1, "jpg")==0) {return "image/jpeg";}
    else if (strcmp(last_dot + 1, "css")==0) {return "text/css"; }
    else if (strcmp(last_dot + 1, "jpg")==0) {return "text/jpeg";}
    else if (strcmp(last_dot + 1, "js")==0) {return "text/javascript";}
    else if (strcmp(last_dot + 1, "csv")==0) {return "text/csv";}
    else if (strcmp(last_dot + 1, "mp3")==0) {return "audio/mpeg";}
    else if (strcmp(last_dot + 1, "mp4")==0) {return "video/mp4";}
    else if (strcmp(last_dot + 1, "pdf")==0) {return "application/pdf";}
    else if (strcmp(last_dot + 1, "webp")==0) {return "image/webp";}
    else {return "";}
} 

unsigned int __stdcall thread_handler(void* data) {

    int client_socket = *(int*)data;
    char rec_buffer[1024];
    int bytes_received;

    bytes_received = recv(client_socket, rec_buffer, sizeof(rec_buffer), 0);
    char requested_url[256];
    if (sscanf(rec_buffer,  "GET %255[^ \t?]", requested_url) != 1) {
        printf("Failed to parse request\n");
        closesocket(client_socket);
        return 1;
    }

    for (int i = 0; i < url_handler_count; i++) {
        if (strcmp(requested_url, url_handlers[i].url) == 0) {
            Request request;
            char type[10];
            sscanf(rec_buffer, "%9[^ ]", type);
            if ((strcmp(type, "POST")) == 0) {request.type = POST;}
            else if ((strcmp(type, "GET")) == 0) {request.type = GET;}
            else {request.type = OTHER;}
            request.client_socket = client_socket;
            request.request = rec_buffer;
            url_handlers[i].handler(request);
            _endthreadex(0);
            return 0;
        }
    }
    for (int o = 0; o < static_file_count; o++) {
        if (strcmp(requested_url, static_files[o]) == 0) {
            char buffer[MAX_PATH + 1];
            strcpy(buffer, static_files[o]);
            memmove(buffer, buffer+1, strlen(buffer));
            serve_static_blob(client_socket, buffer, file_to_mime_type(buffer));
            _endthreadex(0);
            return 0;
        }
    }
    if (error_handler_path == NULL) {
        char response_content[55] = "HTTP/1.1 404 Not Found\r\n\r\n<p>404 - Page not found.</p>";
        send(client_socket, response_content, strlen(response_content), 0);
        shutdown(client_socket, SD_SEND);
        closesocket(client_socket);
    } else {
        serve_tempate(client_socket, error_handler_path);
    }
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
        char path_buffer[MAX_PATH];
        if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            memset(path_buffer, 0, MAX_PATH);
            strcpy(path_buffer, "/");
            strcat(path_buffer, static_folder);
            strcat(path_buffer, "/");
            strcat(path_buffer, FindFileData.cFileName);
            strcpy(static_files[static_file_count], path_buffer);
            static_file_count += 1;
        }

        // Loop through all other files in the directory
        while (FindNextFile(hFind, &FindFileData)) {
            if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                memset(path_buffer, 0, MAX_PATH);
                strcpy(path_buffer, "/");
                strcat(path_buffer, static_folder);
                strcat(path_buffer, "/");
                strcat(path_buffer, FindFileData.cFileName);
                strcpy(static_files[static_file_count], path_buffer);
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
            free(http_response);
            
        }
        
    }
    
    shutdown(client_socket, SD_SEND);
    closesocket(client_socket);
}

void set_static_folder(const char* path)
{
    static_folder = path;
}

void serve_dynamic_template(int client_socket, const char* path, const char** data_array)
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
        
        char* html_content = (char*)malloc(file_size+1);
        fread(html_content, 1, file_size, html_file);
        //find memory needed
        html_content[file_size] = '\0';
        char* match_start;
        char* search_start = html_content;
        unsigned int new_string_length = file_size + 1;
        while ((match_start = strstr(search_start, "{")) != NULL)
        {
            if (match_start[2] == '}') {
                char index_char = match_start[1];
                if (index_char >= '0' && index_char <= '9') {
                    int index = index_char - '0';
                    const char *replacement = data_array[index];
                    if (replacement == NULL) {printf("Error, HTML parsing invalid digit between {}");}
                    unsigned int replace_len = strlen(replacement);
                    if (replace_len > 4) {
                        new_string_length += (replace_len - 4);
                        search_start = match_start + 3;
                        continue;
                    }
                }
            }
            
            search_start += 1;
        }

        //re-allocate adjusing for replacement sizes
        html_content = (char*)realloc(html_content, new_string_length);
        search_start = html_content;
        //Now actually do the replacing
        while ((match_start = strstr(search_start, "{")) != NULL)
        {
            if (match_start[2] == '}') {
                char index_char = match_start[1];
                if (index_char >= '0' && index_char <= '9') {
                    int index = index_char - '0';
                    const char *replacement = data_array[index];
                    unsigned int replace_len = strlen(replacement);
                    memmove(match_start + replace_len, match_start + 3, strlen(match_start)+3);
                    strncpy(match_start, replacement, replace_len);
                    search_start = match_start + replace_len;
                    continue;
                }
            }
            search_start++;
        }
        if (file_size <= 0) {
            const char* response_content = "<p>500 - Internal Server Error: Empty CSS file</p>";
            asprintf(&http_response, "HTTP/1.1 500 Internal Server Error\r\n\r\n%s", response_content);
        } else {

            //html_content[new_string_length] = '\0';

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

void get_arg(const char* request, const char* arg, char* dest) 
{
    const char* arg_start = strstr(request, arg);
    if (arg_start != NULL) {
        arg_start += strlen(arg) + 1;
        sscanf(arg_start, "%255[^ &]", dest);
        char* index = dest;
        char* match_start;
        
        while ((match_start = strstr(index, "%20")) != NULL) {
            memmove(match_start + 1, match_start + 3, strlen(match_start)+3);
            strncpy(match_start, " ", 1);
            index++;
        }
    }
}

void serve_redirect(int client_socket, const char* url) {
    char http_response[45 + MAX_PATH] = "HTTP/1.1 302 Found\r\nLocation: ";
    strcat(http_response, url);
    strcat(http_response, "\r\n\r\n");
    send(client_socket, http_response, strlen(http_response), 0);
}

void get_form_arg(const char* request, const char* name, char* dest) {
    const char* arg_start = strstr(request, name);
    if (arg_start != NULL) {
        arg_start += strlen(name) + 1;
        sscanf(arg_start, "%255[^ &]", dest);
    }
}

void set_error_function(const char* path) {
    error_handler_path = path;
}
