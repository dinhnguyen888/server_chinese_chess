#include "services/JwtAuthService.h"
#include "db/user_db.h"
#include <chrono>
#include <iostream>
#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/nlohmann-json/defaults.h>
#include <utility>

JwtAuthService::JwtAuthService(std::string jwt_secret)
    : jwt_secret_(std::move(jwt_secret)) {}

std::string JwtAuthService::generate_jwt(const std::string& username) const {
    return jwt::create()
        .set_issuer("chinese_chess")
        .set_type("JWS")
        .set_payload_claim("username", jwt::claim(username))
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours(24 * 7))
        .sign(jwt::algorithm::hs256{jwt_secret_});
}

std::string JwtAuthService::decode_and_verify_jwt(const std::string& token) const {
    try {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{jwt_secret_})
            .with_issuer("chinese_chess");
        verifier.verify(decoded);
        return decoded.get_payload_claim("username").as_string();
    } catch (const std::exception& e) {
        std::cerr << "JWT verification failed: " << e.what() << "\n";
        return "";
    }
}

AuthResult JwtAuthService::verify_token(const std::string& token) {
    AuthResult result;
    auto user = decode_and_verify_jwt(token);
    if (!user.empty()) {
        result.success = true;
        result.username = user;
        return result;
    }

    result.error_message = "Phiên đăng nhập hết hạn hoặc không hợp lệ";
    return result;
}

AuthResult JwtAuthService::register_user(const std::string& username, const std::string& password) {
    AuthResult result;
    if (db::user::register_user(username, password)) {
        result.success = true;
        result.username = username;
        result.token = generate_jwt(username);
        return result;
    }

    result.error_message = "Tên đăng nhập đã tồn tại hoặc có lỗi xảy ra";
    return result;
}

AuthResult JwtAuthService::login_user(const std::string& username, const std::string& password) {
    AuthResult result;
    auto user_opt = db::user::login_user(username, password);
    if (user_opt) {
        result.success = true;
        result.username = user_opt->username;
        result.token = generate_jwt(result.username);
        return result;
    }

    result.error_message = "Sai tên đăng nhập hoặc mật khẩu";
    return result;
}
