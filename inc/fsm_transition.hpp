#pragma once

#include <assert.h>
#include "fsm_state.hpp"

template <typename T>
class EventFsmTransition {
public:
    EventFsmTransition(const T event, FsmState<T>* const from, FsmState<T>* const to) : event_{event}, state_from_{from}, state_to_{to} {
        assert(from);
        assert(to);
        assert(from != to);
    }

    FsmState<T>* stateFrom() const {
        return state_from_;
    }

    FsmState<T>* stateTo() const {
        return state_to_;
    }

    T event() const {
        return event_;
    }
private:
    static constexpr const char* const TAG{"EventFsmTransition"};
    T event_{}; 
    FsmState<T>* state_from_;
    FsmState<T>* state_to_;
};
