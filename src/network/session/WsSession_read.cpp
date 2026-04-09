#include "network/session/WsSession.h"
#include <iostream>

namespace beast = boost::beast;
namespace websocket = beast::websocket;

namespace {
void fail(beast::error_code ec, const char* what) {
    if (ec != websocket::error::closed) {
        std::cerr << what << ": " << ec.message() << "\n";
    }
}
}  // namespace

void WsSession::do_read() {
    auto self = shared_from_this();
    ws_.async_read(buffer_, [self](beast::error_code ec, std::size_t n) { self->on_read(ec, n); });
}

void WsSession::on_read(beast::error_code ec, std::size_t /*bytes*/) {
    if (ec) {
        if (ec != websocket::error::closed) {
            fail(ec, "read");
        }
        on_close();
        return;
    }

    std::string text = beast::buffers_to_string(buffer_.data());
    buffer_.consume(buffer_.size());
    handle_message(text);
    do_read();
}
