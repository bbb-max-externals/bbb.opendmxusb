#include "serial_mac.hpp"
#include <IOKit/serial/ioss.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glob.h>
#include <termios.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <chrono>
#include <thread>
namespace opendmx {
using namespace std::chrono_literals;
std::vector<std::string> serial_ports() {
    glob_t paths{};
    std::vector<std::string> result;
    if (glob("/dev/cu.*", 0, nullptr, &paths) == 0)
        for (size_t i = 0; i < paths.gl_pathc; ++i) result.emplace_back(paths.gl_pathv[i]);
    globfree(&paths);
    return result;
}
static bool failed(const char* operation, std::string& error) {
    error = std::string(operation) + ": " + std::strerror(errno);
    return false;
}
bool Serial::open(const std::string& path, std::string& error) {
    close();
    // Only callout serial device paths: never open an arbitrary file as a DMX port.
    if (path.rfind("/dev/cu.", 0) != 0 || path.find('/', 5) != std::string::npos) {
        error = "port must be a /dev/cu.* serial device";
        return false;
    }
    fd_ = ::open(path.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK | O_CLOEXEC | O_NOFOLLOW);
    if (fd_ < 0) return failed("open", error);
    struct stat st{};
    if (fstat(fd_, &st) < 0 || !S_ISCHR(st.st_mode)) {
        error = "port is not a character device"; close(); return false;
    }
    auto fail = [&](const char* op) { failed(op, error); close(); return false; };
    if (ioctl(fd_, TIOCEXCL) < 0) return fail("exclusive access");
    termios config{};
    if (tcgetattr(fd_, &config) < 0) return fail("tcgetattr");
    cfmakeraw(&config);
    config.c_cflag &= ~(CSIZE | PARENB | PARODD | CRTSCTS | CDTR_IFLOW | CDSR_OFLOW | CCAR_OFLOW);
    config.c_cflag |= CS8 | CSTOPB | CLOCAL | CREAD;
    config.c_iflag &= ~(IXON | IXOFF | IXANY);
    config.c_cc[VMIN] = 0;
    config.c_cc[VTIME] = 0;
    cfsetspeed(&config, B19200);
    if (tcsetattr(fd_, TCSANOW, &config) < 0) return fail("configure 8N2");
    speed_t baud = 250000;
    // Must follow tcsetattr: termios would overwrite the non-standard speed.
    if (ioctl(fd_, IOSSIOSPEED, &baud) < 0) return fail("set 250000 baud (IOSSIOSPEED)");
    if (tcflush(fd_, TCIOFLUSH) < 0) return fail("flush");
    int lines = TIOCM_RTS | TIOCM_DTR;
    if (ioctl(fd_, TIOCMBIC, &lines) < 0) return fail("clear RTS/DTR");
    return true;
}
void Serial::close() {
    if (fd_ >= 0) {
        ioctl(fd_, TIOCCBRK);
        ::close(fd_);
        fd_ = -1;
    }
}
bool Serial::send(const uint8_t* frame, size_t size, std::string& error) {
    // No tcsendbreak(): its ~250ms break would limit refresh to <4 Hz.
    if (ioctl(fd_, TIOCSBRK) < 0) return failed("break on", error);
    std::this_thread::sleep_for(200us);
    if (ioctl(fd_, TIOCCBRK) < 0) return failed("break off", error);
    std::this_thread::sleep_for(20us);
    const auto deadline = std::chrono::steady_clock::now() + 250ms;
    size_t sent = 0;
    while (sent < size) {
        const auto n = ::write(fd_, frame + sent, size - sent);
        if (n > 0) sent += static_cast<size_t>(n);
        else if (n < 0 && errno != EINTR && errno != EAGAIN) return failed("write", error);
        else std::this_thread::sleep_for(1ms);
        if (std::chrono::steady_clock::now() >= deadline) {
            error = "serial write timed out"; return false;
        }
    }
    // Bounded drain avoids tcdrain hanging Max shutdown on a stalled USB device.
    for (;;) {
        int pending = 0;
        if (ioctl(fd_, TIOCOUTQ, &pending) < 0) return failed("output queue", error);
        if (pending == 0) break;
        if (std::chrono::steady_clock::now() >= deadline) {
            error = "serial drain timed out"; return false;
        }
        std::this_thread::sleep_for(1ms);
    }
    // USB drivers may report an empty host queue before the UART is empty.
    // Allow a complete 8N2 frame (11 bits/slot) plus a 1ms USB guard before
    // acknowledging it or starting another break. This is intentionally conservative.
    std::this_thread::sleep_for(std::chrono::microseconds(size * 44) + 1ms);
    return true;
}
}
