#pragma once

#include <vector>
#include <assert.h>
#include "fsm_state.hpp"
#include "fsm_transition.hpp"
#include "task.hpp"
#include "message_consumer.hpp"
#include "message_producer.hpp"
#include "esp_log.h"
#include "message_dispatcher.hpp"

#define FSM_QUEUE_SIZE  10U

class Fsm : public Task, public MessageConsumer<EventId, FSM_QUEUE_SIZE>, public MessageProducer<EventMsg, FSM_QUEUE_SIZE> {
public:
    explicit Fsm(const char* const name, FsmState* const initial_state, const uint32_t period, const uint16_t stack_size = configMINIMAL_STACK_SIZE * 5)
        : Task{name, stack_size, 5}, current_state_{initial_state}, period_ms_{period} {

    }

    void run(void* data) {
        while(1) {
            std::vector<EventId> incoming_events{};
            EventId event_msg{};

            if(MessageConsumer::hasMessages()) {
                while(MessageConsumer::hasMessages()) {
                    MessageConsumer::consumeMessage(event_msg, 0U);
                    ESP_LOGD(TAG, "event receive, id = %d\r\n", (int)event_msg);
                    incoming_events.push_back(event_msg);
                }
            } else {
                if(MessageConsumer::consumeMessage(event_msg, period_ms_)) {
                    incoming_events.push_back(event_msg);
                }
            }

            for(const auto event : incoming_events) {
                handleEvent(event);
            }

            const auto new_events{process()};

            for(const auto event : new_events) {
                MessageProducer::produceMessage(event);
            }
        }
    }

    void setInternalTransitions(const std::vector<EventFsmTransition> transitions) {
        event_transitions_ = transitions;
    }
    
    std::vector<EventMsg> process() {
        assert(current_state_);
        std::vector<EventMsg> ret{};

        if(is_first_time_run_) {
            current_state_->onEnter();
            is_first_time_run_ = false;
        }
        return current_state_->onProcess();
    }

    void handleEvent(const EventId event) {
        for(const auto& transition : event_transitions_) {
            if((transition.event() == event) && (transition.stateFrom() == current_state_)) {
                changeState(transition.stateTo());
                break;
            }
        }
    }

    const char* name() const {
        return name_;
    }
private:
    static constexpr const char* const TAG{"Fsm"};

    void changeState(FsmState* const new_state) {
        assert(new_state);
        current_state_->onExit();
        new_state->onEnter();
        current_state_ = new_state;
        is_first_time_run_ = false;
    }

    FsmState* current_state_{};
    const char* const name_{};
    std::vector<EventFsmTransition> event_transitions_{};
    bool is_first_time_run_{true};
    uint32_t period_ms_{};
};
