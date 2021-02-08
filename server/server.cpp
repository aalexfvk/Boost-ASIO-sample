#include <iostream>
#include <fstream>
#include <memory>
#include <thread>
#include <utility>
#include <chrono>
#include <iomanip>
#include <boost/asio.hpp>

#include "server.h"

using namespace boost::asio;


Client::Client(const socket_ptr socket, std::string filename)
: socket(socket),
  file(filename, std::ifstream::in | std::ifstream::binary)
{
    if (!file)
        throw std::runtime_error("Bad file");
}

void Client::start_file_transfer() {
    start_time = std::chrono::system_clock::now();
    write_chunk();
}

void Client::write_chunk()
{
    file.read(buffer.data(), buffer.size());
    size_t chunk_size = buffer.size();
    if (file.eof())
        chunk_size = file.gcount();  // size of last read

    async_write(*socket,
            boost::asio::buffer(buffer, chunk_size),
            std::bind(&Client::on_write, shared_from_this(), std::placeholders::_1,
                      std::placeholders::_2));
}

void Client::on_write(const boost::system::error_code& err,
                      size_t bytes_transferred)
{
    if (err) {
        Log() << "Connection error: " <<  err.message();
        return;
    }

    transferred += bytes_transferred;
    if (!file.eof())
        write_chunk();
    else{
        end_time = std::chrono::system_clock::now();

        auto sec = std::chrono::duration_cast<
            std::chrono::seconds>(end_time - start_time);
        auto speed_mbs = (static_cast<double>(transferred)/(1024*1024)) / sec.count();


        Log() << "Transfer complete for " 
              << socket->remote_endpoint().address().to_string() << "\n"
              << transferred << " bytes sent in " << sec.count() << " secs"
              << std::setprecision(2) << std::fixed
              << " (" << speed_mbs << " Mb/s)";
        socket->close();
    }
}


FileServer::FileServer(boost::asio::io_context& io,
            int server_port, 
            std::string file_name)
  : io(io),
    port(server_port),
    filename(file_name),
    acceptor(io, ip::tcp::endpoint(ip::tcp::v4(), server_port))
{
    accept_next();
}

void FileServer::accept_next()
{
    socket_ptr new_client{new ip::tcp::socket{io}};
    acceptor.async_accept(*new_client, std::bind(&FileServer::handle_accept, this, new_client, std::placeholders::_1));
}

void FileServer::handle_accept(socket_ptr socket, const boost::system::error_code& err)
{
    if (err) {
	    Log() << "Error while accepting:" << err.message();
	    accept_next();
	    return;
	}

    Log() << "New client connected: " 
          << socket->remote_endpoint().address().to_string()
          << ". Start file transferring...";

    auto client = std::make_shared<Client>(socket, filename);
    client->start_file_transfer();
    accept_next();
}


int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: file_server <port> <file_name>\n";
        return 1;
    }

    int port = atoi(argv[1]);
    std::string file_name = argv[2];
    boost::asio::io_context io_context;
    FileServer file_server(io_context, port, file_name);

    int num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> pool;
    for (int ii = 0; ii < num_threads; ii++)
    {
        pool.push_back(std::thread([&]{
            try {
                io_context.run();
            }
            catch (std::exception& ex) {
                Log() << "Error in io_context.run() " << ex.what();
            }
        }));
    }

    for(auto& thread : pool)
        thread.join();

    return 0;
}