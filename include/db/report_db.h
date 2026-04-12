#pragma once
#include <string>
#include <vector>

namespace db::report {
    struct Report {
        int id;
        std::string reporter;
        std::string reported;
        int match_id;
        std::string reason;
        std::string status;
        std::string created_at;
    };

    bool create_report(const std::string& reporter, const std::string& reported, int match_id, const std::string& reason);
    std::vector<Report> get_all_reports();
    bool update_report_status(int id, const std::string& status);
}
