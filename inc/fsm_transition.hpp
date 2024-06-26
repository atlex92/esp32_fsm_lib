#pragma once

#include <assert.h>
#include "fsm_state.hpp"
#include "fsm_event.hpp"

class EventFsmTransition {
public:
    EventFsmTransition(const FsmEvent event, FsmState* const from, FsmState* const to) : event_{event}, state_from_{from}, state_to_{to} {
        assert(from);
        assert(to);
        assert(from != to);
    }

    FsmState* stateFrom() const {
        return state_from_;
    }

    FsmState* stateTo() const {
        return state_to_;
    }

    FsmEvent event() const {
        return event_;
    }
private:
    static constexpr const char* const TAG{"EventFsmTransition"};
    FsmEvent event_{}; 
    FsmState* state_from_;
    FsmState* state_to_;
};
