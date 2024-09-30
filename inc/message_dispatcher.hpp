#pragma once

#include <vector>
#include <set>

#include "task.hpp"
#include "message_consumer.hpp"
#include "queue.hpp"
#include "esp_log.h"
#include "fsm_config.hpp"

using EventId = uint16_t;
using Topic = uint16_t;

union EventPayload {
    uint8_t byte;
    char ch;
    uint16_t ui16;
    uint32_t ui32;
    uint64_t ui64;
    int16_t i16;
    int32_t i32;
    int64_t i64;
    float f;
    const void* ptr;
};

struct RxEventMsg {
    EventId event_id;
    EventPayload payload;
};

struct TxEventMsg {
    Topic topic;
    RxEventMsg msg;
};
class MessageDispatcher : public Task, public MessageConsumer<TxEventMsg, DISPATCHER_MESSAGE_QUEUE_LEN> {

using SubscriberQueue = Queue<RxEventMsg, SUBSCRIBER_MESSAGE_QUEUE_LEN>*;

struct SubscriberSet {
    Topic topic;
    std::set<SubscriberQueue> subscribers;
};

public:
    MessageDispatcher() : Task{"MessageDispatcher", configMINIMAL_STACK_SIZE * 2, (configMAX_PRIORITIES - 1)}{

    }

    void addSubscriber(const Topic topic, const SubscriberQueue sub) {
        SubscriberSet& sublist{subscriberSet(topic)};
        sublist.subscribers.insert(sub);
    }
private:
    static constexpr const char* const TAG{"MessageDispatcher"};
    SubscriberSet& subscriberSet(const Topic topic) {
        for (auto &sublist : subscribers_list_) {
            if (sublist.topic == topic) {
                return sublist;
            }
        }
        
        SubscriberSet newSublist {
            .topic = topic,
            .subscribers = {}
        };

        subscribers_list_.push_back(newSublist);

        return subscribers_list_.back();
    }

    void handleIncommingMessage (const TxEventMsg& message) {
        SubscriberSet subset{subscriberSet(message.topic)};
        for(auto &sub : subset.subscribers) {
            const bool res{sub->enqueueBack(message.msg)};
            /*  Should never fail! Otherwise will lead to race condition,
                when subs try to write to the full RX dispatcher's queue,
                and dispatcher can't handle subs, with the same reason
            */
            if(!res) {
                ESP_LOGE(TAG, "CRITICAL ERROR: failed to handle message: topic = %d", message.topic);
                ESP_LOGE(TAG, "available space in queue: %u", sub->available());
                ESP_LOGE(TAG, "messages in queue: %u", sub->size());

                while(sub->size()) {
                    RxEventMsg msg{};
                    sub->receive(msg);
                    ESP_LOGE(TAG, "message: %u", msg.event_id);
                }
                
                assert(false);
            }
        }
    }
    void run(void* data) override {
        static TxEventMsg msg{};
        while (1) {
            consumeMessage(msg);
            handleIncommingMessage(msg);
        }
    }
    std::vector<SubscriberSet> subscribers_list_;
};