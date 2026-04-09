#include "network/session/WsSession.h"
#include <iostream>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
using json = nlohmann::json;

namespace {
void fail(beast::error_code ec, const char* what) {
    if (ec != websocket::error::closed) {
        std::cerr << what << ": " << ec.message() << "\n";
    }
}
}  // namespace

void WsSession::send_json(const json& j) {
    std::string text = j.dump();
    bool idle = write_queue_.empty();
    write_queue_.push(std::move(text));
    if (idle) {
        do_write();
    }
}

void WsSession::do_write() {
    if (write_queue_.empty()) {
        return;
    }

    auto self = shared_from_this();
    ws_.async_write(boost::asio::buffer(write_queue_.front()),
                    [self](beast::error_code ec, std::size_t n) { self->on_write(ec, n); });
}

void WsSession::on_write(beast::error_code ec, std::size_t /*bytes*/) {
    if (ec) {
        fail(ec, "write");
        on_close();
        return;
    }

    write_queue_.pop();
    if (!write_queue_.empty()) {
        do_write();
    }
}
