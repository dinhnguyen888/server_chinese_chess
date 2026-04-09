#include "services/DbMatchHistoryService.h"
#include "db/match_db.h"

namespace {
std::string opposite_result(const std::string& result) {
    if (result == "win") {
        return "lose";
    }
    if (result == "lose") {
        return "win";
    }
    return "draw";
}
}  // namespace

void DbMatchHistoryService::save_two_sided_result(const std::string& player,
                                                  const std::string& opponent,
                                                  const std::string& player_result,
                                                  int duration_seconds,
                                                  const std::vector<std::string>& moves) {
    if (player.empty() || opponent.empty()) {
        return;
    }

    db::match::save_match(player, opponent, player_result, duration_seconds, moves);
    db::match::save_match(opponent, player, opposite_result(player_result), duration_seconds, moves);
}

std::vector<HistoryRecordDto> DbMatchHistoryService::get_history(const std::string& username, int limit) {
    std::vector<HistoryRecordDto> output;
    auto records = db::match::get_history(username, limit);
    output.reserve(records.size());

    for (const auto& record : records) {
        output.push_back(HistoryRecordDto{
            record.id,
            record.opponent,
            record.result,
            record.played_at,
            record.duration_seconds,
        });
    }

    return output;
}

std::vector<std::string> DbMatchHistoryService::get_replay_moves(int match_id) {
    return db::match::get_match_moves(match_id);
}
