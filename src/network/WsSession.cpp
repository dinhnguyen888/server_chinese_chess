#include "network/WsSession.h"
#include "game/MatchLobby.h"
#include <iostream>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
using boost::asio::ip::tcp;
using json = nlohmann::json;

static void fail(beast::error_code ec, char const* what) {
    if (ec != websocket::error::closed)
        std::cerr << what << ": " << ec.message() << "\n";
}

WsSession::WsSession(tcp::socket&& socket, MatchLobby& lobby)
    : ws_(std::move(socket)), lobby_(lobby) {}

void WsSession::run() {
    ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));
    ws_.set_option(websocket::stream_base::decorator([](websocket::response_type& res) {
        res.set(beast::http::field::server, "chinese-chess-ws");
    }));
    auto self = shared_from_this();
    ws_.async_accept([self](beast::error_code ec) { self->on_accept(ec); });
}

void WsSession::send_json(const json& j) {
    std::string text = j.dump();
    bool idle = write_queue_.empty();
    write_queue_.push(std::move(text));
    if (idle)
        do_write();
}

void WsSession::attach_opponent(const std::shared_ptr<WsSession>& other) {
    opponent_ = other;
}

void WsSession::clear_opponent() {
    opponent_.reset();
}

std::shared_ptr<WsSession> WsSession::opponent_lock() const {
    return opponent_.lock();
}

void WsSession::on_accept(beast::error_code ec) {
    if (ec)
        return fail(ec, "accept");
    do_read();
}

void WsSession::do_read() {
    auto self = shared_from_this();
    ws_.async_read(buffer_, [self](beast::error_code ec, std::size_t n) { self->on_read(ec, n); });
}

void WsSession::on_read(beast::error_code ec, std::size_t /*bytes*/) {
    if (ec) {
        if (ec != websocket::error::closed)
            fail(ec, "read");
        on_close();
        return;
    }

    std::string text = beast::buffers_to_string(buffer_.data());
    buffer_.consume(buffer_.size());
    handle_message(text);
    do_read();
}

void WsSession::handle_message(const std::string& text) {
    json msg;
    try {
        msg = json::parse(text);
    } catch (const json::parse_error&) {
        send_json(json{{"type", "error"}, {"message", "invalid_json"}});
        return;
    }

    const std::string type = msg.value("type", "");
    if (type == "find_match") {
        name_ = msg.value("name", "Player");
        lobby_.try_pair(shared_from_this());
        return;
    }
    if (type == "move") {
        if (auto peer = opponent_lock())
            peer->send_json(msg);
        return;
    }
    if (type == "ping") {
        send_json(json{{"type", "pong"}});
        return;
    }

    send_json(json{{"type", "error"}, {"message", "unknown_type"}});
}

void WsSession::do_write() {
    if (write_queue_.empty())
        return;
    auto self = shared_from_this();
    ws_.async_write(boost::asio::buffer(write_queue_.front()),
        [self](beast::error_code ec, std::size_t n) { self->on_write(ec, n); });
}

void WsSession::on_write(beast::error_code ec, std::size_t /*bytes*/) {
    if (ec) {
        fail(ec, "write");
        on_close();
        return;
    }
    write_queue_.pop();
    if (!write_queue_.empty())
        do_write();
}

void WsSession::on_close() {
    if (closed_)
        return;
    closed_ = true;
    lobby_.cancel_waiting(shared_from_this());
    if (auto peer = opponent_lock()) {
        peer->clear_opponent();
        peer->send_json(json{{"type", "opponent_left"}});
    }
    clear_opponent();
}
