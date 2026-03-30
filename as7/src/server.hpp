
#include "asio/buffer.hpp"
#include "asio/io_context.hpp"
#include "asio/read_until.hpp"
#include "asio/write.hpp"
#include <asio.hpp>
#include <deque>
#include <memory>
#include <system_error>
#include <iostream>
#include <set>

// Struct to hold client information
struct ClientConnection {
    // Stores a pointer to the socket containing the client connection
    std::shared_ptr<asio::ip::tcp::socket> connection;
    // Buffer for incoming strings 
    std::string read_buffer;
    // Queue for string to be written to the server
    std::deque<std::string> write_queue;
};

// List of all clients
std::set<std::shared_ptr<ClientConnection>> clients;

// Queue containing all previous strings in chat log
std::deque<std::string> message_history;

void handle_client_write(std::shared_ptr<ClientConnection> client) {

    // If nothing to write, don't do anything
    if(client->write_queue.empty()) return;

    // Asynchronously begin writing bytes to the stream
    asio::async_write(*client->connection, asio::buffer(client->write_queue.front()),
    [client](std::error_code error, size_t bytes_written){
        // If error writing, (i.e lost connection,) disconnect client from list
        if(error) {
            clients.erase(client);
            return;
        }

        // Once write is complete, pop write request from the queue
        client->write_queue.pop_front();

        // Recursively call function to continue handling new write requests
        handle_client_write(client);
    });
}


void broadcast_message(const std::string& msg) {
    // Add new message onto the history queue
    message_history.push_back(msg);

    // Loop through all clients
    for(auto& client: clients) {
        // Check if a client has written a message to the board
        bool write_in_progress = !client->write_queue.empty();
        // Add current message to client's write queue
        client->write_queue.push_back(msg);
        // If there is no current write being made, begin handling new writes
        if(!write_in_progress)
            handle_client_write(client);
    }
}

void handle_client_read(std::shared_ptr<ClientConnection> client) {
    // Begin reading from buffer
    asio::async_read_until(*client->connection, asio::dynamic_buffer(client->read_buffer), '\n',
    [client](std::error_code error, size_t bytes_read){
        // If error writing, (i.e lost connection,) disconnect client from list
        if(error) {
            clients.erase(client);
            return;
        }

        //create new message object from the read buffer
        std::string msg = client->read_buffer.substr(0, bytes_read);
        // since new message was made, delete currently read portion of the message
        client->read_buffer.erase(0, bytes_read);
        
        // Sends the message back out to every client
        broadcast_message(msg);

        // continue to recursively handle reads
        handle_client_read(client);
    });
}

void accept_connections(asio::ip::tcp::acceptor& acceptor) {
    // begin to accept incoming connection
    acceptor.async_accept([&acceptor](std::error_code error, asio::ip::tcp::socket socket){
        accept_connections(acceptor);

        // if error connecting, do nothing
        if(error) return;

        // display new connection
        std::cout << "New client connected from: " << socket.remote_endpoint() << std::endl;
        // create new client object to store new client info into
        auto client = std::make_shared<ClientConnection>();
        // store client's current socket
        client->connection = std::make_shared<asio::ip::tcp::socket>(std::move(socket));

        // add new client to list of clients
        clients.insert(client);

        // write chat log to client's screen
        for(auto& msg: message_history)
            client->write_queue.push_back(msg);

        // if the client had a non-empty write queue, begin to process their message
        if(!client->write_queue.empty())
            handle_client_write(client);

        // being to handle reads for the client
        handle_client_read(client);
    });
}

inline void server_main() {
    // creat ctx object
    asio::io_context ctx;
    // acceptio init
    asio::ip::tcp::acceptor acceptor(ctx, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), 9999));

    // begin to accept connections using the previously initialised acceptor
    accept_connections(acceptor);
    // run context
    ctx.run();
}
