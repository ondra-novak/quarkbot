#pragma once

#include <string_view>
#include "basic_types.h"

namespace quarkbot {

///WebSocket client
class IWebSocketClient {
public:
    static constexpr int default_reason = 1000;

    virtual ~IWebSocketClient() = default;

    enum FrameType {
        text,       ///<text frame
        binary,     ///<binary frame
        closed      ///<close frame (get_close_reason())
    };

    struct Message {
        FrameType type;
        std::string_view message;
    };


    ///Send message
    virtual bool send(Message message) noexcept = 0;
    ///Receive message
    virtual awaitable<Message> receive() noexcept = 0;
    ///receive close reason
    virtual int get_close_reason() const noexcept = 0;
    ///close current connection
    virtual void close(int reason = default_reason) noexcept = 0;

};




}
