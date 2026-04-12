#pragma once
#include <string>
#include "db/schema_def.h"

struct users {
    primary_key type("SERIAL") int id;
    unique index type("VARCHAR(50) NOT NULL") std::string username;
    type("VARCHAR(255) NOT NULL") std::string password;
    type("VARCHAR(20) DEFAULT 'user'") std::string role;
    type("TIMESTAMP DEFAULT NULL") std::string banned_until;
    type("BOOLEAN DEFAULT TRUE") bool can_chat;
    type("BOOLEAN DEFAULT TRUE") bool can_create_room;
};
