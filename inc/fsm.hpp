#pragma once

#include <vector>
#include <assert.h>
#include "fsm_state.hpp"
#include "fsm_transition.hpp"
#include "task.hpp"
#include "message_consumer.hpp"
#include "message_producer.hpp"
#include "esp_log.h"

#define FSM_QUEUE_SIZE  10U

template <typename T>
class Fsm : public Task, public MessageConsumer<T, FSM_QUEUE_SIZE>, public MessageProducer<T, FSM_QUEUE_SIZE> {
public:
    explicit Fsm(const char* const name, FsmState<T>* const initial_state, const uint32_t period)
        : Task{name, configMINIMAL_STACK_SIZE * 5}, current_state_{initial_state}, period_ms_{period} {

    }

    void run(void* data) {
        while(1) {
            std::vector<T> incoming_events{};
            T event_msg{};

            if(MessageConsumer<T>::hasMessages()) {
                while(MessageConsumer<T>::hasMessages()) {
                    MessageConsumer<T>::consumeMessage(event_msg, 0U);
                    ESP_LOGI(TAG, "event group = %d, id = %d\r\n", (int)event_msg.event_group, (int)event_msg.event_id);
                    incoming_events.push_back(event_msg);
                }
            } else {
                if(MessageConsumer<T>::consumeMessage(event_msg, period_ms_)) {
                    incoming_events.push_back(event_msg);
                }
            }

            for(const auto event : incoming_events) {
                handleEvent(event);
            }

            const auto new_events{process()};

            for(const auto event : new_events) {
                MessageProducer<T>::produceMessage(event);
            }

            delay(100);
        }
    }

    void setInternalTransitions(const std::vector<EventFsmTransition<T>> transitions) {
        event_transitions_ = transitions;
    }
    
    std::vector<T> process() {
        assert(current_state_);
        std::vector<T> ret{};

        if(is_first_time_run_) {
            current_state_->onEnter();
            is_first_time_run_ = false;
        }
        return current_state_->onProcess();
    }

    void handleEvent(const T event) {
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

    void changeState(FsmState<T>* const new_state) {
        assert(new_state);
        current_state_->onExit();
        new_state->onEnter();
        current_state_ = new_state;
    }

    FsmState<T>* current_state_{};
    const char* const name_{};
    std::vector<EventFsmTransition<T>> event_transitions_{};
    bool is_first_time_run_{true};
    uint32_t period_ms_{};
};
