#pragma once

#include "Messages/PhoneNumber.hpp"
#include "Data/SmsData.hpp"
#include <vector>
#include <string>
#include "IUeGui.hpp"

namespace ue
{

class IUserEventsHandler
{
public:
    virtual ~IUserEventsHandler() = default;
    virtual void handleUserAction(const std::string& id) = 0;
};

class IUserPort
{
public:
    virtual ~IUserPort() = default;

    virtual void showNotConnected() = 0;
    virtual void showConnecting() = 0;
    virtual void showConnected() = 0;
    virtual void showNewSms(bool present) = 0;
    virtual void displaySmsList(const std::vector<data::SmsData>& smsList) = 0;
    virtual void viewSms(const data::SmsData& sms) = 0;
    virtual void displaySmsCompose() = 0;
    // Added helper for ComposingSmsState
    virtual bool getComposedSmsData(common::PhoneNumber& recipient, std::string& text) = 0;

    virtual void showDialing() = 0;
    virtual bool getDialedNumber(common::PhoneNumber& recipient) = 0;
    virtual void showIncomingCall(const common::PhoneNumber& caller) = 0;
    virtual void showCalling(const common::PhoneNumber& callee) = 0;
    virtual IUeGui::ICallMode& showCallMode() = 0;
    virtual void showTalkingOverlay() = 0;


};

} // namespace ue
