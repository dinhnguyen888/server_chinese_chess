#include "game/MatchLobby.h"
#include "network/session/WsSession.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace {
json room_to_json(const Room& room) {
    return json{{"id", room.id},
                {"name", room.name},
                {"host", room.host ? room.host->name_ : "Unknown"},
                {"guest", room.guest ? room.guest->name_ : ""},
                {"isPlaying", room.is_playing},
                {"autoStart", room.auto_start}};
}
}  // namespace

void MatchLobby::get_room_list(std::shared_ptr<WsSession> session) {
    std::lock_guard<std::mutex> lock(mutex_);
    json room_list = json::array();
    for (const auto& pair : rooms_) {
        room_list.push_back(room_to_json(pair.second));
    }
    session->send_json(json{{"type", "room_list"}, {"rooms", room_list}});
}

void MatchLobby::search_room(std::shared_ptr<WsSession> session, const std::string& query) {
    std::lock_guard<std::mutex> lock(mutex_);
    json room_list = json::array();
    for (const auto& pair : rooms_) {
        const Room& room = pair.second;
        if (room.name.find(query) != std::string::npos || room.id == query) {
            room_list.push_back(room_to_json(room));
        }
    }
    session->send_json(json{{"type", "room_list"}, {"rooms", room_list}});
}
