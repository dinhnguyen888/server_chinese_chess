#pragma once
#include <memory>
#include <mutex>
#include <string>
#include <map>

class WsSession;

struct Room {
    std::string id;
    std::string name;
    std::shared_ptr<WsSession> host;
    std::shared_ptr<WsSession> guest;
    bool is_playing;
};

class MatchLobby {
public:
    void try_pair(std::shared_ptr<WsSession> session);
    void cancel_waiting(const std::shared_ptr<WsSession>& session);

    void create_room(std::shared_ptr<WsSession> session, const std::string& room_name);
    void join_room(std::shared_ptr<WsSession> session, const std::string& room_id);
    void get_room_list(std::shared_ptr<WsSession> session);
    void search_room(std::shared_ptr<WsSession> session, const std::string& query);
    void chat_in_room(std::shared_ptr<WsSession> sender, const std::string& message);

private:
    void leave_room_internal(const std::shared_ptr<WsSession>& session);

    std::mutex mutex_;
    std::shared_ptr<WsSession> waiting_;
    
    std::map<std::string, Room> rooms_;
    int next_room_id_ = 1;
};
