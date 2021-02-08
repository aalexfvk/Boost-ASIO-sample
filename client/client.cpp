#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <boost/asio.hpp>

using namespace boost::asio;
using double_seconds = std::chrono::duration<double> ;
using time_point = std::chrono::system_clock::time_point;
const size_t CHUNK_SIZE = 8*1024;

void print_stat(time_point start, time_point end, size_t total)
{
    double sec = std::chrono::duration_cast<double_seconds>(
        end - start).count();    
    auto speed_mbs = (static_cast<double>(total)/(1024*1024)) / sec;    
    std::cout << "Transfer complete.\n" 
        << std::setprecision(2) << std::fixed
        << total << " received in " << sec << " secs"        
        << " (" << speed_mbs << " Mb/s)";
}

size_t download_file(std::ofstream& file, ip::tcp::socket& socket)
{
    size_t total_size = 0;    
    boost::system::error_code ec;
    size_t bytes;
    std::array<char, CHUNK_SIZE> buf;
    
    do {        
        bytes = boost::asio::read(socket, boost::asio::buffer(buf, CHUNK_SIZE), ec);   
        total_size += bytes;        
        if (bytes)
            file.write(buf.data(), bytes);
        if (!file.good())
            throw std::runtime_error("Unable write to file");    
    }while(bytes && !ec);
    if (ec != boost::asio::error::eof)
        throw std::runtime_error("Read error: " + ec.message());
    return total_size;
}

int main(int argc, char* argv[])
{
    try{
        if (argc != 4) {
            std::cerr << "Usage: client <host> <port> <output file name>" << std::endl;
            return 1;
        }

        std::string filename = argv[3];
        boost::asio::io_context io_context;        
        ip::tcp::socket socket(io_context);
        ip::tcp::resolver resolver(io_context);
        std::ofstream file(argv[3], std::ios_base::binary);

        if (!file.good())
            throw std::runtime_error("Bad file"); 

        std::cout << "Connecting..." << std::endl;
        boost::asio::connect(socket, resolver.resolve(argv[1], argv[2]));
        std::cout << "Connected.\n" << "Transferring..." << std::endl;   

        auto start_time = std::chrono::system_clock::now();
        size_t total_size = download_file(file, socket);   
        auto end_time = std::chrono::system_clock::now();

        print_stat(start_time, end_time, total_size); 

    }catch (std::exception& e){
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}