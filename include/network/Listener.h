#pragma once
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <memory>

class MatchLobby;
class IAuthService;
class IMatchHistoryService;

class Listener : public std::enable_shared_from_this<Listener> {
public:
    Listener(boost::asio::io_context& ioc,
             boost::asio::ip::tcp::endpoint endpoint,
             std::shared_ptr<MatchLobby> lobby,
             std::shared_ptr<IAuthService> auth_service,
             std::shared_ptr<IMatchHistoryService> match_history_service);
    void run();

private:
    void do_accept();
    void on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket);

    boost::asio::io_context& ioc_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::shared_ptr<MatchLobby> lobby_;
    std::shared_ptr<IAuthService> auth_service_;
    std::shared_ptr<IMatchHistoryService> match_history_service_;
};
