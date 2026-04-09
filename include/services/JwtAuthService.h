#pragma once
#include "services/AuthService.h"

class JwtAuthService : public IAuthService {
public:
    explicit JwtAuthService(std::string jwt_secret);

    AuthResult verify_token(const std::string& token) override;
    AuthResult register_user(const std::string& username, const std::string& password) override;
    AuthResult login_user(const std::string& username, const std::string& password) override;

private:
    std::string generate_jwt(const std::string& username) const;
    std::string decode_and_verify_jwt(const std::string& token) const;

    std::string jwt_secret_;
};
