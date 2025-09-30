#ifndef ORION_SHARED_STATE_HPP
#define ORION_SHARED_STATE_HPP

#include <atomic>
#include <vector>
#include <mutex>

struct OrionDiffSharedState
{
    std::atomic<int64_t> enc_left{0};
    std::atomic<int64_t> enc_right{0};
    std::atomic<int64_t> cmd_left{0};
    std::atomic<int64_t> cmd_right{0};
};

struct ServoState
{
    std::atomic<double> pos{0.0};
    std::atomic<double> cmd{0.0};

    ServoState() = default;
    ServoState(double p, double c) : pos(p), cmd(c) {}
    ServoState(const ServoState&) = delete;
    ServoState& operator=(const ServoState&) = delete;

    ServoState(ServoState&& other) noexcept
    {
        pos.store(other.pos.load(std::memory_order_relaxed));
        cmd.store(other.cmd.load(std::memory_order_relaxed));
    }

    ServoState& operator=(ServoState&& other) noexcept
    {
        if (this != &other)
        {
            pos.store(other.pos.load(std::memory_order_relaxed));
            cmd.store(other.cmd.load(std::memory_order_relaxed));
        }
        return *this;
    }

};

struct OrionForwardSharedState
{
    std::vector<ServoState> servos;

    OrionForwardSharedState(size_t n_servos = 2)
    {
        servos.reserve(n_servos);
        for(size_t i=0; i < n_servos; ++i)
        {
            servos.emplace_back();
        }
    }
};

extern OrionDiffSharedState g_orion_diff_state;
extern OrionForwardSharedState g_orion_forw_state;
extern std::mutex g_orion_forward_allocation_mtx;

#endif