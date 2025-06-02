#pragma once

#include "IUeGui.hpp"
#include "IUserPort.hpp" // Includes IUserEventsHandler, defines IUserPort
#include "Logger/PrefixedLogger.hpp"
#include "IUeGui.hpp"           // Defines IUeGui and nested mode interfaces
#include "UeGui/IDialMode.hpp"
#include "Messages/PhoneNumber.hpp"
#include "Data/SmsData.hpp"
#include <vector>
#include <string>

namespace ue
{

class UserPort : public IUserPort
{
public:
    UserPort(common::ILogger& logger, IUeGui& gui, common::PhoneNumber phoneNumber);
    void start(IUserEventsHandler& handler);
    void stop();

    // IUserPort interface overrides
    void showNotConnected() override;
    void showConnecting() override;
    void showConnected() override;
    void showNewSms(bool present) override;
    void displaySmsList(const std::vector<data::SmsData>& smsList) override;
    void viewSms(const data::SmsData& sms) override;
    void displaySmsCompose() override;
    bool getComposedSmsData(common::PhoneNumber& recipient, std::string& text) override;

    void showDialing() override;                            // ADDED
    void showCalling(const common::PhoneNumber& callee) override;   //ADDED
    bool getDialedNumber(common::PhoneNumber& recipient) override; // ADDED
    void showIncomingCall(const common::PhoneNumber& caller) override; // ADDED
    IUeGui::ICallMode& showCallMode() override; // ADDED
    void showTalkingOverlay() override; // ADDED

private:
    common::PrefixedLogger logger;
    IUeGui& gui;
    common::PhoneNumber phoneNumber;
    IUserEventsHandler* handler = nullptr;
    // Pointer to current compose mode to retrieve data
    IUeGui::ISmsComposeMode* currentSmsComposeMode = nullptr;

    // Additional pointer for dialing mode
    IUeGui::IDialMode* currentDialMode = nullptr;
};

} // namespace ue
