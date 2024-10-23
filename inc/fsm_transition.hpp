#pragma once

#include <assert.h>
#include "fsm_state.hpp"
#include "message_dispatcher.hpp"

class EventFsmTransition {
public:
  EventFsmTransition(const EventId event, FsmState* const from, FsmState* const to)
  : event_{event}, state_from_{from}, state_to_{to} {
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

  EventId event() const {
    return event_;
  }

private:
  static constexpr const char* const TAG{"EventFsmTransition"};
  EventId event_{};
  FsmState* state_from_;
  FsmState* state_to_;
};
