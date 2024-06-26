#pragma once

#include <vector>
#include <assert.h>
#include "fsm_state.hpp"
#include "fsm_transition.hpp"
#include "task.hpp"
#include "message_consumer.hpp"
#include "message_producer.hpp"

class Fsm : public Task, public MessageConsumer<FsmEvent, FSM_QUEUE_SIZE>, public MessageProducer<FsmEvent, FSM_QUEUE_SIZE> {
public:
    explicit Fsm(const char* const name, FsmState* const initial_state, const uint32_t period)
        : Task{name, configMINIMAL_STACK_SIZE * 5}, current_state_{initial_state}, period_ms_{period} {

    }

    void run(void* data) override;

    void setInternalTransitions(const std::vector<EventFsmTransition> transitions);
    std::vector<FsmEvent> process();
    void handleEvent(const FsmEvent event);

    const char* name() const {
        return name_;
    }
private:
    static constexpr const char* const TAG{"Fsm"};
    void changeState(FsmState* const new_state);
    FsmState* current_state_{};
    const char* const name_{};
    std::vector<EventFsmTransition> event_transitions_{};
    bool is_first_time_run_{true};
    uint32_t period_ms_{};
};
