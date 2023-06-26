#pragma once

#include <thread>
#include <atomic>
#include <string>
#include <vector>

class Control;

class Subscriber
{
public:
    Subscriber(Control &control);
    ~Subscriber();

    void read();

private:
    Control &control_;
private:
    std::atomic<bool> reading_ = true;
    std::string message_;
    std::thread thread_;
private:
    std::vector<std::string> tokenise() const;
};

