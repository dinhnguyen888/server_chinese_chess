#pragma once
#include <string>
#include <vector>

#include "model/report_model.h"

class ReportService {
public:
    static bool create_report(const std::string& reporter, const std::string& reported, int match_id, const std::string& reason);
    static std::vector<reports> get_all_reports();
    static bool update_report_status(int id, const std::string& status);
};
