#ifndef ORION_SHARED_STATE_HPP
#define ORION_SHARED_STATE_HPP

#include <atomic>

struct OrionDiffSharedState
{
    std::atomic<int64_t> enc_left{0};
    std::atomic<int64_t> enc_right{0};
    std::atomic<int64_t> cmd_left{0};
    std::atomic<int64_t> cmd_right{0};
};

extern OrionDiffSharedState g_orion_diff_state;

#endif