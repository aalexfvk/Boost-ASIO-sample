#pragma once
#include <iostream>
#include <fstream>
#include <memory>
#include <thread>
#include <utility>
#include <chrono>
#include <iomanip>
#include <boost/asio.hpp>

using namespace boost::asio;
using socket_ptr = std::shared_ptr<ip::tcp::socket>;
using time_point = std::chrono::time_point<std::chrono::system_clock>;

constexpr int CHUNK_SIZE = 8 * 1024;

// Thread safe std::cout wrapper that uses stringstream
// to write buffer to cout atomically.
// Example:
//   Log() << "Hello" << " world" << std::endl;
class Log {
    std::ostringstream st;
    std::ostream &stream;
public:
    Log(std::ostream &s=std::cout):stream(s) { }
    template <typename T>
    Log& operator<<(T const& t) {
        st << t;
        return *this;
    }
    Log& operator<<( std::ostream&(*f)(std::ostream&) ) {
        st << f;
        return *this;
    }
    ~Log() { stream << st.str() << std::endl; }
};


class Client: public std::enable_shared_from_this<Client>
{
public:
    Client(const socket_ptr, std::string);
    void start_file_transfer();   
private:
    std::array<char, CHUNK_SIZE> buffer;
    const socket_ptr socket;
    std::ifstream file;
    time_point start_time;
    time_point end_time;
    size_t transferred = 0;

    void write_chunk();
    void on_write(const boost::system::error_code& err,
                  size_t);


};

class FileServer
{
public:
    FileServer(boost::asio::io_context& io,
               int port, 
               std::string filename);
private:
    int port;
    std::string filename;
    boost::asio::io_context& io;
    ip::tcp::acceptor acceptor;

    void accept_next();
    void handle_accept(const socket_ptr, const boost::system::error_code& err);
};
