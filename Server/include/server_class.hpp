#pragma once
#include <iostream>
#include <memory>
#include <boost/asio.hpp>

class Server {
    public:
        Server(std::shared_ptr<boost::asio::ip::tcp::socket> socket, boost::asio::ip::tcp::acceptor &acceptor, boost::asio::io_context &ioContext, std::string &passwd);
        static bool check_file(std::string &file_path);
    private:
        std::string findKey(std::string &value);
        void accept_handler(const boost::system::error_code &err, std::shared_ptr<boost::asio::ip::tcp::socket> socket, boost::asio::ip::tcp::acceptor &acceptor, boost::asio::io_context &ioContext, std::string &passwd);
        void send_updates(const std::string &resp_code);
        void write_to_socket(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string &message);
        void disconnect_from_user(const std::string &current_user, const std::string &user_to_disconnect);
        void add_user(const std::string &username, const std::string &IP, std::shared_ptr<boost::asio::ip::tcp::socket> socket);
        void read_from_socket(std::shared_ptr<boost::asio::ip::tcp::socket> socket, std::shared_ptr<boost::asio::streambuf> buf, std::string &passwd);
        void read_handler(const boost::system::error_code &err, std::shared_ptr<boost::asio::ip::tcp::socket> socket, std::shared_ptr<boost::asio::streambuf> buf, size_t bytes_transferred, std::string &passwd);
        void do_accept(std::shared_ptr<boost::asio::ip::tcp::socket> socket, boost::asio::ip::tcp::acceptor &acceptor, boost::asio::io_context &ioContext, std::string &passwd);
        void create_user_list_file(std::string &file_path);
        void update_user_list(const std::string &user_name, const std::string &user_ip);
        std::map<std::string, std::string> user_list;
        std::map<std::string, std::string> active_users_list;
        std::map<std::string, std::string> busy_users_list;
        std::map<std::string, std::shared_ptr<boost::asio::ip::tcp::socket>> user_socket_list;
        std::string file_path;
};


