#include "game/MatchLobby.h"
#include "network/session/WsSession.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void MatchLobby::chat_in_room(std::shared_ptr<WsSession> sender, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (sender->current_room_id_.empty()) {
        sender->send_json(json{{"type", "error"}, {"message", "Bạn chưa vào phòng"}});
        return;
    }

    auto it = rooms_.find(sender->current_room_id_);
    if (it == rooms_.end()) {
        return;
    }

    Room& room = it->second;
    json chat_message = {{"type", "chat"}, {"sender", sender->name_}, {"message", message}};
    if (room.host) {
        room.host->send_json(chat_message);
    }
    if (room.guest) {
        room.guest->send_json(chat_message);
    }
}
