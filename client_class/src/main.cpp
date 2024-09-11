#include <iostream>
#include "../include/client_class.hpp"
#include <boost/asio.hpp>
#include <string>
#include <filesystem>
#include <fstream>
#include <cstring>
#include "../include/crypt.hpp"
#define FILE_PATH "/etc/server_config.conf"

bool check_file() {
	return std::filesystem::exists(FILE_PATH);
}
 
bool check_args(int &argc) {
	return argc == 4;
}

void read_from_file(std::string &server_ip, std::string &server_passwd) {
	std::ifstream in(FILE_PATH);
	if (std::getline(in, server_ip)) {
		if (std::getline(in, server_passwd)) {
			in.close();
		}
	}
}

void return_original_home_var() {
	const char* sudo_user = getenv("SUDO_USER");
	std::string original_home = "/home/";
	original_home = original_home + sudo_user + '/';
	setenv("HOME", original_home.c_str(), 1);
}

int main (int argc, char* argv[]) {
	if(argc == 2) {
        if(strcmp(argv[1],"--help") == 0){
            std::cout << "Usage: "<< std:: endl;
            std::cout << "\tIf you are running the program for the first time, please use sudo" << std:: endl;
            std::cout << '\t' << "./client [options]"<< std:: endl;
            std::cout << "Options:" << std:: endl;
            std::cout << '\t' << "--config [server IP ] [server password]\tto set the server IP and password " << std:: endl;
            return 0;
        } 
    }
	try {
		std::string server_ip = "";
		std::string server_passwd = "";
		if(check_file()) {
			read_from_file(server_ip, server_passwd);
		} else {
			if(getuid() != 0) {
				std::cout << "Permission denied.Please run programm with sudo" << std::endl;
				return 1;
			}
			if (!check_args(argc)) {
				std::cout << "Please configure server config file with option --config <IP> <Password>" << std::endl;
				return 1;
			} else {
				if (std::strcmp(argv[1], "--config") == 0) {
					return_original_home_var();
					server_ip = argv[2];
					std::cout << server_ip << std::endl;
					std::string server_passwd1 = argv[3];
					if(!Client::is_valid_ip(server_ip)) {
						Client::get_server_ip(server_ip);
					} else {
						Client::check_server_ip(server_ip);
					}
					server_passwd = hmac_md5(server_passwd1);
					Client::write_to_file(server_ip);
				} else {
					std::cout << "Please configure server config file with option --config <IP> <Password>" << std::endl;
					return 1;
				}
			}
		}
		std::string username = "username";
		std::cout << "\033[93m";
		boost::asio::io_context ioContext;
		Client client1(ioContext, username, server_ip, server_passwd);
		Client client2(ioContext, username, server_ip, server_passwd);
		client1.connect_to_server();
		boost::asio::ip::tcp::acceptor tcp_acceptor(ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 5002));
		client1.accept_client_connections(tcp_acceptor, client2, ioContext);  
		boost::asio:: streambuf buffer_server;
		client1.receive_data_from_server(buffer_server, client2, ioContext, tcp_acceptor);
		ioContext.run();
	} catch(const std::exception& e) {
		std::cout << "Exception caught: " << e.what() << std::endl;
	}
	return 0;
}
