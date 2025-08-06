C HTTP Server

C HTTP Server is a lightweight, custom-built HTTP server implemented in C. It's designed to handle basic HTTP methods (GET, POST, PUT, DELETE), serve static files, and interact with a JSON file for data persistence. This project demonstrates core networking concepts, HTTP protocol parsing, multi-threading for concurrent request handling, and basic CRUD (Create, Read, Update, Delete) operations with JSON data.

🚀 Features

    Multi-threaded Architecture: Handles concurrent client connections efficiently using POSIX threads (pthreads).

    Custom HTTP Parser: Parses raw HTTP requests (method, URI, headers, body) and constructs structured HTTPRequest objects.

    HTTP Method Support:

        GET: Serves static HTML files from the server's root directory.

        POST: Adds new key-value entries to data.json via the /api/data endpoint.

        PUT: Updates existing key-value entries in data.json via the /api/data/1 endpoint.

        DELETE: Removes key-value entries from data.json via the /api/data endpoint.

    JSON Data Persistence: Uses the cJSON library to read from and write to a data.json file for storing application data.

    URL Decoding: Includes functionality to decode URL-encoded form data.

    Modular Design: Code is organized into logical units (Server, HTTPRequest, Launch/Handler) for clarity and maintainability.

    Basic Error Handling: Implements checks for common network and file operation errors.

🛠️ Build and Run

Prerequisites

    A C compiler (e.g., GCC)

    make (for compiling)

    cJSON library (ensure it's installed or available in your include/library paths)

        On Debian/Ubuntu:
        Bash

sudo apt-get install libcjson-dev

On macOS (using Homebrew):
Bash

        brew install cjson

        Building from source (if not available via package manager):
        Download cJSON from https://github.com/DaveGamble/cJSON and follow its build instructions.

Compilation

Navigate to the project's root directory in your terminal and compile using make:
Bash

make

This will create an executable file named server.

Running the Server

Execute the compiled server:
Bash

./server

The server will start listening on http://127.0.0.1:8080 (or http://localhost:8080). You'll see messages indicating the server is waiting for connections.

🖥️ Usage

Once the server is running, you can interact with it using a web browser or tools like curl.

Static File Serving (GET Requests)

    Open your browser and navigate to:

        http://localhost:8080/ (serves Homepage.html)

        http://localhost:8080/add (serves Add Data.html)

        http://localhost:8080/edit (serves Edit Data.html)

API Endpoints

The server provides a simple API for managing key-value data in data.json.

1. Add Data (POST)

Adds a new entry to data.json.

    Endpoint: POST /api/data

    Content-Type: application/x-www-form-urlencoded

    Body Format: name=your_name&value=your_value

Example (using curl):
Bash

curl -X POST -H "Content-Type: application/x-www-form-urlencoded" -d "name=item1&value=valueA" http://localhost:8080/api/data

2. Update Data (PUT)

Updates an existing entry in data.json based on its name.

    Endpoint: PUT /api/data/1 (Note: /1 is a placeholder in the URI; the actual item is identified by name in the body)

    Content-Type: application/x-www-form-urlencoded

    Body Format: name=existing_name&value=new_value

Example (using curl):
Bash

curl -X PUT -H "Content-Type: application/x-www-form-urlencoded" -d "name=item1&value=updatedValueB" http://localhost:8080/api/data/1

3. Delete Data (DELETE)

Deletes an entry from data.json based on its name.

    Endpoint: DELETE /api/data

    Content-Type: application/x-www-form-urlencoded

    Body Format: name=name_to_delete

Example (using curl):
Bash

curl -X DELETE -H "Content-Type: application/x-www-form-urlencoded" -d "name=item1" http://localhost:8080/api/data

📂 Project Structure

.
├── Server.h            # Server struct definition and constructor declaration
├── Server.c            # Server socket creation, binding, listening
├── HTTPRequest.h       # HTTPRequest struct, enum for methods, header struct
├── HTTPRequest.c       # HTTP request parsing logic (method, URI, headers, body)
├── launch.c            # Main server loop, client handling (threading), routing, API logic
├── Homepage.html       # Static HTML file served on '/'
├── Add Data.html       # Static HTML file served on '/add'
├── Edit Data.html      # Static HTML file served on '/edit'
├── data.json           # JSON file for data storage (created/modified by server)
└── Makefile            # Build instructions

💡 Design Choices & Implementation Details

    Server Module: Encapsulates the core server properties like domain, service type, protocol, port, and the socket itself. The server_constructor function handles the necessary socket(), bind(), and listen() calls.

    HTTPRequest Module: Provides a structured way to represent incoming HTTP requests. The http_request_constructor function meticulously parses the raw HTTP request string, extracting key components like the method, URI, HTTP version, headers, and body. It also includes a destructor for proper memory cleanup.

    launch.c (Main Logic):

        The main function initializes the Server and enters an infinite loop, continuously calling server.launch().

        The launch function is where new client connections are accept()ed. Crucially, it then uses pthread_create to spawn a new thread (handle_client) for each accepted connection. This makes the server non-blocking and capable of handling multiple requests concurrently.

        The handle_client function reads the full HTTP request, calls the http_request_constructor, and then implements the routing logic based on the request's method and URI.

        It includes helper functions for serving HTML files (send_file_response), handling 404s (send_404_response), and interacting with data.json (save_to_json_file, edit_json_entry, delete_json_entry).

        The parse_form_data and url_decode functions handle application/x-www-form-urlencoded data parsing.

    Error Handling: perror is used to print system error messages for network operations, providing useful debugging information.

    Memory Management: Dynamic memory allocation (malloc, strdup) is used for parsing parts of the HTTP request (like URI, header keys/values, and body). Corresponding free calls are strategically placed in http_request_destructor and handle_client to prevent memory leaks.

🤝 Contributing

Feel free to fork this repository, open issues, or submit pull requests.
