#include "network/session/WsSession.h"
#include "services/MatchHistoryService.h"

using json = nlohmann::json;

bool WsSession::handle_history_message(const json& msg, const std::string& type) {
    if (type == "get_history") {
        auto records = match_history_service_.get_history(name_, 20);
        json arr = json::array();
        for (const auto& record : records) {
            arr.push_back({
                {"id", record.id},
                {"opponent", record.opponent},
                {"result", record.result},
                {"played_at", record.played_at},
                {"duration_seconds", record.duration_seconds},
            });
        }
        send_json(json{{"type", "history_list"}, {"records", arr}});
        return true;
    }

    if (type == "get_replay") {
        int match_id = msg.value("match_id", -1);
        if (match_id != -1) {
            auto moves = match_history_service_.get_replay_moves(match_id);
            json moves_arr = json::array();
            for (const auto& move : moves) {
                moves_arr.push_back(move);
            }
            send_json(json{{"type", "replay_data"}, {"match_id", match_id}, {"moves", moves_arr}});
        }
        return true;
    }

    return false;
}
