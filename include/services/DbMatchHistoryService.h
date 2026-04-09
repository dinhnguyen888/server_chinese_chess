#pragma once
#include "services/MatchHistoryService.h"

class DbMatchHistoryService : public IMatchHistoryService {
public:
    void save_two_sided_result(const std::string& player,
                               const std::string& opponent,
                               const std::string& player_result,
                               int duration_seconds,
                               const std::vector<std::string>& moves) override;

    std::vector<HistoryRecordDto> get_history(const std::string& username, int limit) override;
    std::vector<std::string> get_replay_moves(int match_id) override;
};
