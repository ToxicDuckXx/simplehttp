# simplehttp
A minimalist windows c http server.

## Sample Usage
```c
#include <httpserver.h>

int main()
{
  set_static_folder("imgs");
  register_url("/", home);
  start_server("127.0.0.1", 8000);
  return 0;
}

void home(int client_socket)
{
  serve_template(client_socket, "index.html");
}
```
## Known Bugs
* The static folder has to be called 'imgs'.
* The static folder can only serve png files.
* You can server other files from other folders by registering a new url and using the server functions.

## Functions
|Function            |Description         |
|--------------------|--------------------|
|set_static_folder(const char* path) |Sets static folder  |
|register_url(const char* url, void* handler)      |User register new url. Pass a handler function defined in your code.|
|server_template(int client_socket, const char* path)|Used in handler function. Serves a html template to user|
|serve_static_text(int client_socket, const char* path, const char* type)|Used in handler function. Serves a static text file. Should be used for css and js. Type is the Content-Type header that will be passed.
|serve_static_blob(int client_socket, const char* path, const char* type)|Used in handler function. Serves static media. Should be used for pictures, audio etc. Type is the Content-Type header that will be passed.
|start_server(const char* host, int port)|Starts the server. Should be called after registering all urls.

List of Content-Types that can be used: https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types
  
