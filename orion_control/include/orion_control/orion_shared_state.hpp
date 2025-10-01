#ifndef ORION_SHARED_STATE_HPP
#define ORION_SHARED_STATE_HPP

#include <atomic>
#include <vector>
#include <mutex>
#include <memory>

// ---------------- Diff state ----------------
struct OrionDiffSharedState
{
    std::atomic<int64_t> enc_left{0};
    std::atomic<int64_t> enc_right{0};
    std::atomic<int64_t> cmd_left{0};
    std::atomic<int64_t> cmd_right{0};
};

// ---------------- Servo state ----------------
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

// ---------------- Forward state ----------------
struct OrionForwardSharedState
{
    std::vector<ServoState> servos;

    OrionForwardSharedState(size_t n_servos = 2)
    {
        servos.reserve(n_servos);
        for (size_t i = 0; i < n_servos; ++i)
        {
            servos.emplace_back();
        }
    }
};

// ---------------- Accessors ----------------
// Use inline to avoid needing a .cpp definition
inline std::shared_ptr<OrionDiffSharedState> get_orion_diff_state()
{
    static std::shared_ptr<OrionDiffSharedState> instance =
        std::make_shared<OrionDiffSharedState>();
    return instance;
}

inline std::shared_ptr<OrionForwardSharedState> get_orion_forward_state(size_t n_servos = 2)
{
    static std::shared_ptr<OrionForwardSharedState> instance =
        std::make_shared<OrionForwardSharedState>(n_servos);
    return instance;
}

inline std::shared_ptr<std::mutex> get_orion_forward_allocation_mtx()
{
    static std::shared_ptr<std::mutex> mtx = std::make_shared<std::mutex>();
    return mtx;
}

#endif // ORION_SHARED_STATE_HPP
