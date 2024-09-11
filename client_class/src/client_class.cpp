#include <iostream>
#include <boost/asio.hpp>
#include <string>
#include <cstring>
#include "../include/client_class.hpp"
#include <stdint.h>
#include <memory>
#include <unistd.h>
#include <thread>
#include <ctime>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sys/ioctl.h>
#include "../include/crypt.hpp"
#define FILE_PATH "/etc/server_config.conf"
#define DOW_FILE_PATH "~/Download"

Client:: Client(boost:: asio:: io_context& io_context , const std::string &user_name , const std::string &server_ip, const std::string &server_passwd) {
    this->_client_socket = std::make_unique<boost::asio::ip::tcp::socket>(io_context);
    this->_server_socket = std::make_unique<boost::asio::ip::tcp::socket>(io_context);
    this->user_name = std:: move(user_name);
    this->server_endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(server_ip), 5001);
    this->encrypted_server_passwd = server_passwd;
    this->terminal_width = 80;
    this->is_accepted = false;
    this->ip_address = "";
    emoji_list = {
        {"/laughingFace", "😂"},
        {"/thumbsUp", "👍"},
        {"/cryingFace", "😢"},
        {"/thinkingFace", "🤔"},
        {"/huggingFace", "🤗"},
        {"/foldedHands", "🙏"},
    };
}

Client::~Client() {
    if(input_thread.joinable()) {
        input_thread.join();
    }
    if(input_thread2.joinable()) {
        input_thread2.join();
    }
}

bool Client::set_user_name(std::string username){
    if(username.empty()){
        std::cout << "Error: Invalid value " << std::endl;
        return 1; 
    }
    else if(!(username.find('>') == std::string::npos)) {
        std::cout << "Please input username without '>'" << std::endl;
    }
    this->user_name = username;
    return 0;  
} 
std::string Client::get_user_name(){
    return this->user_name;
}

bool Client::check_user_name(std::string &username) {
	return username.find('>') == std::string::npos;
}
void Client::getUserName(std::string &username) {
	std::cout << "User name: ";
	std::getline(std::cin, username);
	if(!check_user_name(username)) {
		std::cout << "Please input username without '>'" << std::endl;
		getUserName(username);
	}
}

void Client::set_server_passwd(std::string &server_passwd) {
    this->encrypted_server_passwd = server_passwd;
}

bool Client::is_valid_ip(const std::string &ip) {
	boost::system::error_code err;
	boost::asio::ip::address::from_string(ip, err);
	return !err;
}

void Client::get_server_ip(std::string &server_ip) {
	std::cout << "Please input valid IP address: ";
	std::getline(std::cin, server_ip);
	if(!is_valid_ip(server_ip)) {
		get_server_ip(server_ip);
	}
	check_server_ip(server_ip);
}

void Client::check_server_ip(std::string &server_ip) {
    boost::system::error_code err;
	boost::asio::io_context ioContext;
	boost::asio::ip::tcp::endpoint tcp_endpoint(boost::asio::ip::address::from_string(server_ip), 5001);
	boost::asio::ip::tcp::socket tcp_socket(ioContext);
	tcp_socket.connect(tcp_endpoint, err);
	if(err) {
        tcp_socket.close();
        get_server_ip(server_ip);
        ioContext.stop();
    }
    ioContext.run();
}

void Client::write_to_file(std::string &text) {
	std::ofstream out(FILE_PATH, std::ios::app);
	out << text << std::endl;	
	out.close();
}

void Client::start_input_thread(Client &another_client, std::string client_message, boost::asio::ip::tcp::acceptor &tcp_acceptor, boost::asio::io_context &ioContext) {
    try {
        while (std::getline(std::cin, client_message)) {
            if(client_message.empty()) {
                break;
            }
            if (client_message == "/disconnect") {
                client_message = "DISCN\t";
                boost::system::error_code error;
                write(*_client_socket, boost::asio::buffer(client_message), error);
                if (error) {
                    std::cout << "Error: " << error.message() << std::endl;
                }
                static boost::asio::streambuf buffer;
                is_accepted = false;
                receive_data_from_server(buffer, another_client, ioContext, tcp_acceptor);
                system("bash -c 'echo -e \"\\033]11;#300A24\\007\"'");
                break; 
            }
            for (const auto& pair : emoji_list) {
                const std::string& emoji_code = pair.first;
                const std::string& emoji = pair.second;
                int pos = 0;
                while ((pos = client_message.find(emoji_code, pos)) != std::string::npos) {
                    client_message.replace(pos, emoji_code.length(), emoji);
                    pos += emoji.length();
                }
            }
            send_message(client_message);
        }
    } catch(const std::exception& e) {
        std::cout << "Exception caught: " << e.what() << std::endl;
    }
}

bool Client::check_file_path(const std:: string& file_path ){
    if(file_path.find('~') !=  std::string::npos){
        std::string expand_file  = expandTilde(file_path);
        bool is_checked = std::filesystem::exists(expand_file);
        if(is_checked){
            return true ; 
        }
        std::cout << "File path is incorrect"<< std::endl;
        return 1; 
    }      
    std::cout << "This is incorrect input" << std::endl;
    return false;    
}
std::string Client::expandTilde(const std::string& path) {
    if(!path.empty() && path[0] == '~') {
        const char* homeDir = getenv("HOME");
        if(homeDir) {
            return std::string(homeDir) + path.substr(1);
        }
    }
    return path;
}
void Client::send_message(const std::string& message) {
    std::time_t current_time = std::time(nullptr);    
    char* time_str = std::asctime(std::localtime(&current_time));      
    time_str[19] = '\0';         
    std::string time_only = std::string(time_str + 11);         
    if(!message.empty()) {
        if(message.substr(0, 5) == "file:") {
            std::string file_path = message.substr(5,message.size());
            if(check_file_path(file_path)){
                std::string expand_path = expandTilde(file_path);
                std::filesystem::path path(expand_path);
                std::string filename = path.filename().string();
                std::string message = "_FILE" + filename + '>';
                std::ifstream inputfile (path , std::ios::binary );
                if(!inputfile.is_open()){
                    std::cout << "Error opening file" << std::endl;
                    return;
                }
                std::vector<char> buffer(1024 + message.size());
                message.copy(buffer.data(), message.size());
                boost::system::error_code err;
                while(inputfile.read(buffer.data() + message.size(), buffer.size() - message.size()) || inputfile.gcount() > 0) {
                    if (inputfile.gcount() < buffer.size() - message.size()) {
                        buffer.resize(message.size() + inputfile.gcount());
                    }
                    buffer.push_back('\t');
                    boost::asio::write(*_client_socket, boost::asio::buffer(buffer.data(), buffer.size()));
                } 
                std::cout << "Your file has been transferred successfully" << std::endl;
                return;
            }
        }
        std::string message_to_send = "_MESG" + time_only + " " + user_name + '>' + message + '\t';
        boost::system::error_code err;
        write(*_client_socket, boost::asio::buffer(message_to_send), err);
        if(err) {
            std::cout << "Error: " << err.message() << std::endl;
            return;
        }
        std::cout << user_name + " " + time_only + "> " << message << std::endl;
    }
}

void Client::connect_to_server() {
    boost::system::error_code err;
    _server_socket->connect(server_endpoint, err);
    if(err) {
        std::cout << "Error: " << err.message() << std::endl;
        return;
    }
    std::string message = "PASWD" + encrypted_server_passwd + '\n';
    write(*_server_socket, boost::asio::buffer(message), err);
    if(err) {
        std::cout << "Error: " << err.message() << std::endl;
    }
}

void  Client::connect_to_another_client_request(Client& another_client, boost::asio::io_context &ioContext, boost::asio::ip::tcp::acceptor &tcp_acceptor){
    std::string user_name = another_client.get_user_name();
    std::string user_ip = users_list[user_name];
    boost::system::error_code err;
    boost::asio::ip::tcp::endpoint tcp_endpoint(boost::asio::ip::address::from_string(user_ip), 5002);
    _client_socket->connect(tcp_endpoint, err);
    if(err) {
        std::cout << "Error: " << err.message() << std::endl;
    }
    std::string message = "CNNCT" + this->user_name + '>' + another_client.get_user_name() + '\t';
    write(*_client_socket, boost::asio::buffer(message), err);
    if(err) {
        std::cout << "Error: " << err.message() << std::endl;
    }
    std::shared_ptr<boost::asio::streambuf> buffer_client = std::make_shared<boost::asio::streambuf>();
	receive_data_from_another_client(buffer_client, another_client, ioContext, tcp_acceptor);
}

void Client::send_exit_request() {
    boost::system::error_code err;
    std::string request = "EXITT" + user_name + ">" + '\n';
    write(*_server_socket, boost::asio::buffer(request), err);
    if (err) {
        std::cout << "Error: " << err.message() << std::endl;
    }
}

void Client::get_another_client_username(boost::asio::io_context &ioContext, Client &another_client, boost::asio::ip::tcp::acceptor &tcp_acceptor) {
    try {
        std::string client_to_connect = "";
        boost::system::error_code err;
        while (true) {
            {
                std::lock_guard<std::mutex> lock(mtx);
                std::cout << "Please enter which client you want to connect to: ";
                client_to_connect.clear();
                std::getline(std::cin, client_to_connect);
            }
            if (client_to_connect.empty()) {
                return;
            }

            if (client_to_connect == "/exit") {
                send_exit_request();
                return;
            }

            if (client_to_connect == user_name) {
                std::cout << "Error: You can't connect to yourself." << std::endl;
                continue;
            }

            if (!users_list.count(client_to_connect)) {
                std::cout << "Error: The user does not exist." << std::endl;
                continue;
            }

            if (user_status_list[client_to_connect] == "Busy") {
                std::cout << "Error: The user is busy." << std::endl;
                continue;
            }
            another_client.set_user_name(client_to_connect);
            connect_to_another_client_request(another_client, ioContext, tcp_acceptor);
            std::cout << "Please wait while the user responds to your connection request..." << std::endl;
            return;
        }
    } catch(const std::exception& e) {
        std::cout << "Exception caught: " << e.what() << std::endl;
    }
}

void Client::update_user_list(std::string &message) {
    size_t pos = 0;
    std::string user_data = "";
    size_t greater_pos = 0;
    std::string user_name = "";
    std::string user_ip = "";
    std::string user_status = "";
    users_list.erase(users_list.begin(), users_list.end());
    while(!message.empty()) {
        pos = message.find('|');
        user_data = message.substr(0, pos);
        if (pos != std::string::npos) {
            message.erase(0, pos + 1);
        } else {
            message.clear();
        }
        size_t space_pos = user_data.find(' ');
        user_name = user_data.substr(0, space_pos);
        user_data.erase(0, space_pos + 1);

        size_t greater_pos = user_data.find('>');
        user_status = user_data.substr(0, greater_pos);
        user_data.erase(0, greater_pos + 1);
        user_ip = user_data;
        if(user_name != this->user_name) {
            users_list[user_name] = user_ip;
            user_status_list[user_name] = user_status;
        }
    }
}

void Client::server_receive_handler(const boost::system::error_code &err, boost::asio::streambuf &buf , Client& another_client, boost::asio::io_context &ioContext, boost::asio::ip::tcp::acceptor &tcp_acceptor) {
    if(!err) {
        std::istream in(&buf);
        std::string message = "";
        std::getline(in ,message) ; 
        buf.consume(buf.size());
        std::string resp_code = message.substr(0, 5);
        std::string request_to_server = "";
        if(resp_code == "PASAC") {
            write_to_file(encrypted_server_passwd);
            std::cout << message.substr(5, message.size()) << std::endl;
            std::string username = "";
            getUserName(username);
            set_user_name(username);
            send_data_to_server();
        }
        if(resp_code == "PASDN") {
            std::cout << message.substr(5, message.size()) << std::endl;
            _server_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
            _server_socket->close();
            std::string server_passwd = "";
            std::cout << "Server password: ";
            std::getline(std::cin, server_passwd);
            std::string encrypted_server_password = hmac_md5(server_passwd);
            set_server_passwd(encrypted_server_password);
            connect_to_server();
        }
        if(resp_code == "UNERR") {
            std::cout << message.substr(5, message.size()) << std::endl;
            std::cout << "User name: ";
            std::string username = "";
            std::getline(std::cin, username);
            set_user_name(username);
            request_to_server = "LOGIN" + this->user_name + ">" + '\n';
            boost::asio::write(*_server_socket, boost::asio::buffer(request_to_server));
        }
        if(resp_code == "_UPDT") {
            system("clear");
            message.erase(0, 5);
            update_user_list(message);
            for(auto &pair : users_list) {
                std::cout << pair.first << " ";
                std::cout << user_status_list[pair.first] << std::endl;
            }
            if(!users_list.empty()) {
                if(input_thread.joinable()) {
                    std::cout << "Please press Enter to continue" << std::endl;
                    input_thread.join();
                } 
                input_thread = std::thread(&Client::get_another_client_username, this, std::ref(ioContext), std::ref(another_client), std::ref(tcp_acceptor));
            } else {
                std::cout << "No active users. Please wait or you can exit the program.\nIf you want to exit, enter 'y'/'yes'; if not, enter 'n'/'no'." << std::endl;
                std::string answer;
                waitForExitInput(answer, ioContext);
                if(answer == "y" || answer == "yes") {
                    ioContext.stop();
                }
                if(answer == "n" || answer == "no") {
                    std::string request = "_UPDT\n";
                    write(*_server_socket, boost::asio::buffer(request));
                }
            }
        }
        if(!is_accepted) {
            async_read_until(*_server_socket, buf, '\n', [&buf, &another_client, this, &ioContext, &tcp_acceptor](const boost::system::error_code &err, std::size_t bytes_transferred) {
                server_receive_handler(err, buf, another_client, ioContext, tcp_acceptor);
            });
        }
    } else {
        if(input_thread.joinable()) {
            input_thread.join();
        }
        ioContext.stop();
    }
}

void Client::receive_data_from_server(boost::asio::streambuf& buf, Client& another_client, boost::asio::io_context &ioContext, boost::asio::ip::tcp::acceptor &tcp_acceptor) {
    async_read_until(*_server_socket, buf, '\n', [&buf, &another_client, this, &ioContext, &tcp_acceptor](const boost::system::error_code &err, std::size_t bytes_transferred) {
        server_receive_handler(err, buf, another_client, ioContext, tcp_acceptor);
    });
}

void Client::send_data_to_server() { 
    boost::system::error_code err;
    std::string message = "LOGIN" + this->user_name + ">" + ip_address + '\n' ;
    boost::asio::write(*_server_socket , boost::asio::buffer(message), err);
    if(err){    
        std::cout << "Error during write socket " << err.message() << std::endl;
        return;
    }
}

void Client::waitForExitInput(std::string &answer, boost::asio::io_context &ioContext) {
    if(input_thread.joinable()) {
        std::cout << "Please press Enter to continue" << std::endl;
        input_thread.join();
    }
    std::cout << "Answer: ";
    std::getline(std::cin, answer);
    if(answer == "y" || answer == "yes") {
        send_exit_request();
        return;
    }
    else if(answer == "n" || answer == "no") {
        return;
    }
    else {
        std::cout << "Please input y/yes or n/no" << std::endl;
        waitForExitInput(answer, ioContext);
    }
}

void Client::get_client_response(Client &another_client, boost::asio::io_context &ioContext, std::shared_ptr<boost::asio::streambuf> buf, boost::asio::ip::tcp::acceptor &tcp_acceptor) {
    boost::system::error_code err;
    std::string response = "";
    std::cout << "Answer: ";
    std::getline(std::cin, response);
    if(response == "y" || response == "yes") {
        is_accepted = true;
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        terminal_width = w.ws_col;
        std::cout << "Connection accepted" << std::endl;
        response = "_ACPT\t";
        write(*_client_socket, boost::asio::buffer(response), err);
        if(err) {
            std::cout << "Error: " << err.message() << std::endl;
        }
        std::cout << user_name << std::endl;
        system("clear");
        system("bash -c 'echo -e \"\\033]11;#000000\\007\"'");
        for(int i = 0; i < 38; ++i) {
            std::cout << " ";
        }
        std::cout << "\033[1m";
        std::cout << "Chat" << std::endl;
        std::cout << "\033[0m";
        std::cout << "\033[93m";
        if(input_thread2.joinable()) {
            input_thread2.join();
        }
        std::string client_message = "";
        input_thread2 = std::thread(&Client::start_input_thread, this, std::ref(another_client), client_message, std::ref(tcp_acceptor), std::ref(ioContext));
        return;
    }
    else if (response == "n" || response == "no") {
        std::cout << "Connection denied" << std::endl;
        std::string message = "_DENY\t";
        write(*_client_socket, boost::asio::buffer(message));
        return;
    }
    else {
        std::cout << "Please input y/yes or n/no" << std::endl;
        get_client_response(another_client, ioContext, buf, tcp_acceptor);
    }
}

void Client::client_receive_handler(const boost::system::error_code &err, std::shared_ptr<boost::asio::streambuf> buf, Client &another_client, boost::asio::io_context &ioContext, boost::asio::ip::tcp::acceptor &tcp_acceptor) {
    if(!err) {
        boost::system::error_code error;
        std::istream in(buf.get());
        std::string message = "";  
        std::getline(in, message);
        std::string command = message.substr(0, 5);
        if(command == "CNNCT") {
            message.erase(message.find_last_not_of(" \n\r\t") + 1);
            int index_of_symb = message.find('>');
            std::string username = message.substr(5, index_of_symb - 5);
            std::cout << std::endl;
            std::cout << username << " want to connect to you.Do you agree? y/yes or n/no" << std::endl;
            std::cout << "Please press Enter to continue" << std::endl;
            input_thread.join();
            get_client_response(another_client, ioContext, buf, tcp_acceptor);
        }
        if(command  == "_ACPT") {
            is_accepted = true;
            struct winsize w;
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
            terminal_width = w.ws_col;
            std::string request = "_ACPT" + user_name + '>' + another_client.get_user_name() + '\n';
            boost::system::error_code error;
            write(*_server_socket, boost::asio::buffer(request), error);
            if(error) {
                std::cout << "Error: " << error.message() << std::endl;
            }
            std::cout << "Connection accepted" << std::endl;
            system("clear");
            system("bash -c 'echo -e \"\\033]11;#000000\\007\"'");
            for(int i = 0; i < 38; ++i) {
                std::cout << " ";
            }
            std::cout << "\033[1m";
            std::cout << "Chat" << std::endl;
            std::cout << "\033[0m";
            std::cout << "\033[93m";
            if(input_thread2.joinable()) {
                input_thread2.join();
            }
            std::string client_message = "";
            input_thread2 = std::thread(&Client::start_input_thread, this, std::ref(another_client), client_message, std::ref(tcp_acceptor), std::ref(ioContext));
        }
        if(command == "_DENY") {
            std::cout << "Connection denied" << std::endl;
            input_thread.join();
            _client_socket->cancel();
            _client_socket->close();
            return;
        }
        if(command == "DISCN") {
            is_accepted = false;
            std::cout << "User has disconnected.\nPlease press Enter to continue" << std::endl;
            input_thread2.join();
            _client_socket->cancel();
            _client_socket->close();
            static boost::asio::streambuf buffer;
            receive_data_from_server(buffer, another_client, ioContext, tcp_acceptor);
            std::string request = "DISCN" + user_name + '>' + another_client.get_user_name() + '\n';
            write(*_server_socket, boost::asio::buffer(request), error);
            system("bash -c 'echo -e \"\\033]11;#300A24\\007\"'");
            return;
        }
        if(command == "_MESG") {
            message.erase(message.find_last_not_of(" \n\r\t") + 1);
            int index_of_symb = message.find('>');
            std::string username = message.substr(5, index_of_symb - 5);
            std::string message_to_print = message.substr(index_of_symb + 1, message.size()) + " <" + username;
            std::size_t prompt_length = message_to_print.length();
            for(int i = 0; i < (terminal_width - prompt_length); ++i) {
                std::cout << " ";
            }
            std::cout << "\033[97m" << message_to_print << "\033[93m" << std::endl;
        }    
        if(command == "_FILE"){
            int index_of_symb = message.find('>');
            std::cout << "The file has been received; please check the Downloads directory." << std::endl;
            std::vector<char> buffer(buf->size());
            buf->sgetn(buffer.data(), buffer.size());
            std::string file_name = message.substr(5, index_of_symb - 5);
            std::string file_path = expandTilde(std::string("~/Downloads") + "/" + file_name);
            std::ofstream input_file(file_path, std::ios::app);
            input_file << message.substr(index_of_symb + 1, message.size()) + '\n';
            for(int i = 0; i < buffer.size(); ++i) {
                input_file << buffer[i];
            }
            input_file.close();
            buf->consume(buf->size());
        }
        boost::asio::async_read_until(*_client_socket , *buf , '\t' , [buf, this, &another_client, &ioContext, &tcp_acceptor] (const boost::system::error_code &err , size_t bytes_transfered) {
            client_receive_handler(err, buf, another_client, ioContext, tcp_acceptor);
        });
    } else {
        if(_client_socket->is_open()) {
            _client_socket->cancel();
            _client_socket->close();
            accept_client_connections(tcp_acceptor, another_client, ioContext);
            std::string request = "_UPDT\n";
            usleep(500000);
            write(*_server_socket, boost::asio::buffer(request));
        }
    }
}

void Client::receive_data_from_another_client(std::shared_ptr<boost::asio::streambuf> buf, Client &another_client, boost::asio::io_context &ioContext, boost::asio::ip::tcp::acceptor &tcp_acceptor) {
    boost::asio::async_read_until(*_client_socket , *buf , '\t' ,  [buf, this, &another_client, &ioContext, &tcp_acceptor] (const boost::system::error_code &err , size_t bytes_transfered){
        client_receive_handler(err, buf, another_client, ioContext, tcp_acceptor);
    });
}

void Client::accept_handler(const boost::system::error_code &err, Client &another_client, boost::asio::io_context &ioContext, boost::asio::ip::tcp::acceptor &tcp_acceptor) {
    if(!err) {
        boost::asio::ip::tcp::endpoint remote_endpoint = _client_socket->remote_endpoint();
        std::string IP = remote_endpoint.address().to_string();
        for(const auto &pair : users_list) {
            if(pair.second == IP) {
                another_client.set_user_name(pair.first);
            }
        }
        std::shared_ptr<boost::asio::streambuf> buffer_client = std::make_shared<boost::asio::streambuf>();
	    receive_data_from_another_client(buffer_client, another_client, ioContext, tcp_acceptor);
    } else {
        std::cout << "Error code: " << err.value() << " - " << err.message() << std::endl;
    }
}

void Client::accept_client_connections(boost::asio::ip::tcp::acceptor &tcp_acceptor, Client &another_client, boost::asio::io_context &ioContext) {
    if(!_client_socket->is_open()) {
        tcp_acceptor.async_accept(*_client_socket, [this, &another_client, &ioContext, &tcp_acceptor](const boost::system::error_code &err) {
            accept_handler(err, another_client, ioContext, tcp_acceptor);
        });
    }
}
