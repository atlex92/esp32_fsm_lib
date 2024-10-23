#pragma once

#include <stdint.h>
#include <functional>
#include "message_dispatcher.hpp"

class FsmState {
  using OnEnterCallBack = std::function<void()>;
  using OnExitCallBack = std::function<void()>;
  using OnProcessCallBack = std::function<std::vector<TxEventMsg>()>;
  using OnEventCallback = std::function<void(const RxEventMsg& msg)>;

public:
  virtual ~FsmState() = default;
  explicit FsmState(const char* const name, OnEnterCallBack on_enter = nullptr, OnProcessCallBack on_process = nullptr,
                    OnExitCallBack on_exit = nullptr)
  : name_{name}, on_enter_cb_{on_enter}, on_process_cb_{on_process}, on_exit_cb_{on_exit} {
  }

  const char* name() const {
    return name_;
  }

  void onEnter() {
    if (nullptr != on_enter_cb_) {
      on_enter_cb_();
    }
  }

  std::vector<TxEventMsg> onProcess() {
    if (on_process_cb_) {
      return on_process_cb_();
    } else {
      return std::vector<TxEventMsg>{};
    }
  }

  void onExit() {
    if (nullptr != on_exit_cb_) {
      on_exit_cb_();
    }
  }

  void onEvent(const RxEventMsg& msg) {
    if (nullptr != on_event_cb_) {
      on_event_cb_(msg);
    }
  }

  void setOnEnterCb(OnEnterCallBack cb) {
    on_enter_cb_ = cb;
  }

  void setOnExitCb(OnExitCallBack cb) {
    on_exit_cb_ = cb;
  }

  void setOnProcessCb(OnProcessCallBack cb) {
    on_process_cb_ = cb;
  }

  void setOnEventCb(OnEventCallback cb) {
    on_event_cb_ = cb;
  }

private:
  static constexpr const char* const TAG{"FsmState"};
  const char* const name_{};

protected:
  OnEnterCallBack on_enter_cb_{};
  OnProcessCallBack on_process_cb_{};
  OnExitCallBack on_exit_cb_{};
  OnEventCallback on_event_cb_{};
};