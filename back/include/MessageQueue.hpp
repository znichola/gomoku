#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

#define MQ MessageQueue::out()
#define MBQ(id, msg) MessageQueue::boardMessage((id), (msg))
#define MBL(layer, id, msg) MessageQueue::boardMessage((layer), (id), (msg))
#define COUT MessageQueue::ConsoleSink{}
#define ENABLE_LOG MessageQueue::enabled = true;
#define DISABLE_LOG MessageQueue::enabled = false;

namespace MessageQueue
{
    std::string json_escape(const std::string &s);

    inline bool enabled = true;

    inline std::vector<std::string> messages;

    inline void push(const std::string &msg) {
        if (!enabled) return;
        messages.push_back(json_escape(msg));
    }

    inline std::vector<std::string> drain() {
        auto out = std::move(messages);
        messages.clear();
        return out;
    }

    class ConsoleSink {
    public:
        template <typename T>
        ConsoleSink &operator<<(const T &v) {
            if (enabled) std::cout << v;
            return *this;
        }

        ConsoleSink &operator<<(std::ostream &(*manip)(std::ostream &)) {
            if (enabled) manip(std::cout);
            return *this;
        }
    };

    class Stream {
    public:
        template <typename T>
        Stream &operator<<(const T &v) {
            if (enabled) ss << v;
            return *this;
        }

        ~Stream() {
            if (enabled && !ss.str().empty())
                push(ss.str());
        }

        std::stringstream ss;
    };

    inline Stream out() {
        return Stream{};
    }

    inline std::string json_escape(const std::string &s) {
        std::string out;
        out.reserve(s.size());

        for (char c : s) {
            switch (c) {
            case '\"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                // control characters -> \u00XX
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[7];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += c;
                }
            }
        }

        return out;
    }

    inline void boardMessage(const std::string layer, unsigned id, const std::string &msg) {
        if (!enabled) return;
        std::stringstream ss;
        ss << "{\"layer\":\"" << json_escape(layer) << "\""
           << ",\"id\":" << id
           << ",\"msg\":\"" << json_escape(msg) << "\"}";
        push(ss.str());
    }

    inline void boardMessage(const std::string layer, unsigned id, float msg) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << msg;
        boardMessage(layer, id, oss.str());
    }

    inline void boardMessage(unsigned id, const std::string &msg) {
        boardMessage("default", id, msg);
    }

    inline void boardMessage(unsigned id, float msg) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << msg;
        boardMessage(id, oss.str());
    }

}