#include "asio/io_context.hpp"
#include <asio.hpp>
#include <deque>
#include <memory>

#include <iostream>

std::string client_read_buffer;
std::deque<std::string> client_write_queue;

void handle_client_read(std::shared_ptr<asio::ip::tcp::socket> socket) {
	// begin client read
	asio::async_read_until(*socket, asio::dynamic_buffer(client_read_buffer), '\n',
		[socket](std::error_code ec, std::size_t length) {
			// if connection error returns, close connection
			if(ec) {
				socket->close();
				return;
			}

			// read portion of message 
			auto msg = client_read_buffer.substr(0, length);

			// erase read portion of message from read buffer
			client_read_buffer.erase(0, length);

			// display message to client's screen
			std::cout << msg << std::flush;

			// recursively handle reads
			handle_client_read(socket);
		});
}

void handle_client_write(std::shared_ptr<asio::ip::tcp::socket> socket) {
	// if write queue empty, do nothing
	if (client_write_queue.empty()) return;

	// begin write
	asio::async_write(*socket, asio::buffer(client_write_queue.front()),
		[socket](std::error_code ec, std::size_t) {
			// if error with connection, close connection
			if(ec) {
				socket->close();
				return;
			}
			
			// pop current message from write queue
			client_write_queue.pop_front();

			// if the write queue is not empty, recursively handle writes
			if (!client_write_queue.empty()) 
				handle_client_write(socket);
		});
}

inline void client_main(std::string_view host) {
	// var init
    asio::io_context ctx;
    asio::ip::tcp::resolver resolver(ctx);
    // facebook.com -> 157.240.23.45
    auto endpoints = resolver.resolve(host, "9999");

	// create socket from ctx var
    auto socket = std::make_shared<asio::ip::tcp::socket>(ctx);

	// create local endpoint by connecting socket to host address
    auto local_endpoint = asio::connect(*socket, endpoints);

	// begin handling reads from host
    handle_client_read(socket);

	// create a thread to run the context in order to allow program to multitask
    std::thread t([&ctx] { ctx.run(); });

	// loop to handle reads and writes
    while(true) {
        // Homework get some input from the user here!
        std::string message = "hi\n";
		// check if client wrote a message
        bool write_in_progress = !client_write_queue.empty();
		// push back current message to write queue
        client_write_queue.push_back(message);
		// if no write in progress, begin handling writes
        if(!write_in_progress)
            asio::post(ctx, [socket]() { handle_client_write(socket); });
    }
    
	// join thread to main program
    t.join();
}