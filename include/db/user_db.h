#pragma once
#include <string>
#include <optional>
#include "db/db_types.h"

namespace db::user {
    bool register_user(const std::string& username, const std::string& password);
    std::optional<User> login_user(const std::string& username, const std::string& password);
    std::optional<User> get_user_by_username(const std::string& username);
}
