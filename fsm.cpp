#include "fsm.hpp"
#include "esp_log.h"

void Fsm::setInternalTransitions(const std::vector<EventFsmTransition> transitions) {
    event_transitions_ = transitions;
}

void Fsm::run(void* data) {

    while(1) {
        std::vector<FsmEvent> incoming_events{};
        FsmEvent event_msg{};

        if(hasMessages()) {
            while(hasMessages()) {
                consumeMessage(event_msg, 0U);
                ESP_LOGI(TAG, "event group = %d, id = %d\r\n", (int)event_msg.event_group, (int)event_msg.event_id);
                incoming_events.push_back(event_msg);
            }
        } else {
            if(consumeMessage(event_msg, period_ms_)) {
                incoming_events.push_back(event_msg);
            }
        }

        for(const auto event : incoming_events) {
            handleEvent(event);
        }

        const auto new_events{process()};

        for(const auto event : new_events) {
            produceMessage(event);
        }

        delay(100);
    }
}


void Fsm::handleEvent(const FsmEvent event) {
    for(const auto& transition : event_transitions_) {
        if((transition.event() == event) && (transition.stateFrom() == current_state_)) {
            changeState(transition.stateTo());
            break;
        }
    }
}

void Fsm::changeState(FsmState* const new_state) {
    assert(new_state);
    current_state_->onExit();
    new_state->onEnter();
    current_state_ = new_state;
}

std::vector<FsmEvent> Fsm::process() {
    assert(current_state_);
    std::vector<FsmEvent> ret{};

    if(is_first_time_run_) {
        current_state_->onEnter();
        is_first_time_run_ = false;
    }
    return current_state_->onProcess();
}