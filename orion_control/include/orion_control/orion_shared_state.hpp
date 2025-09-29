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
};

struct OrionForwardSharedState
{
    std::vector<ServoState> servos;

    OrionForwardSharedState(size_t n_servos = 2)
    {
        servos.resize(n_servos);
    }
};

extern OrionDiffSharedState g_orion_diff_state;
extern OrionForwardSharedState g_orion_forw_state;
extern std::mutex g_orion_forward_allocation_mtx;

#endif