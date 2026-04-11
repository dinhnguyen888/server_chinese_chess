#include "network/ws_listener.h"
#include "network/http_server.h"
#include "service/match_lobby_service.h"
#include "db/database.h"
#include <boost/asio.hpp>
#include <iostream>

int main() {
    try {
        db::auto_create("localhost", "5432", "postgres", "postgres", "chess_db");
        db::connect("host=localhost port=5432 user=postgres password=postgres dbname=chess_db");
        db::init_schema();

        auto const address = boost::asio::ip::make_address("0.0.0.0");
        unsigned short ws_port = 8080;
        unsigned short http_port = 8081;

        boost::asio::io_context ioc{1};
        auto lobby = std::make_shared<MatchLobbyService>();
        std::make_shared<Listener>(ioc, boost::asio::ip::tcp::endpoint{address, ws_port}, lobby)->run();
        std::make_shared<HttpListener>(ioc, boost::asio::ip::tcp::endpoint{address, http_port})->run();

        std::cout << "WebSocket server: ws://0.0.0.0:" << ws_port << "\n";
        std::cout << "HTTP API:         http://0.0.0.0:" << http_port << " (/register, /login)\n";
        ioc.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
