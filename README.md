# Boost-ASIO-sample
BOOST ASIO client server example for transferring files.
- The server is asynchronous(multithreaded).
- The client is synchronous.

## Compiling

server
```
cd server
mkdir build
cd build
cmake ..
cmake --build .
```
client
```
cd client
mkdir build
cd build
cmake ..
cmake --build .
```
## Usage

server:
```
file_server <port> <file_name>
```

client:
```
client <host> <port> <output file name>
```
