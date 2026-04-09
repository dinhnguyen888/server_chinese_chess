#pragma once
#include <string>
#include <vector>

struct HistoryRecordDto {
    int id{0};
    std::string opponent;
    std::string result;
    std::string played_at;
    int duration_seconds{0};
};

class IMatchHistoryService {
public:
    virtual ~IMatchHistoryService() = default;

    virtual void save_two_sided_result(const std::string& player,
                                       const std::string& opponent,
                                       const std::string& player_result,
                                       int duration_seconds,
                                       const std::vector<std::string>& moves) = 0;

    virtual std::vector<HistoryRecordDto> get_history(const std::string& username, int limit) = 0;
    virtual std::vector<std::string> get_replay_moves(int match_id) = 0;
};
