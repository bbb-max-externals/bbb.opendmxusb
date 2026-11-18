#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
namespace opendmx {
std::vector<std::string> serial_ports();
// A transport belongs exclusively to the output worker.
class Serial {
public:
    ~Serial() { close(); }
    bool open(const std::string& path, std::string& error);
    void close();
    bool send(const uint8_t* frame, size_t size, std::string& error);
private:
    int fd_ = -1;
};
}
