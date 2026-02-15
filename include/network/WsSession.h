#pragma once
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <nlohmann/json.hpp>
#include <memory>
#include <queue>
#include <string>

class MatchLobby;

class WsSession : public std::enable_shared_from_this<WsSession> {
public:
    std::string name_;

    WsSession(boost::asio::ip::tcp::socket&& socket, MatchLobby& lobby);

    void run();
    void send_json(const nlohmann::json& j);
    void attach_opponent(const std::shared_ptr<WsSession>& other);
    void clear_opponent();
    std::shared_ptr<WsSession> opponent_lock() const;

private:
    void on_accept(boost::beast::error_code ec);
    void do_read();
    void on_read(boost::beast::error_code ec, std::size_t bytes);
    void handle_message(const std::string& text);
    void do_write();
    void on_write(boost::beast::error_code ec, std::size_t bytes);
    void on_close();

    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws_;
    boost::beast::flat_buffer buffer_;
    MatchLobby& lobby_;
    std::weak_ptr<WsSession> opponent_;
    std::queue<std::string> write_queue_;
    bool closed_{false};
};
