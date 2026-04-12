#pragma once
#include <string>
#include <optional>
#include <vector>
#include "db/db_types.h"

class UserService {
public:
    static bool register_user(const std::string& username, const std::string& password);
    static std::optional<User> login_user(const std::string& username, const std::string& password);
    static std::optional<User> get_user_by_username(const std::string& username);
    
    static std::vector<User> get_all_users();
    static bool create_user(const std::string& username, const std::string& password, const std::string& role);
    static bool update_user(int id, const std::string& username, const std::string& password, const std::string& role);
    static bool delete_user(int id);

    static bool apply_punishment(const std::string& username, int ban_days, bool can_chat, bool can_create_room);
};
