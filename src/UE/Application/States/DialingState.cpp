#include "DialingState.hpp"
#include "ConnectedState.hpp"
#include "NotConnectedState.hpp"
#include <chrono>
#include "Messages/MessageId.hpp"
#include "TalkingState.hpp"

namespace ue
{

  DialingState::DialingState(Context& context)
  : BaseState(context, "DialingState")
{
  context.user.showDialing();
  context.timer.startTimer(std::chrono::seconds(30));
  logger.logInfo("Entered DialingState.");
}

void DialingState::handleUserAction(const std::string& id)
{
    logger.logDebug("DialingState handleUserAction: ", id);
    if(id == "ACCEPT") {
        if (context.user.getDialedNumber(dialedNumber)) {
            context.bts.sendCallRequest(dialedNumber);
            context.user.showCalling(dialedNumber);
            logger.logInfo("Call request sent to ", dialedNumber);
        } else {
            logger.logError("Failed to retrieve dialed phone number");
        }
    }
    else if(id == "REJECT") {
        logger.logInfo("User cancelled dialing. Transitioning back to ConnectedState.");
        context.bts.sendCallDropped(dialedNumber);
        context.timer.stopTimer();
        context.setState<ConnectedState>();
    }
    else {
        BaseState::handleUserAction(id);
    }
}

void DialingState::handleTimeout() {
    logger.logInfo("DialingState timeout occurred. No response to call request.");
    context.timer.stopTimer();
    context.setState<ConnectedState>();
}

void DialingState::handleCallAccepted(common::PhoneNumber from) {
    context.timer.stopTimer();
    logger.logInfo("Received CallAccepted. Transitioning to TalkingState.");
    context.setState<TalkingState>(from);
}

void DialingState::handleCallDropped(common::PhoneNumber from) {
    context.timer.stopTimer();
    logger.logInfo("Received CallDropped in DialingState. Transitioning to ConnectedState.");
    context.setState<ConnectedState>();
}

void DialingState::handleUnknownRecipient(common::MessageId msgId, common::PhoneNumber from)
{
    logger.logInfo("DialingState: UnknownRecipient. Transitioning to ConnectedState.");
    context.timer.stopTimer();
    context.setState<ConnectedState>();
}

void DialingState::handleDisconnect()
{
    logger.logInfo("DialingState: transport lost.");
    context.timer.stopTimer();
    context.setState<NotConnectedState>();
}

} // namespace ue
