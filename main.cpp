#include "network/Listener.h"
#include "game/MatchLobby.h"
#include "db/Database.h"
#include <boost/asio.hpp>
#include <iostream>

int main() {
    try {
        // ── 1. Tự động tạo database nếu chưa có ──────────────────────
        db::auto_create("localhost", "5432", "postgres", "postgres", "chess_db");

        // ── 2. Kết nối đến database ───────────────────────────────────
        db::connect("host=localhost port=5432 user=postgres password=postgres dbname=chess_db");

        // ── 3. Khởi tạo schema (tạo bảng nếu chưa có) ────────────────
        db::init_schema();

        // ── 4. Khởi động WebSocket server ─────────────────────────────
        auto const address = boost::asio::ip::make_address("0.0.0.0");
        unsigned short port = 8080;

        boost::asio::io_context ioc{1};
        auto lobby = std::make_shared<MatchLobby>();
        std::make_shared<Listener>(ioc, boost::asio::ip::tcp::endpoint{address, port}, lobby)->run();

        std::cout << "WebSocket Server starting up on ws://0.0.0.0:" << port << "\n";
        ioc.run();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
