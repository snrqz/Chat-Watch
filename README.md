# Chat-Watch

Chat-Watch is a simple client-server chat application designed for real-time communication with idle timeout detection. It provides alerts if there is no activity from the client for a specified period, improving communication efficiency.

## Features

- Real-time messaging between client and server.
- Idle timeout alert system.
- Compatible with GNU Unix environments such as Linux and macOS.

## Requirements

- **GCC**: To compile the C code.
- **GNU Unix Environment**: Preferably Linux or macOS.
- **A C Compiler or IDE**: To build and run the application.

## Installation

1. **Compile the Code**

   Use GCC to compile the server and client programs:

   ```sh
   gcc -o server server.c
   gcc -o client client.c ```

2. **Run the Server and Client**

Start the server in one terminal:
```sh
./server
```

Start the client in another terminal: 
```sh
./client
```
