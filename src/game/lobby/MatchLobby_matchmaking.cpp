#include "game/MatchLobby.h"
#include "network/session/WsSession.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void MatchLobby::try_pair(std::shared_ptr<WsSession> session) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!waiting_) {
        waiting_ = std::move(session);
        waiting_->send_json(json{{"type", "queue"}, {"message", "waiting_for_opponent"}});
        return;
    }

    auto first = waiting_;
    waiting_.reset();
    auto second = std::move(session);

    first->attach_opponent(second);
    second->attach_opponent(first);

    constexpr int kRedFirst = 1;
    first->send_json(
        json{{"type", "matched"}, {"color", "r"}, {"orderSide", kRedFirst}, {"opponentName", second->name_}});
    second->send_json(
        json{{"type", "matched"}, {"color", "b"}, {"orderSide", kRedFirst}, {"opponentName", first->name_}});
}

void MatchLobby::cancel_waiting(const std::shared_ptr<WsSession>& session) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (waiting_ == session) {
        waiting_.reset();
    }
    leave_room_internal(session);
}
