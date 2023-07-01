#pragma once

#include <thread>
#include <atomic>
#include <string>
#include <vector>
#include <iostream>

class Control;

class Subscriber
{
public:
    Subscriber(Control &control);
    ~Subscriber();

    static bool start_stop() { return start_stop_; }

public:
    static void waitfor(const std::string &message);
public:
    void read();

private:
    Control &control_;
private:
    std::atomic<bool> reading_ = true;
    static std::atomic<bool> start_stop_;
    std::string message_;
    std::thread thread_;
private:
    std::vector<std::string> tokenise() const;
};

