#pragma once

#include <vector>
#include <set>

#include "task.hpp"
#include "message_consumer.hpp"
#include "queue.hpp"
#include "esp_log.h"

#define SUBSCRIBER_QUEUE_LEN    10U

using EventId = uint16_t;
using Topic = uint16_t;

struct EventMsg {
    Topic topic;
    EventId event_id;
};

// template <typename TopicT, typename MessageT>
// bool operator==(const DispatcherMessage<TopicT, MessageT>& lhs, const DispatcherMessage<TopicT, MessageT>& rhs) {
//     return ((lhs.msg == rhs.msg) && (lhs.topic == rhs.topic));
// }

// template <typename TopicT, typename MessageT>
class MessageDispatcher : public Task, public MessageConsumer<EventMsg, SUBSCRIBER_QUEUE_LEN> {

using SubscriberQueue = Queue<EventId, SUBSCRIBER_QUEUE_LEN>*;

struct SubscriberSet {
    Topic topic;
    std::set<SubscriberQueue> subscribers;
};

public:
    MessageDispatcher() : Task{"MessageDispatcher", configMINIMAL_STACK_SIZE * 3, 5}{

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

    void handleIncommingMessage (const EventMsg& message) {
        SubscriberSet subset{subscriberSet(message.topic)};
        for(auto &sub : subset.subscribers) {
            const bool res{sub->enqueueBack(message.event_id)};
            /*  Should never fail! Otherwise will lead to race condition,
                when subs try to write to the full RX dispatcher's queue,
                and dispatcher can't handle subs, with the same reason
            */
            if(!res) {
                ESP_LOGE(TAG, "CRITICAL ERROR: failed to handle message: topic = %d", message.topic);
                assert(false);
            }
        }
    }
    void run(void* data) override {
        static EventMsg msg{};
        while (1) {
            consumeMessage(msg);
            handleIncommingMessage(msg);
        }
    }
    std::vector<SubscriberSet> subscribers_list_;
};