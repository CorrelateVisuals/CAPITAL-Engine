#include "subscriber.h"
#include "Control.h"

#include <iostream>
#include <sstream>

Subscriber::Subscriber(Control &control)
    : control_(control), message_{""}, thread_{[this] { this->read(); }} {}

Subscriber::~Subscriber() {
  reading_.store(false);
  thread_.join();
}

void Subscriber::read() {

  while (reading_) {
    std::getline(std::cin, message_);
    std::cout << "Subsriber: " << message_ << std::endl;

    auto tkns = tokenise();
    reading_ = tkns[0] != "quit";
    if (reading_) {
      if (tkns[0] == "speed") {
        control_.timer.speed = atof(tkns[2].c_str());
      }
    }
  }
  std::cout << "Subsriber: stopped\n";
}

std::vector<std::string> Subscriber::tokenise() const {
  std::vector<std::string> tkns;
  std::stringstream strs{message_};

  std::string token;
  while (std::getline(strs, token, ' '))
    tkns.push_back(token);

  return tkns;
}
