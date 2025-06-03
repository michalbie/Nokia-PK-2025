#pragma once

#include "BaseState.hpp"
#include "Messages/MessageId.hpp"
#include "UeGui/IDialMode.hpp"
#include <chrono>

namespace ue
{

  class DialingState : public BaseState
  {
  public:
      DialingState(Context& context);
  
      void handleUserAction(const std::string& id) override;
      void handleTimeout() override;
      void handleCallAccepted(common::PhoneNumber from) override;
      void handleCallDropped(common::PhoneNumber from) override;
      void handleUnknownRecipient(common::MessageId msgId, common::PhoneNumber from) override;
      void handleDisconnect() override;
      void handleCallRequest(common::PhoneNumber from) override;
      void handleSms(const common::PhoneNumber& from, const std::string& text) override;

  
  private:
      common::PhoneNumber dialedNumber;
  };

} // namespace ue
