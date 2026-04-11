#include "db/database.h"
#include <pqxx/pqxx>
#include <iostream>

static std::string s_conn_str;

namespace db {
const std::string& conn_str() {
    return s_conn_str;
}

bool auto_create(const std::string& host, const std::string& port,
                 const std::string& user, const std::string& password,
                 const std::string& dbname) {
    try {
        std::string admin_conn = "host=" + host + " port=" + port
                               + " dbname=postgres user=" + user
                               + " password=" + password;
        pqxx::connection c(admin_conn);
        pqxx::nontransaction tx(c);

        pqxx::result r = tx.exec_params("SELECT 1 FROM pg_database WHERE datname=$1", dbname);
        if (r.empty()) {
            std::cout << "Database '" << dbname << "' chua ton tai, dang tao moi...\n";
            tx.exec("CREATE DATABASE " + dbname);
            std::cout << "Tao Database '" << dbname << "' thanh cong!\n";
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "auto_create error: " << e.what() << "\n";
        return false;
    }
}

bool connect(const std::string& conn_str) {
    try {
        pqxx::connection c(conn_str);
        if (c.is_open()) {
            s_conn_str = conn_str;
            return true;
        }
        return false;
    } catch (const std::exception& e) {
        std::cerr << "DB connect error: " << e.what() << "\n";
        return false;
    }
}

void init_schema() {
    try {
        pqxx::connection c(s_conn_str);
        pqxx::work w(c);

        w.exec(
            "CREATE TABLE IF NOT EXISTS users ("
            "  id       SERIAL PRIMARY KEY,"
            "  username VARCHAR(50) UNIQUE NOT NULL,"
            "  password VARCHAR(255) NOT NULL"
            ");"
        );
        w.exec(
            "CREATE TABLE IF NOT EXISTS match_history ("
            "  id               SERIAL PRIMARY KEY,"
            "  username         VARCHAR(50) NOT NULL,"
            "  opponent         VARCHAR(50) NOT NULL,"
            "  result           VARCHAR(10) NOT NULL,"
            "  duration_seconds INT DEFAULT 0,"
            "  played_at        TIMESTAMP DEFAULT NOW()"
            ");"
        );
        w.exec(
            "CREATE TABLE IF NOT EXISTS match_moves ("
            "  id         SERIAL PRIMARY KEY,"
            "  match_id   INT NOT NULL REFERENCES match_history(id) ON DELETE CASCADE,"
            "  move_index INT NOT NULL,"
            "  move_data  VARCHAR(150) NOT NULL"
            ");"
        );

        w.commit();
        std::cout << "Schema OK (users, match_history, match_moves).\n";
    } catch (const std::exception& e) {
        std::cerr << "init_schema error: " << e.what() << "\n";
    }
}

} 
