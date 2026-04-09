#include "game/MatchLobby.h"
#include "network/session/WsSession.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void MatchLobby::create_room(std::shared_ptr<WsSession> session, const std::string& room_name, bool auto_start) {
    std::lock_guard<std::mutex> lock(mutex_);

    leave_room_internal(session);

    std::string room_id = std::to_string(next_room_id_++);
    Room room;
    room.id = room_id;
    room.name = room_name;
    room.host = session;
    room.is_playing = false;
    room.auto_start = auto_start;

    rooms_[room_id] = room;
    session->current_room_id_ = room_id;

    session->send_json(json{{"type", "room_created"},
                            {"roomId", room_id},
                            {"roomName", room_name},
                            {"autoStart", auto_start}});
}

void MatchLobby::join_room(std::shared_ptr<WsSession> session, const std::string& room_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = rooms_.find(room_id);
    if (it == rooms_.end()) {
        session->send_json(json{{"type", "error"}, {"message", "Phòng không tồn tại"}});
        return;
    }

    Room& room = it->second;
    if (room.host == session) {
        session->send_json(json{{"type", "error"}, {"message", "Bạn là chủ phòng này"}});
        return;
    }

    if (room.guest || room.is_playing) {
        session->send_json(json{{"type", "error"}, {"message", "Phòng đã đầy"}});
        return;
    }

    room.guest = session;
    session->current_room_id_ = room_id;

    session->send_json(
        json{{"type", "room_joined"}, {"roomId", room_id}, {"hostName", room.host ? room.host->name_ : "Unknown"}});

    if (room.host) {
        room.host->send_json(json{{"type", "guest_joined"}, {"guestName", session->name_}, {"roomId", room_id}});
    }

    if (room.auto_start) {
        do_start_game(room);
    }
}

void MatchLobby::start_game(std::shared_ptr<WsSession> session, const std::string& room_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = rooms_.find(room_id);
    if (it == rooms_.end()) {
        session->send_json(json{{"type", "error"}, {"message", "Phòng không tồn tại"}});
        return;
    }

    Room& room = it->second;
    if (room.host != session) {
        session->send_json(json{{"type", "error"}, {"message", "Chỉ chủ phòng mới có thể bắt đầu"}});
        return;
    }

    if (!room.guest) {
        session->send_json(json{{"type", "error"}, {"message", "Chưa có người chơi vào phòng"}});
        return;
    }

    do_start_game(room);
}

void MatchLobby::do_start_game(Room& room) {
    room.is_playing = true;

    auto host = room.host;
    auto guest = room.guest;

    host->attach_opponent(guest);
    guest->attach_opponent(host);

    constexpr int kRedFirst = 1;
    host->send_json(json{{"type", "matched"},
                         {"color", "r"},
                         {"orderSide", kRedFirst},
                         {"opponentName", guest->name_},
                         {"roomId", room.id}});
    guest->send_json(json{{"type", "matched"},
                          {"color", "b"},
                          {"orderSide", kRedFirst},
                          {"opponentName", host->name_},
                          {"roomId", room.id}});
}

void MatchLobby::leave_room_internal(const std::shared_ptr<WsSession>& session) {
    if (session->current_room_id_.empty()) {
        return;
    }

    auto it = rooms_.find(session->current_room_id_);
    if (it != rooms_.end()) {
        Room& room = it->second;
        if (room.host == session) {
            if (room.guest) {
                room.guest->current_room_id_.clear();
                room.guest->send_json(json{{"type", "opponent_left"}});
            }
            rooms_.erase(it);
        } else if (room.guest == session) {
            room.guest = nullptr;
            room.is_playing = false;
            if (room.host) {
                room.host->send_json(json{{"type", "guest_left"}, {"roomId", room.id}});
            }
        }
    }

    session->current_room_id_.clear();
}
