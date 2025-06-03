#include "BtsPort.hpp" // Includes BinaryMessage.hpp via its own include now
#include "Messages/IncomingMessage.hpp"
#include "Messages/OutgoingMessage.hpp"
#include "Messages/MessageId.hpp"

#include <vector>
#include <string>
#include <exception> // Include for std::exception

namespace ue
{

BtsPort::BtsPort(common::ILogger &logger, common::ITransport &transport, common::PhoneNumber phoneNumber)
    : logger(logger, "[BTS-PORT]"),
      transport(transport),
      phoneNumber(phoneNumber) // Store own phone number
{}

void BtsPort::start(IBtsEventsHandler &handler)
{
    transport.registerMessageCallback([this](common::BinaryMessage msg) {handleMessage(msg);});
    transport.registerDisconnectedCallback([this]() {handleDisconnect();});
    this->handler = &handler;
}

void BtsPort::stop()
{
    transport.registerMessageCallback(nullptr);
    transport.registerDisconnectedCallback(nullptr);
    handler = nullptr;
}

void BtsPort::handleMessage(common::BinaryMessage msg)
{
    try
    {
        common::IncomingMessage reader{msg};
        auto msgId = reader.readMessageId();
        auto from = reader.readPhoneNumber();
        auto to = reader.readPhoneNumber();

        if (to != phoneNumber and
            msgId != common::MessageId::Sib and
            msgId != common::MessageId::AttachResponse
           )
        {
             logger.logInfo("Received message addressed to different UE (", to, "), ignoring. MsgId: ", msgId);
             return;
        }

        // Ensure handler is not null before calling methods on it
        if (!handler) {
            logger.logError("Message received but handler is null!");
            return;
        }

        switch (msgId)
        {
        case common::MessageId::Sib:
            handler->handleSib(reader.readBtsId());
            break;
        case common::MessageId::AttachResponse:
            { // Added scope for variable
                bool accept = reader.readNumber<std::uint8_t>() != 0u;
                if (accept) handler->handleAttachAccept();
                else handler->handleAttachReject();
            }
            break;
        case common::MessageId::Sms:
            {
                // Basic implementation assumes no encryption
                std::string text = reader.readRemainingText();
                logger.logDebug("Received SMS from ", from, " with text: ", text);
                handler->handleSms(from, text);
            }
            break;
            case common::MessageId::CallRequest: 
            handler->handleCallRequest(from); 
            break;
        case common::MessageId::CallAccepted:
            handler->handleCallAccepted(from);
            break;
        case common::MessageId::CallDropped:
            handler->handleCallDropped(from); 
            break;
        case common::MessageId::CallTalk:
            {
                std::string text = reader.readRemainingText();
                handler->handleCallTalk(from, text);
            }
            break;
        case common::MessageId::UnknownRecipient:
                logger.logInfo("BTS → UE: UnknownRecipient for call request to ", from);
                    if (handler) {
                        handler->handleUnknownRecipient(common::MessageId::CallRequest, from);
                    }
                break;
        case common::MessageId::UnknownSender:
             logger.logError("Received error message from BTS: ", msgId, ", original sender: ", from);
             // TODO: Could parse failing header and call handler->handleUnknownRecipientSms etc.
             break;
        default:
            logger.logError("Unknown message received: ", msgId, ", from: ", from, ", to: ", to);
            break;
        }
    }
    catch (std::exception const& ex)
    {
        logger.logError("handleMessage error: ", ex.what());
    }
}


void BtsPort::sendAttachRequest(common::BtsId btsId)
{
    logger.logDebug("sendAttachRequest: ", btsId);
    common::OutgoingMessage msg{common::MessageId::AttachRequest, phoneNumber, {}};
    msg.writeBtsId(btsId);
    transport.sendMessage(msg.getMessage());
}

void BtsPort::handleDisconnect()
{
        logger.logInfo("Disconnected from BTS");
        if (handler) handler->handleDisconnect();
}

void BtsPort::sendSms(const common::PhoneNumber& recipient, const std::string& text)
{
    logger.logInfo("Sending SMS from ", phoneNumber, " to ", recipient);
    try
    {
        common::OutgoingMessage msg{common::MessageId::Sms, phoneNumber, recipient};
        // No encryption mode
        msg.writeNumber<std::uint8_t>(0u);
        msg.writeText(text);
        transport.sendMessage(msg.getMessage());
        logger.logDebug("SMS message sent.");
    }
    catch (const std::exception& e)
    {
        logger.logError("Failed to construct or send SMS: ", e.what());
    }
}

// Implementation for sending call-related messages
void BtsPort::sendCallRequest(const common::PhoneNumber& recipient)
{
    logger.logDebug("sendCallRequest: to ", recipient);
    try {
        common::OutgoingMessage msg{common::MessageId::CallRequest, phoneNumber, recipient};
        transport.sendMessage(msg.getMessage());
        logger.logDebug("CallRequest message sent.");
    } catch (const std::exception& e) {
        logger.logError("Failed to send CallRequest: ", e.what());
    }
}

void BtsPort::sendCallAccepted(const common::PhoneNumber& recipient)
{
    logger.logDebug("sendCallAccepted: to ", recipient);
    try {
        common::OutgoingMessage msg{common::MessageId::CallAccepted, phoneNumber, recipient};
        transport.sendMessage(msg.getMessage());
        logger.logDebug("CallAccepted message sent.");
    } catch (const std::exception& e) {
        logger.logError("Failed to send CallAccepted: ", e.what());
    }
}

void BtsPort::sendCallDropped(const common::PhoneNumber& recipient)
{
    logger.logDebug("sendCallDropped: to ", recipient);
    try {
        common::OutgoingMessage msg{common::MessageId::CallDropped, phoneNumber, recipient};
        transport.sendMessage(msg.getMessage());
        logger.logDebug("CallDropped message sent.");
    } catch (const std::exception& e) {
        logger.logError("Failed to send CallDropped: ", e.what());
    }
}

void BtsPort::sendCallTalk(const common::PhoneNumber& recipient, const std::string& text)
{
    logger.logDebug("sendCallTalk: to ", recipient, " text: ", text);
    try {
        common::OutgoingMessage msg{common::MessageId::CallTalk, phoneNumber, recipient};
        msg.writeText(text);
        transport.sendMessage(msg.getMessage());
        logger.logDebug("CallTalk message sent.");
    } catch (const std::exception& e) {
        logger.logError("Failed to send CallTalk: ", e.what());
    }
}

} // namespace ue
