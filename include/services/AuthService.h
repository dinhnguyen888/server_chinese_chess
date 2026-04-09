#pragma once
#include <string>

struct AuthResult {
    bool success{false};
    std::string username;
    std::string token;
    std::string error_message;
};

class IAuthService {
public:
    virtual ~IAuthService() = default;

    virtual AuthResult verify_token(const std::string& token) = 0;
    virtual AuthResult register_user(const std::string& username, const std::string& password) = 0;
    virtual AuthResult login_user(const std::string& username, const std::string& password) = 0;
};
