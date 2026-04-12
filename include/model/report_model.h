#pragma once
#include <string>
#include "db/schema_def.h"

struct reports {
    primary_key type("SERIAL") int id;
    type("VARCHAR(50) NOT NULL") std::string reporter;
    index type("VARCHAR(50) NOT NULL") std::string reported;
    one2many("match_history(id) ON DELETE SET NULL") type("INT") int match_id;
    type("TEXT NOT NULL") std::string reason;
    index type("VARCHAR(20) DEFAULT 'pending'") std::string status;
    index type("TIMESTAMP DEFAULT NOW()") std::string created_at;
};
