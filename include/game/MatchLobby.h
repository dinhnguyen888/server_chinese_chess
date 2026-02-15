#pragma once
#include <memory>
#include <mutex>

class WsSession;

class MatchLobby {
public:
    void try_pair(std::shared_ptr<WsSession> session);
    void cancel_waiting(const std::shared_ptr<WsSession>& session);

private:
    std::mutex mutex_;
    std::shared_ptr<WsSession> waiting_;
};
