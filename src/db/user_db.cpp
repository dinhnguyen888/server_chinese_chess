#include "db/user_db.h"
#include "db/database.h"
#include <pqxx/pqxx>
#include <iostream>

namespace db::user {

bool register_user(const std::string& username, const std::string& password) {
    if (username.empty() || username.length() > 10) return false;
    try {
        pqxx::connection c(db::conn_str());
        pqxx::work w(c);
        w.exec_params("INSERT INTO users (username, password) VALUES ($1, $2);",
                      username, password);
        w.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "user::register error: " << e.what() << "\n";
        return false;
    }
}

std::optional<User> login_user(const std::string& username, const std::string& password) {
    if (username.empty() || username.length() > 10) return std::nullopt;
    try {
        pqxx::connection c(db::conn_str());
        pqxx::nontransaction w(c);
        pqxx::result r = w.exec_params(
            "SELECT id, username FROM users WHERE username=$1 AND password=$2;",
            username, password
        );
        if (r.empty()) return std::nullopt;
        
        User user;
        user.id = r[0]["id"].as<int>();
        user.username = r[0]["username"].c_str();
        return user;
    } catch (const std::exception& e) {
        std::cerr << "user::login error: " << e.what() << "\n";
        return std::nullopt;
    }
}

std::optional<User> get_user_by_username(const std::string& username) {
    if (username.empty()) return std::nullopt;
    try {
        pqxx::connection c(db::conn_str());
        pqxx::nontransaction w(c);
        pqxx::result r = w.exec_params(
            "SELECT id, username FROM users WHERE username=$1;",
            username
        );
        if (r.empty()) return std::nullopt;
        
        User user;
        user.id = r[0]["id"].as<int>();
        user.username = r[0]["username"].c_str();
        return user;
    } catch (const std::exception& e) {
        std::cerr << "user::get_user_by_username error: " << e.what() << "\n";
        return std::nullopt;
    }
}

} 