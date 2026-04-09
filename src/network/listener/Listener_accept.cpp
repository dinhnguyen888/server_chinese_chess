#include "network/Listener.h"
#include "network/session/WsSession.h"
#include "game/MatchLobby.h"
#include "services/AuthService.h"
#include "services/MatchHistoryService.h"
#include <iostream>

namespace beast = boost::beast;
using boost::asio::ip::tcp;

namespace {
void fail(beast::error_code ec, const char* what) {
    std::cerr << what << ": " << ec.message() << "\n";
}
}  // namespace

void Listener::run() {
    do_accept();
}

void Listener::do_accept() {
    auto self = shared_from_this();
    acceptor_.async_accept([self](beast::error_code ec, tcp::socket socket) {
        self->on_accept(ec, std::move(socket));
    });
}

void Listener::on_accept(beast::error_code ec, tcp::socket socket) {
    if (ec) {
        fail(ec, "accept");
    } else {
        std::make_shared<WsSession>(std::move(socket), *lobby_, *auth_service_, *match_history_service_)->run();
    }
    do_accept();
}
