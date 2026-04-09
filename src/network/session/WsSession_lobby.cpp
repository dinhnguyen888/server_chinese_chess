#include "network/session/WsSession.h"
#include "game/MatchLobby.h"

using json = nlohmann::json;

bool WsSession::handle_lobby_message(const json& msg, const std::string& type) {
    if (type == "find_match") {
        lobby_.try_pair(shared_from_this());
        return true;
    }
    if (type == "create_room") {
        bool auto_start = msg.value("autoStart", false);
        lobby_.create_room(shared_from_this(), msg.value("roomName", "My Room"), auto_start);
        return true;
    }
    if (type == "join_room") {
        lobby_.join_room(shared_from_this(), msg.value("roomId", ""));
        return true;
    }
    if (type == "start_game") {
        lobby_.start_game(shared_from_this(), msg.value("roomId", ""));
        return true;
    }
    if (type == "list_rooms") {
        lobby_.get_room_list(shared_from_this());
        return true;
    }
    if (type == "search_room") {
        lobby_.search_room(shared_from_this(), msg.value("query", ""));
        return true;
    }
    if (type == "chat") {
        lobby_.chat_in_room(shared_from_this(), msg.value("message", ""));
        return true;
    }

    return false;
}
