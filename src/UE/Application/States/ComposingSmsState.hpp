#pragma once

#include "BaseState.hpp"
#include "ReceivingCallState.hpp"

namespace ue
{

class ComposingSmsState : public BaseState
{
public:
    ComposingSmsState(Context& context);

    void handleUserAction(const std::string& id) override;
    void handleCallRequest(common::PhoneNumber from) override;
};

}
