#ifndef UTIL_HPP
#define UTIL_HPP
#include <iostream>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

class Util {
    public:
    template<typename NumberType, typename Distribution>
    static NumberType random_number(Distribution dist) {
        thread_local static boost::random::mt19937 gen{static_cast<unsigned int>(std::time(0))};
        return dist(gen);
    }

    // Convenience function specifically for random message IDs
    static uint32_t random_message_id() {
        thread_local static boost::random::uniform_int_distribution<uint32_t> dist(1, 4294967295);
        return random_number<uint32_t>(dist);
    }

    static uint32_t random_position() {
        thread_local static boost::random::uniform_int_distribution<uint32_t> dist(0, 100);
        return random_number<uint32_t>(dist);
    }

    // static uint32_t random_message_id() {
    //     // Thread-local static ensures that each thread gets its own generator.
    //     // Initialized once per thread and avoids the need for external synchronization.
    //     thread_local static boost::random::mt19937 gen{static_cast<unsigned int>(std::time(0))};
    //     thread_local static boost::random::uniform_int_distribution<uint32_t> dist(1, 4294967295);
    //     return dist(gen);
    // }

    template<typename T>
    const static T& clamp(const T& v, const T& lo, const T& hi) {
        return (v < lo)? lo : (hi < v) ? hi : v;
    }
};

#endif