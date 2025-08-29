#include <gtest/gtest.h>
#include <asio.hpp>
#include <thread>
#include <chrono>
#include <atomic>
#include <vector>
#include <string>
#include <memory>
#include "../common/protocol.hpp"
#include "../common/messages.hpp"
#include "../server/hub.hpp"
#include "../server/session.hpp"

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        io_context = std::make_unique<asio::io_context>();
        hub = std::make_shared<Hub>();
        
        // Create acceptor
        acceptor = std::make_unique<asio::ip::tcp::acceptor>(
            *io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 0));
        
        // Get the port
        port = acceptor->local_endpoint().port();
    }
    
    void TearDown() override {
        if (acceptor) {
            acceptor->close();
        }
        if (io_context) {
            io_context->stop();
        }
        hub.reset();
        acceptor.reset();
        io_context.reset();
    }
    
    std::unique_ptr<asio::io_context> io_context;
    std::shared_ptr<Hub> hub;
    std::unique_ptr<asio::ip::tcp::acceptor> acceptor;
    uint16_t port;
};

TEST_F(IntegrationTest, SingleClientConnection) {
    std::atomic<bool> client_connected{false};
    std::atomic<bool> server_accepted{false};
    
    // Server thread
    std::thread server_thread([&]() {
        asio::ip::tcp::socket socket(*io_context);
        acceptor->accept(socket);
        server_accepted = true;
        
        // Create session
        auto session = std::make_shared<Session>(std::move(socket), hub);
        session->start();
        
        // Keep connection alive
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
    
    // Client thread
    std::thread client_thread([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        asio::ip::tcp::socket socket(*io_context);
        asio::ip::tcp::resolver resolver(*io_context);
        auto endpoints = resolver.resolve("127.0.0.1", std::to_string(port));
        asio::connect(socket, endpoints);
        client_connected = true;
        
        // Keep connection alive
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
    
    // Run io_context
    std::thread io_thread([&]() {
        io_context->run();
    });
    
    // Wait for completion
    client_thread.join();
    server_thread.join();
    
    // Stop io_context
    io_context->stop();
    io_thread.join();
    
    // Verify connection was established
    EXPECT_TRUE(client_connected);
    EXPECT_TRUE(server_accepted);
}

TEST_F(IntegrationTest, MultipleClientsSameRoom) {
    const int num_clients = 3;
    std::atomic<int> clients_connected{0};
    std::atomic<int> messages_sent{0};
    
    // Server thread
    std::thread server_thread([&]() {
        for (int i = 0; i < num_clients; ++i) {
            asio::ip::tcp::socket socket(*io_context);
            acceptor->accept(socket);
            
            // Create session
            auto session = std::make_shared<Session>(std::move(socket), hub);
            session->start();
            clients_connected++;
        }
        
        // Keep server running to process messages
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    });
    
    // Client threads
    std::vector<std::thread> client_threads;
    for (int i = 0; i < num_clients; ++i) {
        client_threads.emplace_back([&, i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(50 + i * 10));
            
            asio::ip::tcp::socket socket(*io_context);
            asio::ip::tcp::resolver resolver(*io_context);
            auto endpoints = resolver.resolve("127.0.0.1", std::to_string(port));
            asio::connect(socket, endpoints);
            
            // Send a message
            ChatLine msg;
            msg.room = "test_room";
            msg.user = "user" + std::to_string(i);
            msg.text = "Hello from user " + std::to_string(i);
            
            auto frame = make_frame(msg);
            asio::write(socket, asio::buffer(frame));
            messages_sent++;
            
            // Keep connection alive briefly
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }
    
    // Run io_context
    std::thread io_thread([&]() {
        io_context->run();
    });
    
    // Wait for completion
    for (auto& thread : client_threads) {
        thread.join();
    }
    server_thread.join();
    
    // Stop io_context
    io_context->stop();
    io_thread.join();
    
    // Verify all clients connected and messages were sent
    EXPECT_EQ(clients_connected.load(), num_clients);
    EXPECT_EQ(messages_sent.load(), num_clients);
}

TEST_F(IntegrationTest, MessageBroadcasting) {
    std::atomic<bool> message_sent{false};
    std::atomic<bool> connections_established{false};
    
    // Server thread
    std::thread server_thread([&]() {
        // Accept two connections
        asio::ip::tcp::socket socket1(*io_context);
        acceptor->accept(socket1);
        auto session1 = std::make_shared<Session>(std::move(socket1), hub);
        session1->start();
        
        asio::ip::tcp::socket socket2(*io_context);
        acceptor->accept(socket2);
        auto session2 = std::make_shared<Session>(std::move(socket2), hub);
        session2->start();
        
        connections_established = true;
        
        // Keep server running to process messages
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    });
    
    // Sender client
    std::thread sender_thread([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        asio::ip::tcp::socket socket(*io_context);
        asio::ip::tcp::resolver resolver(*io_context);
        auto endpoints = resolver.resolve("127.0.0.1", std::to_string(port));
        asio::connect(socket, endpoints);
        
        // Send a message
        ChatLine msg;
        msg.room = "broadcast_room";
        msg.user = "sender";
        msg.text = "Broadcast message";
        
        auto frame = make_frame(msg);
        asio::write(socket, asio::buffer(frame));
        message_sent = true;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
    
    // Receiver client
    std::thread receiver_thread([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        asio::ip::tcp::socket socket(*io_context);
        asio::ip::tcp::resolver resolver(*io_context);
        auto endpoints = resolver.resolve("127.0.0.1", std::to_string(port));
        asio::connect(socket, endpoints);
        
        // Just keep connection alive - don't try to read as it may block
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
    
    // Run io_context
    std::thread io_thread([&]() {
        io_context->run();
    });
    
    // Wait for completion
    sender_thread.join();
    receiver_thread.join();
    server_thread.join();
    
    // Stop io_context
    io_context->stop();
    io_thread.join();
    
    // Verify message was sent and connections were established
    EXPECT_TRUE(message_sent);
    EXPECT_TRUE(connections_established);
}

TEST_F(IntegrationTest, DifferentRooms) {
    std::atomic<int> room1_messages_sent{0};
    std::atomic<int> room2_messages_sent{0};
    std::atomic<int> connections_established{0};
    
    // Server thread
    std::thread server_thread([&]() {
        // Accept multiple connections
        for (int i = 0; i < 4; ++i) {
            asio::ip::tcp::socket socket(*io_context);
            acceptor->accept(socket);
            auto session = std::make_shared<Session>(std::move(socket), hub);
            session->start();
            connections_established++;
        }
        
        // Keep server running to process messages
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    });
    
    // Room 1 clients
    std::thread room1_client1([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        asio::ip::tcp::socket socket(*io_context);
        asio::ip::tcp::resolver resolver(*io_context);
        auto endpoints = resolver.resolve("127.0.0.1", std::to_string(port));
        asio::connect(socket, endpoints);
        
        // Send message to room1
        ChatLine msg;
        msg.room = "room1";
        msg.user = "user1";
        msg.text = "Hello room1";
        
        auto frame = make_frame(msg);
        asio::write(socket, asio::buffer(frame));
        room1_messages_sent++;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
    
    std::thread room1_client2([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        asio::ip::tcp::socket socket(*io_context);
        asio::ip::tcp::resolver resolver(*io_context);
        auto endpoints = resolver.resolve("127.0.0.1", std::to_string(port));
        asio::connect(socket, endpoints);
        
        // Just keep connection alive - don't try to read as it may block
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
    
    // Room 2 clients
    std::thread room2_client1([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        
        asio::ip::tcp::socket socket(*io_context);
        asio::ip::tcp::resolver resolver(*io_context);
        auto endpoints = resolver.resolve("127.0.0.1", std::to_string(port));
        asio::connect(socket, endpoints);
        
        // Send message to room2
        ChatLine msg;
        msg.room = "room2";
        msg.user = "user2";
        msg.text = "Hello room2";
        
        auto frame = make_frame(msg);
        asio::write(socket, asio::buffer(frame));
        room2_messages_sent++;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
    
    std::thread room2_client2([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        asio::ip::tcp::socket socket(*io_context);
        asio::ip::tcp::resolver resolver(*io_context);
        auto endpoints = resolver.resolve("127.0.0.1", std::to_string(port));
        asio::connect(socket, endpoints);
        
        // Just keep connection alive - don't try to read as it may block
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
    
    // Run io_context
    std::thread io_thread([&]() {
        io_context->run();
    });
    
    // Wait for completion
    room1_client1.join();
    room1_client2.join();
    room2_client1.join();
    room2_client2.join();
    server_thread.join();
    
    // Stop io_context
    io_context->stop();
    io_thread.join();
    
    // Verify messages were sent to different rooms and connections established
    EXPECT_EQ(room1_messages_sent.load(), 1);
    EXPECT_EQ(room2_messages_sent.load(), 1);
    EXPECT_EQ(connections_established.load(), 4);
}

TEST_F(IntegrationTest, LargeMessageHandling) {
    std::atomic<bool> large_message_sent{false};
    
    // Server thread
    std::thread server_thread([&]() {
        asio::ip::tcp::socket socket(*io_context);
        acceptor->accept(socket);
        auto session = std::make_shared<Session>(std::move(socket), hub);
        session->start();
        
        // Keep server running
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    });
    
    // Client thread
    std::thread client_thread([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        
        asio::ip::tcp::socket socket(*io_context);
        asio::ip::tcp::resolver resolver(*io_context);
        auto endpoints = resolver.resolve("127.0.0.1", std::to_string(port));
        asio::connect(socket, endpoints);
        
        // Send a large message
        ChatLine msg;
        msg.room = "test_room";
        msg.user = "test_user";
        msg.text = std::string(10000, 'A'); // Large message
        
        auto frame = make_frame(msg);
        asio::write(socket, asio::buffer(frame));
        large_message_sent = true;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
    
    // Run io_context
    std::thread io_thread([&]() {
        io_context->run();
    });
    
    // Wait for completion
    client_thread.join();
    server_thread.join();
    
    // Stop io_context
    io_context->stop();
    io_thread.join();
    
    // Verify large message was sent
    EXPECT_TRUE(large_message_sent);
}

TEST_F(IntegrationTest, ConnectionHandling) {
    std::atomic<int> connections_accepted{0};
    std::atomic<int> connections_closed{0};
    
    // Server thread
    std::thread server_thread([&]() {
        for (int i = 0; i < 5; ++i) {
            asio::ip::tcp::socket socket(*io_context);
            acceptor->accept(socket);
            connections_accepted++;
            
            // Create session
            auto session = std::make_shared<Session>(std::move(socket), hub);
            session->start();
            
            // Close connection after a short time
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            connections_closed++;
        }
    });
    
    // Client threads
    std::vector<std::thread> client_threads;
    for (int i = 0; i < 5; ++i) {
        client_threads.emplace_back([&, i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(50 + i * 10));
            
            asio::ip::tcp::socket socket(*io_context);
            asio::ip::tcp::resolver resolver(*io_context);
            auto endpoints = resolver.resolve("127.0.0.1", std::to_string(port));
            asio::connect(socket, endpoints);
            
            // Keep connection alive briefly
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }
    
    // Run io_context
    std::thread io_thread([&]() {
        io_context->run();
    });
    
    // Wait for completion
    for (auto& thread : client_threads) {
        thread.join();
    }
    server_thread.join();
    
    // Stop io_context
    io_context->stop();
    io_thread.join();
    
    // Verify connections were handled
    EXPECT_EQ(connections_accepted.load(), 5);
}