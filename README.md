# Simple HTTP
Simple http is a fast, minimalist webserve written in C for C/C++ for windows. Just a single header and source file, add it to your project and make sure to add -lws2_32 to the compile options.

## Features
* Serve static files
* Map urls
* Dynamic templating
* GET and POST requests
* Url argument and form parsing
* Static folder

## Sample Usage
```C
#include <httpserver.h>

void home(Request request) {
    serve_template(request.client_socket, "index.html");
}

int main() {
    register_url("/", home);
    start_server("127.0.0.1", 8080);
    return 0;
}
```
For a more detailed example with other features, see examples folder.

## Docs

|Function/Type|Desciption|
|--------|----------|
|void register_url(const char* url, UrlHandlerFunc handler)|Use to register url. Pass function that does does something with the request.|
|void start_server(const char* host, int port)|Use onces all urls are registered. Starts the server on a specified host and port number.|
|void serve_template(int client_socket, const char* path)|Used to serve a template. Pass client socket from Request object and path to template|
|void serve_static_blob(int client_socket, const char* path, const char* type)|Used to serve a static file outside of static folder. Pass client socket from Request object, path to file, and the Content-Type header to be added.|
|void set_static_folder(const char* path)|Used to set static folder. All files in static folder are served when /static_folder/file_name is requested. Pass path to pre-existing static folder.
|void serve_dynamic_template(int client_socket, const char* path, const char** data_array)|Used to serve a dynamic template. Pass client socket from Requset object, path to the template and a data array. data_array is an array of strings used to replace sequences in the template. To make a sequence to be replaced, add '{num}' to the file, where num is the element at the nums posistion to be inserted. Num must be one digit. For a detailed example, see examples folder.|
|void get_arg(const char* request, cont char* arg, const char* dest)|Used to parge an arg from a request. Pass the request element from the Request object, and argument name to find, and a destination string to write the arg to. BUG-Dest string must have memory allocated for url encoding eg. replacing spaces with %20.
|void serve_static_text(int client_socket, const char* path, const char* type)|Depreciated. Use serve_static_blob|
|Request|Object passed to handler function. Contains client socket and a string containing the full, unparsed contents of the client's request.|
|UrlHandlerFunc|A void function. Used to denote handler functions.|
