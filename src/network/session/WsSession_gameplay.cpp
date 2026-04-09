#include "network/session/WsSession.h"
#include "services/MatchHistoryService.h"
#include <iostream>

using json = nlohmann::json;

std::vector<std::string> WsSession::parse_moves_array(const json& msg) const {
    std::vector<std::string> moves_array;
    if (!msg.contains("moves") || !msg["moves"].is_array()) {
        return moves_array;
    }

    moves_array.reserve(msg["moves"].size());
    for (const auto& move : msg["moves"]) {
        moves_array.push_back(move.get<std::string>());
    }
    return moves_array;
}

bool WsSession::handle_gameplay_message(const json& msg, const std::string& type) {
    if (type == "move") {
        auto peer = opponent_lock();
        if (peer) {
            peer->send_json(msg);
        }

        if (msg.contains("winner") && !msg["winner"].is_null() && peer) {
            int winner_side = msg["winner"].get<int>();
            std::string my_name = name_;
            std::string opp_name = peer->name_;
            int duration = msg.value("duration_seconds", 0);

            std::string my_result;
            if (winner_side == 1) {
                my_result = "win";
            } else if (winner_side == -1) {
                my_result = "lose";
            } else {
                my_result = "draw";
            }

            auto moves_array = parse_moves_array(msg);
            if (!my_name.empty() && !opp_name.empty()) {
                match_history_service_.save_two_sided_result(
                    my_name, opp_name, my_result, duration, moves_array);
                std::cout << "Match saved: " << my_name << "(" << my_result << ") vs "
                          << opp_name << "\\n";
            }
        }
        return true;
    }

    if (type == "ping") {
        send_json(json{{"type", "pong"}});
        return true;
    }

    if (type == "game_result") {
        std::string opponent = msg.value("opponent", "");
        std::string result = msg.value("result", "draw");
        int duration = msg.value("duration_seconds", 0);
        auto moves_array = parse_moves_array(msg);

        if (!name_.empty() && !opponent.empty()) {
            match_history_service_.save_two_sided_result(name_, opponent, result, duration, moves_array);
        }
        return true;
    }

    return false;
}
