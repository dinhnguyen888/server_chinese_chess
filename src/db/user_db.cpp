#include "db/user_db.h"
#include "db/database.h"
#include <pqxx/pqxx>
#include <iostream>

namespace db::user {

bool register_user(const std::string& username, const std::string& password) {
    if (username.empty() || username.length() > 20) return false;
    try {
        pqxx::connection c(db::conn_str());
        pqxx::work w(c);
        w.exec_params("INSERT INTO users (username, password, role) VALUES ($1, $2, 'user');",
                      username, password);
        w.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "user::register error: " << e.what() << "\n";
        return false;
    }
}

std::optional<User> login_user(const std::string& username, const std::string& password) {
    if (username.empty() || username.length() > 20) return std::nullopt;
    try {
        pqxx::connection c(db::conn_str());
        pqxx::nontransaction w(c);
        pqxx::result r = w.exec_params(
            "SELECT id, username, role FROM users WHERE username=$1 AND password=$2;",
            username, password
        );
        if (r.empty()) return std::nullopt;
        
        User user;
        user.id = r[0]["id"].as<int>();
        user.username = r[0]["username"].c_str();
        user.role = r[0]["role"].c_str();
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
            "SELECT id, username, role FROM users WHERE username=$1;",
            username
        );
        if (r.empty()) return std::nullopt;
        
        User user;
        user.id = r[0]["id"].as<int>();
        user.username = r[0]["username"].c_str();
        user.role = r[0]["role"].c_str();
        return user;
    } catch (const std::exception& e) {
        std::cerr << "user::get_user_by_username error: " << e.what() << "\n";
        return std::nullopt;
    }
}

std::vector<User> get_all_users() {
    std::vector<User> users;
    try {
        pqxx::connection c(db::conn_str());
        pqxx::nontransaction w(c);
        pqxx::result r = w.exec("SELECT id, username, role FROM users ORDER BY id ASC;");
        for (auto const& row : r) {
            User u;
            u.id = row["id"].as<int>();
            u.username = row["username"].c_str();
            u.role = row["role"].c_str();
            users.push_back(u);
        }
    } catch (const std::exception& e) {
        std::cerr << "user::get_all_users error: " << e.what() << "\n";
    }
    return users;
}

bool create_user(const std::string& username, const std::string& password, const std::string& role) {
    try {
        pqxx::connection c(db::conn_str());
        pqxx::work w(c);
        w.exec_params("INSERT INTO users (username, password, role) VALUES ($1, $2, $3);",
                      username, password, role);
        w.commit();
        return true;
    } catch (...) { return false; }
}

bool update_user(int id, const std::string& username, const std::string& password, const std::string& role) {
    try {
        pqxx::connection c(db::conn_str());
        pqxx::work w(c);
        if (password.empty()) {
            w.exec_params("UPDATE users SET username=$1, role=$2 WHERE id=$3;", username, role, id);
        } else {
            w.exec_params("UPDATE users SET username=$1, password=$2, role=$3 WHERE id=$4;", username, password, role, id);
        }
        w.commit();
        return true;
    } catch (...) { return false; }
}

bool delete_user(int id) {
    try {
        pqxx::connection c(db::conn_str());
        pqxx::work w(c);
        w.exec_params("DELETE FROM users WHERE id=$1;", id);
        w.commit();
        return true;
    } catch (...) { return false; }
}

} 