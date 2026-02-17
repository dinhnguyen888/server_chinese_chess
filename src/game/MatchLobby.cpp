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
    
    leave_room_internal(session);
}

void MatchLobby::create_room(std::shared_ptr<WsSession> session, const std::string& room_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string room_id = std::to_string(next_room_id_++);
    
    Room room;
    room.id = room_id;
    room.name = room_name;
    room.host = session;
    room.is_playing = false;
    
    rooms_[room_id] = room;
    session->current_room_id_ = room_id;
    
    session->send_json(json{
        {"type", "room_created"},
        {"roomId", room_id},
        {"roomName", room_name}
    });
}

void MatchLobby::join_room(std::shared_ptr<WsSession> session, const std::string& room_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = rooms_.find(room_id);
    if (it == rooms_.end()) {
        session->send_json(json{{"type", "error"}, {"message", "Room not found"}});
        return;
    }
    
    Room& room = it->second;
    if (room.guest) {
        session->send_json(json{{"type", "error"}, {"message", "Room is full"}});
        return;
    }
    
    room.guest = session;
    room.is_playing = true;
    session->current_room_id_ = room_id;
    
    auto a = room.host;
    auto b = room.guest;
    
    a->attach_opponent(b);
    b->attach_opponent(a);
    
    constexpr int kRedFirst = 1;
    a->send_json(json{{"type", "matched"}, {"color", "r"}, {"orderSide", kRedFirst}, {"opponentName", b->name_}, {"roomId", room_id}});
    b->send_json(json{{"type", "matched"}, {"color", "b"}, {"orderSide", kRedFirst}, {"opponentName", a->name_}, {"roomId", room_id}});
}

void MatchLobby::get_room_list(std::shared_ptr<WsSession> session) {
    std::lock_guard<std::mutex> lock(mutex_);
    json room_list = json::array();
    
    for (const auto& pair : rooms_) {
        const Room& r = pair.second;
        room_list.push_back({
            {"id", r.id},
            {"name", r.name},
            {"host", r.host ? r.host->name_ : "Unknown"},
            {"isPlaying", r.is_playing}
        });
    }
    
    session->send_json(json{
        {"type", "room_list"},
        {"rooms", room_list}
    });
}

void MatchLobby::search_room(std::shared_ptr<WsSession> session, const std::string& query) {
    std::lock_guard<std::mutex> lock(mutex_);
    json room_list = json::array();
    
    for (const auto& pair : rooms_) {
        const Room& r = pair.second;
        if (r.name.find(query) != std::string::npos || r.id == query) {
            room_list.push_back({
                {"id", r.id},
                {"name", r.name},
                {"host", r.host ? r.host->name_ : "Unknown"},
                {"isPlaying", r.is_playing}
            });
        }
    }
    
    session->send_json(json{
        {"type", "room_list"},
        {"rooms", room_list}
    });
}

void MatchLobby::chat_in_room(std::shared_ptr<WsSession> sender, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (sender->current_room_id_.empty()) {
        sender->send_json(json{{"type", "error"}, {"message", "You are not in a room"}});
        return;
    }
    
    auto it = rooms_.find(sender->current_room_id_);
    if (it != rooms_.end()) {
        Room& room = it->second;
        json chat_msg = {
            {"type", "chat"},
            {"sender", sender->name_},
            {"message", message}
        };
        
        if (room.host) room.host->send_json(chat_msg);
        if (room.guest) room.guest->send_json(chat_msg);
    }
}

void MatchLobby::leave_room_internal(const std::shared_ptr<WsSession>& session) {
    if (session->current_room_id_.empty()) return;
    
    auto it = rooms_.find(session->current_room_id_);
    if (it != rooms_.end()) {
        Room& room = it->second;
        if (room.host == session || room.guest == session) {
            if (room.host == session && room.guest) {
                room.guest->current_room_id_ = "";
            } else if (room.guest == session && room.host) {
                room.guest = nullptr;
                room.is_playing = false;
                session->current_room_id_ = "";
                return; 
            }
            if (room.host == session) {
                rooms_.erase(it);
            }
        }
    }
    session->current_room_id_ = "";
}
