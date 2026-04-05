#pragma once
#include <string>
#include <vector>
#include "db/db_types.h"

namespace db::match {
    bool save_match(const std::string& username, const std::string& opponent,
                    const std::string& result, int duration_seconds,
                    const std::vector<std::string>& moves);

    std::vector<MatchRecord> get_history(const std::string& username, int limit = 20);

    std::vector<std::string> get_match_moves(int match_id);
}
