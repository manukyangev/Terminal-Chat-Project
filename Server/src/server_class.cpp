#include "../include/server_class.hpp"
#include <iostream>
#include <boost/asio.hpp>
#include <memory>
#include <map>
#include <fstream>
#include <filesystem>

void Server::write_to_socket(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string &message) {
    std::shared_ptr<boost::asio::streambuf> buf = std::make_shared<boost::asio::streambuf>();
    std::ostream out(buf.get());
    out << message << '\n';
    async_write(*socket, *buf, [socket, buf](const boost::system::error_code &err, std::size_t) {
        if(err) {
            std::cout << "Error: " << err.message() << std::endl;
        } else {
            std::cout << "Sent" << std::endl;
        }
    });
}

bool Server::check_file(std::string &file_path) {
    return std::filesystem::exists(file_path);
}

void Server::create_user_list_file(std::string &file_path) {
    std::ofstream user_list(file_path);
    user_list.close();
    system("sudo chmod 777 user_list.txt");
}

void Server::send_updates(const std::string &resp_code) {
    std::string message_for_current_socket;    
    for (const auto &active_user : active_users_list) {
        if(active_user.second == "Active") {
            message_for_current_socket = message_for_current_socket + active_user.first + " " + busy_users_list[active_user.first] + '>' + user_list[active_user.first] + '|';
        }
    }
    size_t pos = message_for_current_socket.find(resp_code);
    if (pos != std::string::npos) {
        message_for_current_socket.erase(pos, resp_code.length());
    }
    message_for_current_socket = resp_code + message_for_current_socket;
    for (auto &user_socket : user_socket_list) {
        if(busy_users_list[user_socket.first] != "Busy") {
            write(*user_socket.second, boost::asio::buffer(message_for_current_socket + '\n'));
        }
    } 
}

void Server::disconnect_from_user(const std::string &current_user, const std::string &user_to_disconnect) {
    busy_users_list[current_user] = "Not busy";
    busy_users_list[user_to_disconnect] = "Not busy";
}

void Server::update_user_list(const std::string &user_name, const std::string &user_ip) {
    std::ofstream user_list_file(file_path, std::ios::app);
    user_list_file << user_name << '>' << user_ip << std::endl;
    user_list_file.close();
}

void Server::add_user(const std::string &username, const std::string &IP, std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
    std::string message;
    std::string resp_code;
    bool user_exists = false;
    std::ifstream users_file(file_path);
    std::string line;
    while(std::getline(users_file, line)) {
        line.erase(line.find_last_not_of(" \n\r\t") + 1);
        int index_of_symb = line.find('>');
        std::string userName = line.substr(0, index_of_symb);
        std::string userIP = line.substr(index_of_symb + 1, line.size());
        if(username == userName) {
            user_exists = true;
            if(IP != userIP) {
                message = "UNERRThe username is already taken\n";
                write_to_socket(socket, message);
                break;
            } else {
                user_list[username] = IP;
                active_users_list[username] = "Active";
                busy_users_list[username] = "Not busy";
                user_socket_list[username] = socket;
                resp_code = "_UPDT";
                send_updates(resp_code);
                break;
            }
        }
    }
    users_file.close();
    if(!user_exists) {
        update_user_list(username, IP);
        user_list[username] = IP;
        active_users_list[username] = "Active";
        busy_users_list[username] = "Not busy";
        user_socket_list[username] = socket;
        resp_code = "_UPDT";
        send_updates(resp_code);
    }
}

void Server::read_handler(const boost::system::error_code &err, std::shared_ptr<boost::asio::ip::tcp::socket> socket, std::shared_ptr<boost::asio::streambuf> buf, size_t bytes_transferred, std::string &passwd) {
    if(!err) {
        std::istream in(buf.get());
        std::string data;
        std::getline(in, data);
        buf->consume(bytes_transferred);
        data.erase(data.find_last_not_of(" \n\r\t") + 1);
        std::string user_to_connect;
        std::string command = data.substr(0, 5);
        if(command == "_UPDT") {
            send_updates(data.substr(0, 5));
        }
        if(command == "PASWD") {
            std::string password = data.substr(5, data.size());
            if(password == passwd) {
                std::string message = "PASACThe password is correct. The connection is accepted.\n";
                write_to_socket(socket, message);
            } else {
                std::string message = "PASDNIncorrect password. The connection is rejected.\n";
                write_to_socket(socket, message);
                return;
            }
        }
        if(command == "LOGIN") {
            int index_of_symb = data.find('>');
            std::string username = data.substr(5, index_of_symb - 5);
            boost::asio::ip::tcp::endpoint remote_endpoint = socket->remote_endpoint();
            std::string IP = remote_endpoint.address().to_string();
            add_user(username, IP, socket);
        }
        if(command == "EXITT") {
            int index_of_symb = data.find('>');
            std::string username = data.substr(5, index_of_symb - 5);
            boost::system::error_code error;
            active_users_list[username] = "Inactive";
            user_socket_list.erase(username);
            std::cout << "Connection closed with IP " << user_list[username] << std::endl;
            std::string resp_code = "_UPDT";
            send_updates(resp_code);
            write(*socket, boost::asio::buffer(data + '\n'), error);
            if(error) {
                std::cout << "Error: " << err.message() << std::endl;
            }
            if(socket->is_open()) {
                socket->close();
            }
            return;
        }
        if(command == "DISCN") {
            int index_of_symb = data.find('>');
            std::string username = data.substr(5, index_of_symb - 5);
            std::string user_to_disconnect = data.substr(index_of_symb + 1, data.size());
            disconnect_from_user(username, user_to_disconnect);
        }
        if(command == "_ACPT") {
            int index_of_symb = data.find('>');
            std::string current_user = data.substr(5, index_of_symb - 5);
            std::string user_to_connect = data.substr(index_of_symb + 1, data.size());
            busy_users_list[current_user] = "Busy";
            busy_users_list[user_to_connect] = "Busy";
            std::string resp_code = "_UPDT";
            send_updates(resp_code);
        }
        async_read_until(*socket, *buf, '\n', [socket, buf, this, &passwd](const boost::system::error_code &err, size_t bytes_transferred) {
            read_handler(err, socket, buf, bytes_transferred, passwd);
        });
    } else {
        std::cout << "Error: " << err.message() << std::endl;
    }
}

void Server::read_from_socket(std::shared_ptr<boost::asio::ip::tcp::socket> socket, std::shared_ptr<boost::asio::streambuf> buf, std::string &passwd) {
    async_read_until(*socket, *buf, '\n', [socket, buf, this, &passwd](const boost::system::error_code &err, size_t bytes_transferred) {
        read_handler(err, socket, buf, bytes_transferred, passwd);
    });
}

void Server::accept_handler(const boost::system::error_code &err, std::shared_ptr<boost::asio::ip::tcp::socket> socket, boost::asio::ip::tcp::acceptor &acceptor, boost::asio::io_context &ioContext, std::string &passwd) {
    if(!err) {
        std::shared_ptr<boost::asio::streambuf> buf = std::make_shared<boost::asio::streambuf>();
        read_from_socket(socket, buf, passwd);
    } else {
        std::cout << "Error: " << err.message() << std::endl;
    }
    std::shared_ptr<boost::asio::ip::tcp::socket> tcp_socket = std::make_shared<boost::asio::ip::tcp::socket>(ioContext);
    acceptor.async_accept(*tcp_socket, [tcp_socket, &acceptor, &ioContext, this, &passwd](const boost::system::error_code &err) {
        accept_handler(err, tcp_socket, acceptor, ioContext, passwd);
    });
}

void Server::do_accept(std::shared_ptr<boost::asio::ip::tcp::socket> socket, boost::asio::ip::tcp::acceptor &acceptor, boost::asio::io_context &ioContext, std::string &passwd) {
    acceptor.async_accept(*socket, [socket, &acceptor, &ioContext, this, &passwd](const boost::system::error_code &err) {
        accept_handler(err, socket, acceptor, ioContext, passwd);
    });
}

Server::Server(std::shared_ptr<boost::asio::ip::tcp::socket> socket, boost::asio::ip::tcp::acceptor &acceptor, boost::asio::io_context &ioContext, std::string &passwd) {
    file_path = "user_list.txt";
    if(!check_file(file_path)) {
        create_user_list_file(file_path);
    }
    do_accept(socket, acceptor, ioContext, passwd);
    std::cout << "Server is listening on port 5001" << std::endl;
}
