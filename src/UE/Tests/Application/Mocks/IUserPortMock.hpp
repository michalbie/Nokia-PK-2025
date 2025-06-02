#pragma once

#include <gmock/gmock.h>
#include "Ports/IUserPort.hpp" // Includes IUserEventsHandler definition
#include <string>            // Include for handleUserAction parameter
#include <vector>            // Include for displaySmsList parameter
#include "Data/SmsData.hpp"  // Include for displaySmsList/viewSms parameters
#include "IUeGui.hpp"

namespace ue
{

class IUserEventsHandlerMock : public IUserEventsHandler
{
public:
    // Keep existing constructor/destructor declarations if defined in .cpp
    IUserEventsHandlerMock();
    ~IUserEventsHandlerMock() override;

    // Ensure MOCK_METHOD for handleUserAction is present
    MOCK_METHOD(void, handleUserAction, (const std::string& id), (override));
};

class IUserPortMock : public IUserPort
{
public:
    // Keep existing constructor/destructor declarations if defined in .cpp
    IUserPortMock();
    ~IUserPortMock() override;

    // Ensure all IUserPort methods are mocked
    MOCK_METHOD(void, showNotConnected, (), (override));
    MOCK_METHOD(void, showConnecting, (), (override));
    MOCK_METHOD(void, showConnected, (), (override));
    MOCK_METHOD(void, showNewSms, (bool present), (override));
    MOCK_METHOD(void, displaySmsList, (const std::vector<data::SmsData>& smsList), (override));
    MOCK_METHOD(void, viewSms, (const data::SmsData& sms), (override));
    MOCK_METHOD(void, displaySmsCompose, (), (override));
    MOCK_METHOD(bool, getComposedSmsData, (common::PhoneNumber& recipient, std::string& text), (override));
    MOCK_METHOD(void, showDialing, (), (override));
    MOCK_METHOD(bool, getDialedNumber, (common::PhoneNumber&), (override));
    MOCK_METHOD(void, showIncomingCall, (const common::PhoneNumber&), (override));
    MOCK_METHOD(void, showCalling, (const common::PhoneNumber&), (override));
    MOCK_METHOD(IUeGui::ICallMode&, showCallMode, (), (override));
    MOCK_METHOD(void, showTalkingOverlay, (), (override));

    
};

}
