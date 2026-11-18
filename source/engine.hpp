#pragma once
#include <algorithm>
#include <array>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>
namespace opendmx {
struct Frame {
    std::array<uint8_t, 513> bytes{}; // slot zero is the DMX start code
    size_t size = 513;
    bool operator==(const Frame& other) const {
        return size == other.size && std::equal(bytes.begin(), bytes.begin() + size, other.bytes.begin());
    }
};
struct Event { std::string kind, detail; };
struct Report { std::vector<Event> events; uint64_t sent = 0; };
// Transport is injectable so timing, disconnect and shutdown can be exercised
// without sending arbitrary levels to a lighting rig.
template<class Transport> class Engine {
public:
    explicit Engine(std::function<void()> notify) : notify_(std::move(notify)), worker_([this] { run(); }) {}
    ~Engine() {
        { std::lock_guard<std::mutex> lock(mutex_); stop_ = true; }
        cv_.notify_one();
        worker_.join();
    }
    void connect(std::string path) {
        std::lock_guard<std::mutex> lock(mutex_);
        desired_ = std::move(path); ++generation_; pending_.reset(); previous_.reset();
        cv_.notify_one();
    }
    void configure(int mode, int channels, bool blackout, double rate) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (mode_ != mode) pending_.reset();
        mode_ = mode; channels_ = channels; blackout_ = blackout; rate_ = rate;
        cv_.notify_one();
    }
    void update(size_t offset, const std::vector<uint8_t>& values, bool trigger) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (offset >= 512) return;
        std::copy_n(values.begin(), std::min(values.size(), 512 - offset), data_.begin() + offset);
        if (trigger && (mode_ == 0 || mode_ == 2 || mode_ == 3)) {
            auto f = frame();
            if (mode_ != 3 || !previous_ || !(f == *previous_)) request(f);
        }
    }
    void bang() { std::lock_guard<std::mutex> lock(mutex_); request(frame()); }
    std::vector<uint8_t> dump() {
        std::lock_guard<std::mutex> lock(mutex_);
        return {data_.begin(), data_.begin() + channels_};
    }
    bool connected() { std::lock_guard<std::mutex> lock(mutex_); return connected_; }
    Report report() {
        std::lock_guard<std::mutex> lock(mutex_);
        Report r; std::swap(r, report_); return r;
    }
private:
    Frame frame() const {
        Frame f; f.size = static_cast<size_t>(channels_) + 1;
        if (!blackout_) std::copy_n(data_.begin(), channels_, f.bytes.begin() + 1);
        return f;
    }
    void request(const Frame& f) {
        // Requests while closed are deliberately discarded; stored levels survive.
        if (desired_.empty()) return;
        pending_ = f; previous_ = f; cv_.notify_one();
    }
    void event(std::string kind, std::string detail) {
        if (report_.events.size() == 32) report_.events.erase(report_.events.begin());
        report_.events.push_back({std::move(kind), std::move(detail)});
        notify_(); // only schedules Max qelem; never calls outlets here
    }
    void run() {
        using Clock = std::chrono::steady_clock;
        Transport transport;
        std::unique_lock<std::mutex> lock(mutex_);
        uint64_t applied = 0;
        auto last_started = Clock::now() - std::chrono::seconds(100);
        while (!stop_) {
            if (applied != generation_) {
                const auto generation = generation_;
                const auto path = desired_;
                const bool was_connected = connected_;
                connected_ = false;
                lock.unlock();
                transport.close();
                std::string error;
                const bool opened = !path.empty() && transport.open(path, error);
                lock.lock();
                applied = generation;
                if (was_connected) event("closed", "");
                if (opened) {
                    connected_ = true;
                    event("opened", path);
                } else if (!path.empty()) {
                    pending_.reset(); previous_.reset();
                    event("error", path + ": " + error);
                }
                last_started = Clock::now() - std::chrono::seconds(100);
                continue; // apply a newer port request before transmitting
            }
            if (!connected_ || (mode_ != 4 && !pending_)) {
                cv_.wait(lock, [&] { return stop_ || applied != generation_ || (connected_ && (mode_ == 4 || pending_)); });
                continue;
            }
            const auto interval = std::chrono::duration<double>(1.0 / rate_);
            const auto next = last_started + std::chrono::duration_cast<Clock::duration>(interval);
            if (Clock::now() < next) { cv_.wait_until(lock, next); continue; }
            const Frame outgoing = pending_ ? *pending_ : frame();
            pending_.reset();
            last_started = Clock::now();
            lock.unlock();
            std::string error;
            const bool ok = transport.send(outgoing.bytes.data(), outgoing.size, error);
            if (!ok) transport.close();
            lock.lock();
            if (ok) { ++report_.sent; notify_(); }
            else {
                connected_ = false; pending_.reset(); previous_.reset();
                event("error", error); event("closed", "");
            }
        }
        lock.unlock();
        transport.close();
    }
    std::function<void()> notify_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::array<uint8_t, 512> data_{};
    int mode_ = 4, channels_ = 512;
    bool blackout_ = false, stop_ = false, connected_ = false;
    double rate_ = 40;
    std::string desired_;
    uint64_t generation_ = 0;
    std::optional<Frame> pending_, previous_;
    Report report_;
    std::thread worker_;
};
}
