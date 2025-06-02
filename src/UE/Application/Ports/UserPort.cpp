#include "UserPort.hpp"
#include "Application.hpp"
#include "UeGui/IListViewMode.hpp"
#include "UeGui/ITextMode.hpp"
#include "UeGui/ISmsComposeMode.hpp" // Include compose mode interface
#include "UeGui/IDialMode.hpp"
#include "IUeGui.hpp"
#include <sstream>
#include <vector>
#include "UeGui/ICallMode.hpp"

namespace ue
{

UserPort::UserPort(common::ILogger &logger, IUeGui &gui, common::PhoneNumber phoneNumber)
    : logger(logger, "[USER-PORT]"),
      gui(gui),
      phoneNumber(phoneNumber)
{}

void UserPort::start(IUserEventsHandler &handler_param)
{
    this->handler = &handler_param;
    gui.setTitle("Nokia " + common::to_string(phoneNumber));

    gui.setRejectCallback([this]() {
         if(this->handler) {
             logger.logDebug("Reject callback triggered");
             this->handler->handleUserAction("REJECT");
        } else {
             logger.logError("Reject callback triggered, but handler is null");
         }
    });
    gui.setAcceptCallback(nullptr);
}

void UserPort::stop()
{
    this->handler = nullptr;
}

void UserPort::showNotConnected()
{
    gui.setAcceptCallback(nullptr);
    gui.showNotConnected();
}

void UserPort::showConnecting()
{
    gui.setAcceptCallback(nullptr);
    gui.showConnecting();
}

void UserPort::showConnected()
{

    currentSmsComposeMode = nullptr; // Ensure compose mode ptr is null
    logger.logInfo("Showing Connected state main menu");
    IUeGui::IListViewMode& menu = gui.setListViewMode();
    menu.clearSelectionList();
    gui.setTitle("Nokia " + common::to_string(phoneNumber));

    const std::vector<std::pair<std::string, std::string>> items = {
        {"Compose SMS", "sms.compose"},
        {"View SMS", "sms.view"},
        {"Dial Call", "call.dial"}
    };

    for(const auto& item : items) {
        menu.addSelectionListItem(item.first, item.second);
    }

    gui.setAcceptCallback([this, &menu, items](){ // Capture menu by reference
        IUeGui::IListViewMode::OptionalSelection selection = menu.getCurrentItemIndex();
        if (selection.first && selection.second < items.size()) {
            const std::string& actionId = items[selection.second].second;
            logger.logDebug("Main menu Accept: item ", selection.second, ", action: ", actionId);
            if (this->handler) {
                this->handler->handleUserAction(actionId);
            } else {
                 logger.logError("Accept callback (main menu) triggered, but handler is null");
            }
        } else {
             logger.logInfo("Accept callback (main menu) triggered, but no valid item selected.");
        }
    });
    gui.showNewSms(false);
}

void UserPort::showNewSms(bool present)
{
    logger.logInfo("Setting new SMS indicator: ", (present ? "ON" : "OFF"));
    gui.showNewSms(present);
}

void UserPort::displaySmsList(const std::vector<data::SmsData>& smsList)
{
    currentSmsComposeMode = nullptr; // Ensure compose mode ptr is null
    logger.logInfo("Displaying SMS list with ", smsList.size(), " items.");
    IUeGui::IListViewMode& menu = gui.setListViewMode(); // Get reference
    menu.clearSelectionList();
    gui.setTitle("Received SMS");

    std::size_t listSize = smsList.size();

    if (smsList.empty()) {
         menu.addSelectionListItem("No SMS messages", "sms.list.empty");
         gui.setAcceptCallback(nullptr);
    } else {
        for (std::size_t i = 0; i < listSize; ++i) {
            const auto& sms = smsList[i];
            std::stringstream ss;
            ss << (sms.isRead ? "  " : "[N] ") << "From: " << common::to_string(sms.from);
            menu.addSelectionListItem(ss.str(), "");
        }

        // Capture menu by reference for use in lambda
        gui.setAcceptCallback([this, &menu, listSize](){
            IUeGui::IListViewMode::OptionalSelection selection = menu.getCurrentItemIndex();
            if (selection.first && selection.second < listSize) {
                std::string actionId = "sms.list.select." + std::to_string(selection.second);
                logger.logDebug("SMS List Accept: item ", selection.second, ", action: ", actionId);
                 if (this->handler) {
                    this->handler->handleUserAction(actionId);
                } else {
                    logger.logError("Accept callback (SMS list) triggered, but handler is null");
                }
            } else {
                logger.logInfo("Accept callback (SMS list) triggered, but no valid item selected.");
            }
        });
    }
}

void UserPort::viewSms(const data::SmsData& sms)
{
    currentSmsComposeMode = nullptr; // Ensure compose mode ptr is null
    logger.logInfo("Viewing SMS from: ", sms.from);
    IUeGui::ITextMode& view = gui.setViewTextMode();
    gui.setTitle("View SMS");
    std::stringstream ss;
    ss << "From: " << common::to_string(sms.from) << "\n\n";
    ss << sms.text;
    view.setText(ss.str());
    gui.setAcceptCallback(nullptr); // No specific accept action here
}

void UserPort::displaySmsCompose()
{
    logger.logInfo("Displaying SMS compose screen.");
    gui.setTitle("Compose SMS");
    // Get the compose mode interface and store pointer
    currentSmsComposeMode = &gui.setSmsComposeMode();
    currentSmsComposeMode->clearSmsText(); // Clear any previous text

    // Set Accept callback to handle sending
    gui.setAcceptCallback([this](){
        if (this->handler && this->currentSmsComposeMode) {
             // We don't retrieve data here, the state will do it via the pointer
             logger.logDebug("Compose SMS Accept triggered.");
             this->handler->handleUserAction("ACCEPT"); // State handles ACCEPT
        } else {
            logger.logError("Accept callback (compose SMS) triggered, but handler or mode is null");
        }
    });
    // Reject callback is already set globally to handle "Back/Cancel"
}

// Helper needed by ComposingSmsState to get data from the GUI mode
// This isn't ideal (breaks encapsulation slightly) but avoids complex callback data passing
// Returns true if data retrieval is successful
bool UserPort::getComposedSmsData(common::PhoneNumber& recipient, std::string& text)
{
     if (!currentSmsComposeMode) {
         logger.logError("Attempted to get composed SMS data, but compose mode is not active.");
         return false;
     }
     try {
          recipient = currentSmsComposeMode->getPhoneNumber();
          text = currentSmsComposeMode->getSmsText();
          logger.logInfo("Retrieved composed SMS to: ", recipient, ", text: '", text, "'");
          return true;
     } catch (const std::exception& e) {
          // IUeGui::ISmsComposeMode can throw if number is invalid
          logger.logError("Error retrieving composed SMS data: ", e.what());
          // Optionally: Display error to user via GUI? gui.setAlertMode().setText(e.what());
          return false;
     }
}

void UserPort::showDialing()
{
    gui.setAlertMode().setText("");
    {
        IUeGui::ICallMode& cm = gui.setCallMode();
        cm.clearIncomingText();
        cm.clearOutgoingText();
    }

    logger.logInfo("Switching UI to dialing mode.");
    // Ustawiamy tryb dialingu w GUI
    currentDialMode = &gui.setDialMode(); 

    // Nadpisujemy callbacki Accept i Reject na "ACCEPT" i "REJECT"
    gui.setAcceptCallback([this]()
    {
        if (this->handler)
        {
            logger.logDebug("Dialing Accept callback");
            this->handler->handleUserAction("ACCEPT");
        }
        else
        {
            logger.logError("Dialing Accept callback triggered, but handler is null");
        }
    });

    gui.setRejectCallback([this]()
    {
        if (this->handler)
        {
            logger.logDebug("Dialing Reject callback");
            this->handler->handleUserAction("REJECT");
        }
        else
        {
            logger.logError("Dialing Reject callback triggered, but handler is null");
        }
    });
}

bool UserPort::getDialedNumber(common::PhoneNumber& recipient)
{
    if (!currentDialMode) {
        logger.logError("Dial mode not active - cannot retrieve dialed number.");
        return false;
    }
    try {
        recipient = currentDialMode->getPhoneNumber();
        logger.logInfo("Retrieved dialed number: ", recipient);
        return true;
    } catch(const std::exception& e) {
        logger.logError("Error retrieving dialed number: ", e.what());
        return false;
    }
}

void UserPort::showIncomingCall(const common::PhoneNumber& caller)
{
    gui.setAlertMode().setText("");

    logger.logInfo("Switching UI to incoming call mode for caller: ", caller);
    IUeGui::ITextMode& alert = gui.setAlertMode();
    alert.setText("Incoming call from: " + common::to_string(caller));

    gui.setAcceptCallback([this]() {
        if (this->handler) {
            logger.logDebug("IncomingCall: ACCEPT");
            this->handler->handleUserAction("ACCEPT");
        } else {
            logger.logError("Accept callback (incoming call) but handler is null");
        }
    });
    gui.setRejectCallback([this]() {
        if (this->handler) {
            logger.logDebug("IncomingCall: REJECT");
            this->handler->handleUserAction("REJECT");
        } else {
            logger.logError("Reject callback (incoming call) but handler is null");
        }
    });
}

void UserPort::showCalling(const common::PhoneNumber& callee)
{
    gui.setAlertMode().setText("");

    logger.logInfo("Switching UI to outbound-calling mode for: ", callee);
    IUeGui::ITextMode& alert = gui.setAlertMode();
    alert.setText("Calling to: " + common::to_string(callee));

    // only wire “Reject”
    gui.setAcceptCallback(nullptr);

    gui.setRejectCallback([this]() {
        if (handler) {
            logger.logDebug("Outgoing call CANCEL pressed");
            handler->handleUserAction("REJECT");
        } else {
            logger.logError("Reject (during showCalling) but handler is null");
        }
    });
}

IUeGui::ICallMode& UserPort::showCallMode()
{
    logger.logInfo("Switching UI to call mode");
    IUeGui::ICallMode& mode = gui.setCallMode();

    gui.setAcceptCallback([this]() {
        if (handler) handler->handleUserAction("ACCEPT");
    });

    gui.setRejectCallback([this]() {
        if (handler) handler->handleUserAction("REJECT");
    });

    return mode;

}

void UserPort::showTalkingOverlay()
{
    gui.setAcceptCallback(nullptr);
    gui.setRejectCallback(nullptr);

    // color?
    IUeGui::ITextMode& alert = gui.setAlertMode();
    alert.setText("Talking");
}

} // namespace ue
