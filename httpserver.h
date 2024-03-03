#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <process.h>

#include <winsock2.h>

//Builin-function. Should not be called.
unsigned int __stdcall thread_handler(void* data);
//Builtin type. Should not be used.
typedef void (*UrlHandlerFunc)(int client_socket);
//Registers a new URL. Pass the url endpoint as a string, and a function to be run when the url is requested.
void register_url(const char* url, UrlHandlerFunc handler);
//Call this function after registering all URLs. Starts the server.
void start_server(const char* host, int port);
//Serves static text after a request. Examples: css, js. Type is the Content-Type header that will be sent.
void serve_static_text(int client_socket, const char* path, const char* type);
//Serves static blob after a request. Examples: images, audio etc. Type is the Content-Type header that will be sent.
void serve_static_blob(int client_socket, const char* path, const char* type);
//Serves HTML template after request.
void serve_tempate(int client_socket, const char* path);
//Sets the static folder. As of now, has to be called "imgs"
void set_static_folder(const char* path);
