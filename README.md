# Terminal Chat

## Objective
The ability to transfer data between two devices on the same local network.
## Project start time and deadline
- **Project start time:** July 29, 2024
- **Deadline:** September 11, 2024
## Team Members
- **Team Member 1 Gevorg Manukyan:
- **Team Member 2 Tigran Harutyunyan:
##Prerequisites
    - C++ Compiler (e.g., g++, clang++)
    - Boost Library
    - OpenSSL Library
    - CMake (version 3.10 or higher)
##Installation
    - To download the Boost library
          sudo apt-get install libboost-all-dev
    - To download the OpenSSL library
          sudo apt-get install openssl libssl-dev
    - To download CMake
          sudo apt-get install cmake
##Building the Project
    - Clone the repository
          git clone https://github.com/manukyangev/Terminal-Chat-Project.git
          cd Terminal-Chat-Project
          mkdir build
          cd build
          cmake ..
          make
## Usage
# Server 
The server accepts client requests. When a client connects to the server, it sends the server's encrypted password, and the server checks if the password is correct. If it is, the client can log in; if not, it prompts the client to enter the password again. After that, the server receives the client's IP address and name, which it stores in a file. The server also keeps track of the status of these clients—whether they are active or not, and if active, whether they are available or not. Then, the server sends an update to all active clients. This update contains a list of clients, their names, and IP addresses. This cycle continues asynchronously, with the server creating new sockets, accepting client connections, checking the password, saving their data, and sending updates to active clients whenever there are changes.

To run the server for the first time, you need to use sudo and provide the server password using the --passwd [password] parameter. The password will be stored in an encrypted form in the server_passwd.conf file located in the /etc directory. Sudo is used to store the file in the /etc directory. After the file is created, you can run the program without sudo. You can also use the --help option for assistance.
# Client
The client connects to the server and sends the server's encrypted password, which is retrieved from the server_config.conf file located in the /etc directory. If the password is correct, the client is prompted to log in; if not, the password must be entered again. Once the client logs in and enters their password, a request is sent to the server with the client's name. After validation, if the login is successful, the client receives a list of other active clients and can choose whom to connect with. If the selected client is available, the connection can be made. After the client enters the name of the client they wish to connect with, a request is sent to that client, who can either accept or decline the connection request. If the client accepts the connection, a chat is opened, and clients can send messages, files, and emojis. To send a file, use the format file:~[path]. For emojis, the following codes are used: /laughingFace, /thumbsUp, /huggingFace, /foldedHands, /thinkingFace, /cryingFace.
To disconnect a user these two commands are used
    - /disconnect - Сloses the chat with the user and sends a request to the server that he is already free and an activator for other connections.
    - /exit - This disconnects the user permanently and sends a request to the server where the user's status becomes inactive.
    
To run the client for the first time, you need to use sudo and provide the server's IP address and password using the --config [server IP] [server password] option. These will be stored in the server_config.conf file located in the /etc directory. Sudo is used to store the file in the /etc directory. After the file is created, you can run the program without sudo. You can use the --help option for assistance.
## Technology Stack
- Linux (distribution Ubuntu OS )
- Computer Network
- C++ (Boost.Asio)
### Tools
Slack,Skype,Gmail.


