#pragma once
#include <string>
#include "db/schema_def.h"

struct notify {
    primary_key type("SERIAL") int id;
    
};
