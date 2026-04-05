#pragma once
#include <string>
#include <vector>

struct MatchRecord {
    int id;
    std::string opponent;
    std::string result;  
    std::string played_at;
    int duration_seconds;
};

struct User {
    int id;
    std::string username;
};
