#pragma once
#include <string>

// quan ly ket noi csdl
namespace db {
    bool auto_create(const std::string& host, const std::string& port,
                     const std::string& user, const std::string& password,
                     const std::string& dbname);

    bool connect(const std::string& conn_str);
    void init_schema();

    const std::string& conn_str();
}
