#ifndef __CLIENT_CLASS__
#define __CLIENT_CLASS__
#include <string>
#include <boost/asio.hpp>
#include <stdint.h>
#include <map>
class Client{
    private:
        std::unique_ptr<boost::asio::ip::tcp::socket> _client_socket;
        std::unique_ptr<boost::asio::ip::tcp::socket> _server_socket; 
        boost::asio::ip::tcp::endpoint server_endpoint;
        std::mutex mtx;
        std::string user_name;
        std::string encrypted_server_passwd;
        bool is_accepted;
        bool set_user_name(const std::string _user_name);
        std::string get_user_name();
        void get_another_client_username(boost::asio::io_context &ioContext, Client &another_client, boost::asio::ip::tcp::acceptor &tcp_acceptor);
        void connect_to_another_client_request(Client&  other_client, boost::asio::io_context &ioContext, boost::asio::ip::tcp::acceptor &tcp_acceptor);
        void accept_handler(const boost::system::error_code &err, Client &another_client, boost::asio::io_context &ioContext, boost::asio::ip::tcp::acceptor &tcp_acceptor);
        void client_receive_handler(const boost::system::error_code &err, std::shared_ptr<boost::asio::streambuf> buf, Client &another_client, boost::asio::io_context &ioContext, boost::asio::ip::tcp::acceptor &tcp_acceptor);
        void server_receive_handler(const boost::system::error_code &err, boost:: asio :: streambuf &buf , Client& another_client, boost::asio::io_context &ioContext, boost::asio::ip::tcp::acceptor &tcp_acceptor);
        bool check_user_name(std::string &username);
        void getUserName(std::string &username);
        void send_message(const std::string &message);
        void start_input_thread(Client &another_client, std::string client_message, boost::asio::ip::tcp::acceptor &tcp_acceptor, boost::asio::io_context &ioContext);
        std::thread input_thread;
        std::thread input_thread2;
        bool check_file_path(const std:: string& file_path);
        std::string expandTilde(const std::string& path);
        unsigned short terminal_width;
        std::map<std::string, std::string> emoji_list;
        std::map<std::string, std::string> users_list;
        std::map<std::string, std::string> user_status_list;
        void update_user_list(std::string &message);
        void get_client_response(Client &another_client, boost::asio::io_context &ioContext, std::shared_ptr<boost::asio::streambuf> buf, boost::asio::ip::tcp::acceptor &tcp_acceptor);
        void set_server_passwd(std::string &server_passwd);
        void send_exit_request();
        void waitForExitInput(std::string &answer, boost::asio::io_context &ioContext);
    public:
        std::string ip_address;
        Client(boost::asio::io_context& io_context , const std::string &user_name , const std::string &server_ip, const std::string &server_passwd);
        ~Client();
        void send_data_to_server();
        void receive_data_from_server(boost:: asio :: streambuf& buf,Client& another_client, boost::asio::io_context &ioContext, boost::asio::ip::tcp::acceptor &tcp_acceptor);
        void accept_client_connections(boost::asio::ip::tcp::acceptor &tcp_acceptor, Client &another_client, boost::asio::io_context &ioContext);
        void connect_to_server();
        void receive_data_from_another_client(std::shared_ptr<boost::asio::streambuf> buf, Client &another_client, boost::asio::io_context &ioContext, boost::asio::ip::tcp::acceptor &tcp_acceptor);
        static void write_to_file(std::string &text);
        static bool is_valid_ip(const std::string &ip);
        static void get_server_ip(std::string &server_ip);
        static void check_server_ip(std::string &server_ip);
};

#endif