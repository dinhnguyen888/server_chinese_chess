#pragma once
#include <string>
#include <vector>

struct MatchRecord {
    int id;
    std::string opponent;
    std::string result;  
    std::string played_at;
    int duration_seconds;
    bool has_report;
};

struct User {
    int id;
    std::string username;
    std::string role;
    std::string banned_until; // Is empty or timestamp
    bool can_chat;
    bool can_create_room;
};
