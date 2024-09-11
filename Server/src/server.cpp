#include "../include/server_class.hpp"
#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include <thread>
#include <filesystem>
#include <fstream>
#include "../include/crypt.hpp"
#define FILE_PATH "/etc/server_passwd.conf"

bool check_args(int &argc) {
	return argc == 3;
}

void write_to_file(std::string &text) {
	std::ofstream out(FILE_PATH);
	out << text << std::endl;	
	out.close();
}

void read_from_file(std::string &server_passwd) {
	std::ifstream in(FILE_PATH);
	if (std::getline(in, server_passwd)) {
        in.close();
	}
}

int main (int argc, char* argv[]) {
    if(argc == 2) {
        if(strcmp(argv[1],"--help") == 0){
            std:: cout << "Usage: "<< std:: endl;
            std:: cout << "\tIf you are running the program for the first time, please use sudo" << std:: endl;
            std:: cout << '\t' << "./server [options]"<< std:: endl;
            std:: cout << "Options:" << std:: endl;
            std:: cout << '\t' << "--passwd [password]\tto set the server password  " << std:: endl;
            return 0;
        } 
    }
    std::string server_passwd;
    std::string file_path = "/etc/server_passwd.conf";
    if(Server::check_file(file_path)) {
        read_from_file(server_passwd);    
    } else {
        if(getuid() != 0) {
            std::cout << "Permission denied.Please run programm with sudo" << std::endl;
            return 1;
        }
        if(!check_args(argc)) {
            std::cout << "Please set server password with option --passwd <Password>" << std::endl;
            return 1;
        } else {
            if(std::strcmp(argv[1], "--passwd") == 0) {
                std::string encrypted_passwd = hmac_md5(std::string(argv[2]));
                write_to_file(encrypted_passwd);
                read_from_file(server_passwd);
            } else {
                std::cout << "Please set server password with option --passwd <Password>" << std::endl;
                return 1;
            }
        }
    }
    system("bash -c 'echo -e \"\\033]11;#000000\\007\"'");
    std::cout << "\033[1;32m";
    try {
        boost::asio::io_context ioContext;
        std::shared_ptr<boost::asio::ip::tcp::socket> tcp_socket = std::make_shared<boost::asio::ip::tcp::socket>(ioContext);
        boost::asio::ip::tcp::acceptor tcp_acceptor(ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 5001));
        Server(tcp_socket, tcp_acceptor, ioContext, server_passwd);
        ioContext.run();
    } catch(const std::exception &e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
}

