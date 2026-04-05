#include "db/match_db.h"
#include "db/database.h"
#include <pqxx/pqxx>
#include <iostream>

namespace db::match {

bool save_match(const std::string& username, const std::string& opponent,
                const std::string& result, int duration_seconds,
                const std::vector<std::string>& moves) {
    try {
        pqxx::connection c(db::conn_str());
        pqxx::work w(c);

        pqxx::result res = w.exec_params(
            "INSERT INTO match_history (username, opponent, result, duration_seconds)"
            " VALUES ($1, $2, $3, $4) RETURNING id;",
            username, opponent, result, duration_seconds
        );
        if (res.empty()) return false;
        int match_id = res[0][0].as<int>();

        for (size_t i = 0; i < moves.size(); ++i) {
            w.exec_params(
                "INSERT INTO match_moves (match_id, move_index, move_data) VALUES ($1, $2, $3);",
                match_id, (int)i, moves[i]
            );
        }

        w.commit();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "match::save error: " << e.what() << "\n";
        return false;
    }
}

std::vector<MatchRecord> get_history(const std::string& username, int limit) {
    std::vector<MatchRecord> records;
    try {
        pqxx::connection c(db::conn_str());
        pqxx::nontransaction w(c);
        pqxx::result r = w.exec_params(
            "SELECT id, opponent, result,"
            "       TO_CHAR(played_at, 'DD/MM/YYYY HH24:MI') AS played_at,"
            "       duration_seconds"
            " FROM match_history"
            " WHERE username=$1"
            " ORDER BY played_at DESC"
            " LIMIT $2;",
            username, limit
        );
        for (const auto& row : r) {
            MatchRecord rec;
            rec.id               = row["id"].as<int>();
            rec.opponent         = row["opponent"].c_str();
            rec.result           = row["result"].c_str();
            rec.played_at        = row["played_at"].c_str();
            rec.duration_seconds = row["duration_seconds"].as<int>();
            records.push_back(rec);
        }
    } catch (const std::exception& e) {
        std::cerr << "match::get_history error: " << e.what() << "\n";
    }
    return records;
}

std::vector<std::string> get_match_moves(int match_id) {
    std::vector<std::string> moves;
    try {
        pqxx::connection c(db::conn_str());
        pqxx::nontransaction w(c);
        pqxx::result r = w.exec_params(
            "SELECT move_data"
            " FROM match_moves"
            " WHERE match_id=$1"
            " ORDER BY move_index ASC;",
            match_id
        );
        for (const auto& row : r) {
            moves.push_back(row["move_data"].c_str());
        }
    } catch (const std::exception& e) {
        std::cerr << "match::get_match_moves error: " << e.what() << "\n";
    }
    return moves;
}
} 
