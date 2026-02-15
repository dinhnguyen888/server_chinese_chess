#include "game/MatchLobby.h"
#include "network/WsSession.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void MatchLobby::try_pair(std::shared_ptr<WsSession> session) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!waiting_) {
        waiting_ = std::move(session);
        waiting_->send_json(json{{"type", "queue"}, {"message", "waiting_for_opponent"}});
        return;
    }

    auto a = waiting_;
    waiting_.reset();
    auto b = std::move(session);

    a->attach_opponent(b);
    b->attach_opponent(a);

    constexpr int kRedFirst = 1;
    a->send_json(json{{"type", "matched"}, {"color", "r"}, {"orderSide", kRedFirst}, {"opponentName", b->name_}});
    b->send_json(json{{"type", "matched"}, {"color", "b"}, {"orderSide", kRedFirst}, {"opponentName", a->name_}});
}

void MatchLobby::cancel_waiting(const std::shared_ptr<WsSession>& session) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (waiting_ == session)
        waiting_.reset();
}
