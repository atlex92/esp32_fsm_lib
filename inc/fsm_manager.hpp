// #pragma once
// #include "fsm.hpp"
// #include "esp_log.h"

// class FsmManager {
// public:
//     void addFsm(Fsm* const fsm) {
//         if(fsms_.end() != std::find(fsms_.begin(), fsms_.end(), fsm)) {
//             ESP_LOGE(TAG, "fsm duplicate");
//         } else {
//             fsms_.push_back(fsm);
//         }
//     }

//     void process() {
//         std::vector<EventId> events{collectEvents()};
//         handleEvents(events);        
//     }
// private:
//     std::vector<EventId> collectEvents() {
//         std::vector<EventId> ret{};
//         for(auto fsm : fsms_) {
//             const auto new_events{fsm->process()};
//             ret.insert(ret.end(), new_events.begin(), new_events.end());
//         }
//         return ret;
//     }

//     void handleEvents(const std::vector<EventId>& events) {
//         for(auto fsm : fsms_) {
//             for(auto event : events) {
//                 fsm->handleEvent(event);
//             }
//         }
//     }
//     std::vector<Fsm*> fsms_;
//     static constexpr const char* const TAG{"FsmManager"};
// };