#ifndef PVTREE_UTILS_SIGNAL_RECEIVER_HPP
#define PVTREE_UTILS_SIGNAL_RECEIVER_HPP

/* @file
 * \brief Contains class to be used to capture
 *        system signals and then perform
 *        user defined actions.
 *
 * This is necessary to allow the use of lambda functions with
 * capture!
 */

#include <functional>
#include <map>
#include <vector>

/*! \brief Class which captures signals and
 *         performs user defined actions.
 *
 */
class SignalReceiver {
public:
  
  /*! \brief Mechanism to access signal receiver
   */
  static SignalReceiver* instance();

  /*! \brief Set the user action for a single signal
   */
  void setSignal(int signalNumber,                std::function<void(int)> userAction);

  /*! \brief Set the user action for a set of signals
   */
  void setSignals(std::vector<int> signalNumbers, std::function<void(int)> userAction);

  /*! \brief Change all the signal actions to the defaults
   */
  void resetToDefault();

  /*! \brief Change all the signal actions to the defaults
   */
  void resetToDefault(int signalNumber);
  
private:
  
  /*! \brief Create an singleton instance
   */
  SignalReceiver();
  SignalReceiver(SignalReceiver& signalReceiver);
  ~SignalReceiver();

  /*! \brief Functions defined by user which should be 
   *         called when a signal is received.
   *
   */
  static std::map<int, std::function<void(int)> > m_userActions;

  /*! \brief Function to wrap the C signal handling.
   *
   * params[in] signalNumber signal receieved by program.
   */
  static void cWrapper(int signalNumber);
};


#endif //PVTREE_UTILS_SIGNAL_RECEIVER_HPP
