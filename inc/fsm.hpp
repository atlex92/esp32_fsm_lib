#pragma once

#include <assert.h>
#include <vector>
#include "esp_log.h"
#include "fsm_config.hpp"
#include "fsm_state.hpp"
#include "fsm_transition.hpp"
#include "message_consumer.hpp"
#include "message_dispatcher.hpp"
#include "message_producer.hpp"
#include "task.hpp"

class Fsm : public Task,
            public MessageConsumer<RxEventMsg, SUBSCRIBER_MESSAGE_QUEUE_LEN>,
            public MessageProducer<TxEventMsg, DISPATCHER_MESSAGE_QUEUE_LEN> {
public:
  explicit Fsm(const char* const name, FsmState* const initial_state, const uint32_t period, const uint8_t prio = 1,
               const uint16_t stack_size = configMINIMAL_STACK_SIZE * 2, const BaseType_t core_id = 0)
  : Task{name, stack_size, prio, core_id}, current_state_{initial_state}, name_{name}, period_ms_{period} {
  }

  void run(void* data) {
    while (1) {
      std::vector<RxEventMsg> incoming_events{};
      RxEventMsg event_msg{};

      if (MessageConsumer::hasMessages()) {
        while (MessageConsumer::hasMessages()) {
          MessageConsumer::consumeMessage(event_msg, 0U);
          ESP_LOGD(TAG, "%s - event receive, id = %d\r\n", name_, (int) event_msg.event_id);
          incoming_events.push_back(event_msg);
        }
      } else {
        if (MessageConsumer::consumeMessage(event_msg, period_ms_)) {
          incoming_events.push_back(event_msg);
        }
      }

      for (const auto event : incoming_events) {
        handleEvent(event);
      }

      const auto new_events{process()};

      for (const auto event : new_events) {
        MessageProducer::produceMessage(event);
      }
    }
  }

  void setInternalTransitions(const std::vector<EventFsmTransition> transitions) {
    event_transitions_ = transitions;
  }

  std::vector<TxEventMsg> process() {
    assert(current_state_);
    std::vector<TxEventMsg> ret{};

    if (is_first_time_run_) {
      current_state_->onEnter();
      is_first_time_run_ = false;
    }
    return current_state_->onProcess();
  }

  void handleEvent(const RxEventMsg event) {
    current_state_->onEvent(event);
    for (const auto& transition : event_transitions_) {
      if ((transition.event() == event.event_id) && (transition.stateFrom() == current_state_)) {
        changeState(transition.stateTo());
        break;
      }
    }
    last_message_ = event;
  }

  const char* name() const {
    return name_;
  }

protected:
  RxEventMsg lastMessage() const {
    return last_message_;
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
  const char* const name_{""};
  std::vector<EventFsmTransition> event_transitions_{};
  bool is_first_time_run_{true};
  uint32_t period_ms_{};
  RxEventMsg last_message_{};
};
