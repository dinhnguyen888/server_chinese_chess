#include "network/session/WsSession.h"
#include "services/AuthService.h"

using json = nlohmann::json;

bool WsSession::handle_auth_message(const json& msg, const std::string& type) {
    if (type == "verify_jwt") {
        auto result = auth_service_.verify_token(msg.value("token", ""));
        if (result.success) {
            name_ = result.username;
            send_json(json{{"type", "auth_success"}, {"username", result.username}, {"action", "verify"}});
        } else {
            send_json(json{{"type", "auth_fail"}, {"message", result.error_message}});
        }
        return true;
    }

    if (type == "register") {
        auto result = auth_service_.register_user(msg.value("username", ""), msg.value("password", ""));
        if (result.success) {
            name_ = result.username;
            send_json(json{{"type", "auth_success"},
                           {"username", result.username},
                           {"action", "register"},
                           {"token", result.token}});
        } else {
            send_json(json{{"type", "error"}, {"message", result.error_message}});
        }
        return true;
    }

    if (type == "login") {
        auto result = auth_service_.login_user(msg.value("username", ""), msg.value("password", ""));
        if (result.success) {
            name_ = result.username;
            send_json(json{{"type", "auth_success"},
                           {"username", result.username},
                           {"action", "login"},
                           {"token", result.token}});
        } else {
            send_json(json{{"type", "error"}, {"message", result.error_message}});
        }
        return true;
    }

    return false;
}
