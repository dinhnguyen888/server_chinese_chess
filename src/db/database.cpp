#include "db/database.h"
#include <pqxx/pqxx>
#include <iostream>
#include <vector>
#include <functional>

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

void apply_migrations(pqxx::connection& c) {
    pqxx::work w(c);
    
    // 1. Create migrations table if not exists
    w.exec("CREATE TABLE IF NOT EXISTS schema_migrations (version INT PRIMARY KEY, applied_at TIMESTAMP DEFAULT NOW());");
    
    // 2. Get current version
    int current_version = 0;
    pqxx::result r = w.exec("SELECT MAX(version) FROM schema_migrations;");
    if (!r.empty() && !r[0][0].is_null()) {
        current_version = r[0][0].as<int>();
    }

    // 3. Define migrations
    struct MigrationStep {
        int version;
        std::string description;
        std::vector<std::string> sqls;
    };

    std::vector<MigrationStep> migrations = {
        {1, "Initial baseline", {
            "CREATE TABLE IF NOT EXISTS users (id SERIAL PRIMARY KEY, username VARCHAR(50) UNIQUE NOT NULL, password VARCHAR(255) NOT NULL, role VARCHAR(20) DEFAULT 'user');",
            "CREATE TABLE IF NOT EXISTS match_history (id SERIAL PRIMARY KEY, username VARCHAR(50) NOT NULL, opponent VARCHAR(50) NOT NULL, result VARCHAR(10) NOT NULL, duration_seconds INT DEFAULT 0, played_at TIMESTAMP DEFAULT NOW());",
            "CREATE TABLE IF NOT EXISTS match_moves (id SERIAL PRIMARY KEY, match_id INT NOT NULL REFERENCES match_history(id) ON DELETE CASCADE, move_index INT NOT NULL, move_data VARCHAR(150) NOT NULL);"
        }},
        {2, "Add reports table", {
            "CREATE TABLE IF NOT EXISTS reports (id SERIAL PRIMARY KEY, reporter VARCHAR(50) NOT NULL, reported VARCHAR(50) NOT NULL, match_id INT REFERENCES match_history(id) ON DELETE SET NULL, reason TEXT NOT NULL, status VARCHAR(20) DEFAULT 'pending', created_at TIMESTAMP DEFAULT NOW());"
        }},
        {3, "Add punishment fields to users", {
            "ALTER TABLE users ADD COLUMN IF NOT EXISTS banned_until TIMESTAMP DEFAULT NULL;",
            "ALTER TABLE users ADD COLUMN IF NOT EXISTS can_chat BOOLEAN DEFAULT TRUE;",
            "ALTER TABLE users ADD COLUMN IF NOT EXISTS can_create_room BOOLEAN DEFAULT TRUE;"
        }}
    };

    // 4. Run pending migrations
    bool changed = false;
    for (const auto& m : migrations) {
        if (m.version > current_version) {
            std::cout << "[Migration] Applying version " << m.version << ": " << m.description << "\n";
            for (const auto& sql : m.sqls) {
                w.exec(sql);
            }
            w.exec_params("INSERT INTO schema_migrations (version) VALUES ($1);", m.version);
            changed = true;
        }
    }

    if (changed) {
        w.commit();
        std::cout << "[Migration] All migrations applied successfully.\n";
    } else {
        std::cout << "[Migration] Database is up to date (Version " << current_version << ").\n";
    }
}

void init_schema() {
    try {
        pqxx::connection c(s_conn_str);
        
        // Run migrations
        apply_migrations(c);

        // Post-migration: Create default admin
        {
            pqxx::work w2(c);
            pqxx::result r = w2.exec_params("SELECT 1 FROM users WHERE username=$1", "admin");
            if (r.empty()) {
                w2.exec_params("INSERT INTO users (username, password, role) VALUES ($1, $2, $3)",
                             "admin", "admin123", "admin");
                w2.commit();
                std::cout << "[Init] Created default admin account (admin/admin123)\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "init_schema error: " << e.what() << "\n";
    }
}

} 
