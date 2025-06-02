#pragma once

#include "BaseState.hpp"

namespace ue {

class TalkingState : public BaseState {
public:
    TalkingState(Context& context, common::PhoneNumber peer);

    void handleUserAction(const std::string& id) override;
    void handleCallTalk(common::PhoneNumber from, const std::string& text) override;
    void handleCallDropped(common::PhoneNumber from) override;
    void handleDisconnect() override;
    void handleUnknownRecipient(common::MessageId msgId, common::PhoneNumber from) override;


    void handleSms(const common::PhoneNumber& from, const std::string& text) override;
    void handleCallRequest(common::PhoneNumber from) override;
private:
    common::PhoneNumber peer;
};

} // namespace ue
