#include "utils/signalReceiver.hpp"
#include <csignal>

std::map<int, std::function<void(int)> > SignalReceiver::m_userActions;


SignalReceiver* SignalReceiver::instance() {
  static SignalReceiver signalReceiver;
  return &signalReceiver;
}

void SignalReceiver::setSignal(int signalNumber, std::function<void(int)> userAction) {

  m_userActions[signalNumber] = userAction;

  // Add wrapper function to a single signal
  std::signal(signalNumber, SignalReceiver::cWrapper);
}

void SignalReceiver::setSignals(std::vector<int> signalNumbers, std::function<void(int)> userAction) {

  for (auto signalNumber : signalNumbers) {
    setSignal(signalNumber, userAction);
  }
}

void SignalReceiver::resetToDefault() {

  // For each user action reset the signal handler to default
  for (auto& action : m_userActions) {
    resetToDefault(action.first);
  }

}

void SignalReceiver::resetToDefault(int signalNumber) {
  std::signal(signalNumber, SIG_DFL);
}

SignalReceiver::SignalReceiver() {}
SignalReceiver::SignalReceiver(SignalReceiver& /*signalReceiver*/) {}
SignalReceiver::~SignalReceiver() {}

void SignalReceiver::cWrapper(int signalNumber) {

  // Call the user action if present.
  if (m_userActions.find(signalNumber) != m_userActions.end()) {
    m_userActions[signalNumber](signalNumber);
  }

}

