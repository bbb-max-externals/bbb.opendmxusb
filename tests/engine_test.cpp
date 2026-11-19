#include "engine.hpp"
#include <atomic>
#include <iostream>
#include <stdexcept>
using namespace std::chrono_literals;
static void check(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
struct FakeSerial {
    inline static std::mutex mutex;
    inline static std::vector<std::vector<uint8_t>> frames;
    inline static std::atomic<bool> fail{false};
    inline static std::atomic<int> handles{0};
    bool opened = false;
    bool open(const std::string& path, std::string& error) {
        if (path == "missing") { error = "missing device"; return false; }
        opened = true; ++handles; return true;
    }
    void close() { if (opened) { opened = false; --handles; } }
    bool send(const uint8_t* bytes, size_t size, std::string& error) {
        if (fail) { error = "unplugged"; return false; }
        std::lock_guard<std::mutex> lock(mutex);
        frames.emplace_back(bytes, bytes + size); return true;
    }
    static size_t count() { std::lock_guard<std::mutex> lock(mutex); return frames.size(); }
    static std::vector<uint8_t> last() { std::lock_guard<std::mutex> lock(mutex); return frames.back(); }
};
template<class F> static void until(F f) {
    const auto end = std::chrono::steady_clock::now() + 1s;
    do { if (f()) return; std::this_thread::sleep_for(1ms); } while (std::chrono::steady_clock::now() < end);
    throw std::runtime_error("timed out waiting for worker");
}
int main() try {
    {
        opendmx::Engine<FakeSerial> e([]{});
        auto update = [&](size_t offset, std::initializer_list<uint8_t> data, bool trigger) {
            e.update(offset, data.begin(), data.size(), trigger);
        };
        e.configure(1, 512, false, 44);
        update(0, {10, 20, 30}, false);
        update(511, {255, 128}, false); // must never overflow start code / frame
        e.connect("fake"); until([&] { return e.connected(); });
        check(FakeSerial::count() == 0, "bang mode transmitted without a bang");
        e.bang(); until([&] { return FakeSerial::count() == 1; });
        auto f = FakeSerial::last();
        check(f.size() == 513 && f[0] == 0 && f[1] == 10 && f[512] == 255, "DMX slot numbering/length corrupted");
        e.configure(1, 3, true, 44);
        std::this_thread::sleep_for(35ms);
        check(FakeSerial::count() == 1, "bang mode sent on blackout change");
        e.bang();
        until([&] { return FakeSerial::count() == 2; });
        check(FakeSerial::last() == std::vector<uint8_t>({0, 0, 0, 0}), "blackout must mask wire output");
        check(e.dump() == std::vector<uint8_t>({10, 20, 30}), "blackout destroyed stored levels");
        for (int mode : {0, 2, 3}) {
            const auto before = FakeSerial::count();
            e.configure(mode, 3, false, 44);
            until([&] { return FakeSerial::count() == before + 1; });
            check(FakeSerial::last() == std::vector<uint8_t>({0, 10, 20, 30}), "blackout release did not restore levels");
            e.configure(mode, 3, true, 44);
            until([&] { return FakeSerial::count() == before + 2; });
            check(FakeSerial::last() == std::vector<uint8_t>({0, 0, 0, 0}), "blackout change did not send zeros");
            e.configure(mode, 3, true, 44);
            std::this_thread::sleep_for(35ms);
            check(FakeSerial::count() == before + 2, "unchanged blackout resent a frame");
        }
        e.configure(3, 3, false, 44);
        until([&] { return FakeSerial::last() == std::vector<uint8_t>({0, 10, 20, 30}); });
        const auto baseline = FakeSerial::count();
        update(0, {11}, true);
        until([&] { return FakeSerial::count() == baseline + 1; });
        update(0, {11}, true);
        std::this_thread::sleep_for(35ms);
        check(FakeSerial::count() == baseline + 1, "change mode resent identical output");
        e.configure(4, 3, false, 0.01);
        e.configure(4, 3, false, 40);
        until([&] { return FakeSerial::count() >= baseline + 3; });
        check(FakeSerial::last() == std::vector<uint8_t>({0, 11, 20, 30}), "forced output did not restore data");
        FakeSerial::fail = true;
        until([&] { return !e.connected(); });
        auto report = e.report();
        check(std::any_of(report.events.begin(), report.events.end(), [](const auto& v) { return v.kind == "error"; }), "disconnect was not reported");
        FakeSerial::fail = false;
        e.connect("missing");
        until([&] { auto r = e.report(); return std::any_of(r.events.begin(), r.events.end(), [](const auto& v) { return v.detail.find("missing device") != std::string::npos; }); });
        e.connect("fake"); until([&] { return e.connected(); });
        e.connect(""); until([&] { return !e.connected() && FakeSerial::handles == 0; });
        const auto n = FakeSerial::count();
        e.bang(); update(0, {99}, true); std::this_thread::sleep_for(35ms);
        check(FakeSerial::count() == n, "closed port still transmitted");
    }
    check(FakeSerial::handles == 0, "worker leaked a serial port");
    std::cout << "PASS: frame bounds, modes, blackout, refresh, disconnect, reopen, close and shutdown\n";
    return 0;
} catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
