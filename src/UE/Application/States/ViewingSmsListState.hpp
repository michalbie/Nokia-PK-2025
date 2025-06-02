#pragma once

#include "BaseState.hpp"
#include "ReceivingCallState.hpp"

namespace ue
{

class ViewingSmsListState : public BaseState
{
public:
    ViewingSmsListState(Context& context);

    void handleUserAction(const std::string& id) override;
    void handleCallRequest(common::PhoneNumber from) override;
    // CORRECTED: Add handleSms override declaration
    void handleSms(const common::PhoneNumber& from, const std::string& text) override;
};

}
