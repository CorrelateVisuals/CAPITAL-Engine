#include "publisher.h"

  void Publisher::publish(const std::string& message, const std::string& value) {
        auto it = previous_.find(message);

        if (it != previous_.end() and it->second != value) 
        {
          std::cout << message << " = " << value << std::endl;
        }
        previous_[message] = value;
    }


 void Publisher::publish(const std::string& message) const {
        std::cout << message << std::endl;
    }