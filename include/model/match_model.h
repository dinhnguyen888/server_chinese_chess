#pragma once
#include <string>
#include "db/schema_def.h"

struct match_history {
    primary_key type("SERIAL") int id;
    index type("VARCHAR(50) NOT NULL") std::string username;
    type("VARCHAR(50) NOT NULL") std::string opponent;
    type("VARCHAR(10) NOT NULL") std::string result;
    type("INT DEFAULT 0") int duration_seconds;
    index type("TIMESTAMP DEFAULT NOW()") std::string played_at;
};

struct match_moves {
    primary_key type("SERIAL") int id;
    one2many("match_history(id) ON DELETE CASCADE") type("INT NOT NULL") int match_id;
    type("INT NOT NULL") int move_index;
    type("VARCHAR(150) NOT NULL") std::string move_data;
};
