#pragma once
#include <string_view>

namespace quarkbot {

class IEventTarget;

class IMessageQueue {
public:

    virtual bool send_message(IEventTarget *target,
            std::string_view channel,
            std::string_view message,
            unsigned int conversation_id) = 0;

    virtual void subscribe(IEventTarget *target, std::string_view channel) = 0;
    virtual void unsubscribe(IEventTarget *target, std::string_view channel) = 0;
    virtual void unsubscribe_all(IEventTarget *target) = 0;
    virtual ~IMessageQueue() = default;
};

}
