#include "Application.hpp"
#include "Context.hpp"

#include "States/BaseState.hpp"
#include "States/NotConnectedState.hpp"
#include "States/ConnectingState.hpp"
#include "States/ConnectedState.hpp"
#include "States/ViewingSmsListState.hpp"
#include "States/ViewingSingleSmsState.hpp"
#include "States/ComposingSmsState.hpp"
#include "UeGui/IDialMode.hpp"

namespace ue
{

// Definition of Context::setState is now in Context.hpp

Application::Application(common::PhoneNumber phoneNumber,
                         common::ILogger &iLogger,
                         IBtsPort &bts,
                         IUserPort &user,
                         ITimerPort &timer)
    : context{iLogger, bts, user, timer, *this},
      logger(iLogger, "[APP] "),
      phoneNumber(phoneNumber)
{
    timer.setHandler(this);
    logger.logInfo("Started");
    context.setState<NotConnectedState>();
}

Application::~Application()
{
    logger.logInfo("Stopped");
}

void Application::handleSib(common::BtsId btsId) { if (context.state) context.state->handleSib(btsId); }
void Application::handleAttachAccept() { if (context.state) context.state->handleAttachAccept(); }
void Application::handleAttachReject() { if (context.state) context.state->handleAttachReject(); }


void Application::handleDisconnect()
{
     logger.logInfo("Handle disconnect event from transport");
     if (context.state) {
        context.state->handleDisconnect();
     }
     context.timer.stopTimer();
     context.user.showNotConnected();
     context.setState<NotConnectedState>();
}


void Application::handleSms(const common::PhoneNumber& from, const std::string& text)
{
    if (context.state) context.state->handleSms(from, text);
}

void Application::handleUserAction(const std::string& id)
{
    logger.logDebug("User action: ", id);
    if (context.state) context.state->handleUserAction(id);
}

void Application::handleCallRequest(common::PhoneNumber from) {
    if (context.state) context.state->handleCallRequest(from);
}

void Application::handleCallAccepted(common::PhoneNumber from) {
    if (context.state) context.state->handleCallAccepted(from);
}

void Application::handleCallDropped(common::PhoneNumber from) {
    if (context.state) context.state->handleCallDropped(from);
}

void Application::handleCallTalk(common::PhoneNumber from, const std::string& text) {
    if (context.state) context.state->handleCallTalk(from, text);
}


void Application::storeReceivedSms(const common::PhoneNumber& from, const std::string& text)
{
    logger.logInfo("Storing received SMS from: ", from);
    // 'to' field remains std::nullopt for received messages
    smsDb.push_back({from, std::nullopt, text, false, false}); // from, to, text, isRead, isSent
    updateSmsIndicator();
}

// Added implementation for storeSentSms
void Application::storeSentSms(const common::PhoneNumber& to, const std::string& text)
{
     logger.logInfo("Storing sent SMS to: ", to);
     // 'from' field is own number, 'to' is the recipient
     smsDb.push_back({phoneNumber, to, text, true, true}); // from, to, text, isRead=true(N/A), isSent=true
     // No indicator update needed for sent SMS
}


const std::vector<data::SmsData>& Application::getSmsDb() const
{
    return smsDb;
}

void Application::markSmsAsRead(std::size_t index)
{
    if (index < smsDb.size())
    {
         if (!smsDb[index].isSent && !smsDb[index].isRead) // Only mark received, unread messages
         {
            logger.logInfo("Marking SMS at index ", index, " as read.");
            smsDb[index].isRead = true;
            updateSmsIndicator(); // Update indicator potentially
         } else {
            logger.logDebug("SMS at index ", index, " is already read or was sent, not marking as read.");
         }
    }
    else
    {
         logger.logError("Attempted to mark invalid SMS index as read: ", index);
    }
}

void Application::updateSmsIndicator()
{
    bool unread_remain = false;
    for(const auto& sms : smsDb) {
        // Indicator only counts received, unread messages
        if (!sms.isSent && !sms.isRead) {
            unread_remain = true;
            break;
        }
    }
    logger.logInfo("Updating SMS indicator status: ", (unread_remain ? "ON" : "OFF"));
    context.user.showNewSms(unread_remain);
}

void Application::handleUnknownRecipient(common::MessageId msgId, common::PhoneNumber from)
{
    logger.logInfo("App received unknownRecipient(", msgId, ") from: ", from);
    if (context.state)
        context.state->handleUnknownRecipient(msgId, from);
}

void Application::handleTimeout()
{
    if (context.state)
        context.state->handleTimeout();
}

Context& Application::getContext() {
    return context;
}
}
