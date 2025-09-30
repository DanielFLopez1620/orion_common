#include "orion_control/orion_shared_state.hpp"

OrionDiffSharedState g_orion_diff_state{};
OrionForwardSharedState g_orion_forw_state{};
std::mutex g_orion_forward_allocation_mtx;