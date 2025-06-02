#pragma once

#include "BaseState.hpp"
#include "NotConnectedState.hpp"

namespace ue
{

class ReceivingCallState : public BaseState
{
public:
    ReceivingCallState(Context& context, common::PhoneNumber caller);

    void handleUserAction(const std::string& id) override;
    void handleTimeout() override;

    // Call event handlers for incoming calls
    void handleCallAccepted(common::PhoneNumber from) override;
    void handleCallDropped(common::PhoneNumber from) override;
    void handleCallTalk(common::PhoneNumber from, const std::string& text) override;

    virtual void handleDisconnect() override;
    virtual void handleUnknownRecipient(common::MessageId msgId, common::PhoneNumber from) override;

    void handleCallRequest(common::PhoneNumber from) override;
    void handleSms(const common::PhoneNumber& from, const std::string& text) override;

private:
    common::PhoneNumber caller;
};

} // namespace ue
