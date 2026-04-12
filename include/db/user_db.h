#pragma once
#include <string>
#include <optional>
#include "db/db_types.h"

namespace db::user {
    bool register_user(const std::string& username, const std::string& password);
    std::optional<User> login_user(const std::string& username, const std::string& password);
    std::optional<User> get_user_by_username(const std::string& username);
    
    std::vector<User> get_all_users();
    bool create_user(const std::string& username, const std::string& password, const std::string& role);
    bool update_user(int id, const std::string& username, const std::string& password, const std::string& role);
    bool delete_user(int id);

    bool apply_punishment(const std::string& username, int ban_days, bool can_chat, bool can_create_room);
}
