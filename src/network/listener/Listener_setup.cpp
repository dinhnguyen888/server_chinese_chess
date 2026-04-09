#include "network/Listener.h"
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

Listener::Listener(boost::asio::io_context& ioc,
                   tcp::endpoint endpoint,
                   std::shared_ptr<MatchLobby> lobby,
                   std::shared_ptr<IAuthService> auth_service,
                   std::shared_ptr<IMatchHistoryService> match_history_service)
    : ioc_(ioc),
      acceptor_(ioc),
      lobby_(std::move(lobby)),
      auth_service_(std::move(auth_service)),
      match_history_service_(std::move(match_history_service)) {
    beast::error_code ec;
    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
        fail(ec, "open");
        return;
    }
    acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
    if (ec) {
        fail(ec, "set_option");
        return;
    }
    acceptor_.bind(endpoint, ec);
    if (ec) {
        fail(ec, "bind");
        return;
    }
    acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
    if (ec) {
        fail(ec, "listen");
        return;
    }
}
