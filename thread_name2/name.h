#include <thread>
#include <atomic>

namespace thname {
    thread_local std::atomic_int count{0};
}

bool threadSetName (std::string name);