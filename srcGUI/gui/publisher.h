#pragma once

#include <string>
#include <iostream>
#include <map>

class Publisher {
    public:
    Publisher() = default;
    ~Publisher() = default;

    void publish(const std::string& message, const std::string& value);
     void publish(const std::string& message) const;

private: 
    std::map<std::string, std::string> previous_;
};

