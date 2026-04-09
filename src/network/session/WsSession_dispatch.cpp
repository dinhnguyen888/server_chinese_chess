#include "network/session/WsSession.h"

using json = nlohmann::json;

void WsSession::attach_opponent(const std::shared_ptr<WsSession>& other) {
    opponent_ = other;
}

void WsSession::clear_opponent() {
    opponent_.reset();
}

std::shared_ptr<WsSession> WsSession::opponent_lock() const {
    return opponent_.lock();
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
    if (msg.contains("name")) {
        name_ = msg.value("name", name_.empty() ? "Player" : name_);
    }

    if (handle_auth_message(msg, type)) {
        return;
    }
    if (handle_lobby_message(msg, type)) {
        return;
    }
    if (handle_gameplay_message(msg, type)) {
        return;
    }
    if (handle_history_message(msg, type)) {
        return;
    }

    send_json(json{{"type", "error"}, {"message", "unknown_type"}});
}
