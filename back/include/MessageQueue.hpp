#pragma once

#include <vector>
#include <string>
#include <sstream>

#define MQ MessageQueue::out()
#define MBQ(id, msg) MessageQueue::boardMessage((id), (msg))

namespace MessageQueue
{
    std::string json_escape(const std::string &s);

    inline std::vector<std::string> messages;

    inline void push(const std::string &msg) {
        messages.push_back(json_escape(msg));
    }

    inline std::vector<std::string> drain() {
        auto out = std::move(messages);
        messages.clear();
        return out;
    }

    class Stream {
    public:
        template <typename T>
        Stream &operator<<(const T &v) {
            ss << v;
            return *this;
        }

        ~Stream() {
            if (!ss.str().empty())
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
                }
                else {
                    out += c;
                }
            }
        }

        return out;
    }

    inline void boardMessage(unsigned id, const std::string &msg) {
        std::stringstream ss;
        ss << "{\"id\":" << id
        << ",\"msg\":\"" << json_escape(msg) << "\"}";
        push(ss.str());
    }
}