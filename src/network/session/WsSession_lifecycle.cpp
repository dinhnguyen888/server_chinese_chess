#include "network/session/WsSession.h"
#include "game/MatchLobby.h"
#include "services/AuthService.h"
#include "services/MatchHistoryService.h"
#include <iostream>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
using boost::asio::ip::tcp;
using json = nlohmann::json;

namespace {
void fail(beast::error_code ec, const char* what) {
    if (ec != websocket::error::closed) {
        std::cerr << what << ": " << ec.message() << "\n";
    }
}
}  // namespace

WsSession::WsSession(tcp::socket&& socket,
                     MatchLobby& lobby,
                     IAuthService& auth_service,
                     IMatchHistoryService& match_history_service)
    : ws_(std::move(socket)),
      lobby_(lobby),
      auth_service_(auth_service),
      match_history_service_(match_history_service) {}

void WsSession::run() {
    ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));
    ws_.set_option(websocket::stream_base::decorator([](websocket::response_type& res) {
        res.set(beast::http::field::server, "chinese-chess-ws");
    }));
    auto self = shared_from_this();
    ws_.async_accept([self](beast::error_code ec) { self->on_accept(ec); });
}

void WsSession::on_accept(beast::error_code ec) {
    if (ec) {
        return fail(ec, "accept");
    }
    do_read();
}

void WsSession::on_close() {
    if (closed_) {
        return;
    }

    closed_ = true;
    lobby_.cancel_waiting(shared_from_this());

    if (auto peer = opponent_lock()) {
        peer->clear_opponent();
        peer->send_json(json{{"type", "opponent_left"}});
    }
    clear_opponent();
}
