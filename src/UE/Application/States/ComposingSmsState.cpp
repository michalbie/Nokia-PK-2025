#include "ComposingSmsState.hpp"
#include "ConnectedState.hpp" // Include for state transition
#include "Application.hpp"    // Include Application for storing/sending
#include "Ports/IBtsPort.hpp" // Include IBtsPort for sending
#include "Ports/IUserPort.hpp"// Include IUserPort for getting data
#include "ReceivingCallState.hpp"

namespace ue
{

ComposingSmsState::ComposingSmsState(Context& context)
    : BaseState(context, "ComposingSmsState")
{
    context.user.displaySmsCompose(); // Show the compose screen
}

void ComposingSmsState::handleUserAction(const std::string& id)
{
    logger.logDebug("Handling user action: ", id);
    if (id == "ACCEPT") // User pressed Send/Accept
    {
        logger.logInfo("User pressed Accept to send SMS.");
        common::PhoneNumber recipient;
        std::string text;
        // Retrieve data from UserPort (which queries the GUI)
        if (context.user.getComposedSmsData(recipient, text))
        {
            // Data retrieved successfully, send it via BtsPort
            context.bts.sendSms(recipient, text);
            // Store the sent message
            context.app.storeSentSms(recipient, text);
            // Transition back to Connected state
            context.setState<ConnectedState>();
        }
        else
        {
            // Error retrieving data (e.g., invalid number entered)
            logger.logError("Failed to retrieve valid SMS data from compose screen.");
            // Stay in this state, UserPort might show an error via GUI
        }
    }
    else if (id == "REJECT") // User pressed Cancel/Reject/Back
    {
        logger.logInfo("User pressed Reject, cancelling SMS compose.");
        // Transition back to Connected state without sending/saving
        context.setState<ConnectedState>();
    }
    else
    {
        logger.logInfo("Ignoring unexpected user action in this state: ", id);
    }
}

void ComposingSmsState::handleCallRequest(common::PhoneNumber from)
    {
        logger.logInfo("ComposingSmsState: incoming call from ", from);
        context.user.showIncomingCall(from);
        // Switch into ReceivingCallState
        context.setState<ReceivingCallState>(from);
    }

} 
