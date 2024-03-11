#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <process.h>

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#define MAX_STATIC_FILES 50
#define MAX_URLS 20

enum HttpType{
    GET,
    POST,
    OTHER
};
//Request object.
typedef struct {
    //The client socket addr. Pass to serve function.
    int client_socket;
    //The request bytes. Pass to existing helper function or parse.
    const char* request;
    //The type (GET or POST) of the request. 
    enum HttpType type;
} Request;
//Builin-function. Should not be called.
unsigned int __stdcall thread_handler(void* data);
//Builtin type. Should not be used.
typedef void (*UrlHandlerFunc)(Request request);
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
//Serves a dynamic template. See docs for details. 
void serve_dynamic_template(int client_socket, const char* path, const char** data_array);
//Sets the static folder.
void set_static_folder(const char* path);
//Gets an argument from a request. 
void get_arg(const char* request, const char* arg, char* dest);
//Gets a form element.
void get_form_arg(const char* request, const char* name, char* dest);
//Redirects client
void serve_redirect(int client_socket, const char* url);
//Gets a file from a form and saves it. Not yet implemented.
void get_file(const char* request);
//Set error html page
void set_error_template(const char* path);

