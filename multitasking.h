#ifndef _MULTITASKING_H_
#define _MULTITASKING_H_

// TODO(chsnow): make this a separate compilation unit and not a giant #include hack.

// An ugly implementation of cooperative multitasking. The main loop yields by sleeping using 
// PeriodicEvent::Delay, giving us time to check if we should run any periodic callbacks.
//
// The bool return value of Delay is a signal to the MainLoop that some global state has changed, and 
// that it should re-parse its control code and restart its lighting program. For example, someone might
// have uploaded a new lighting program file via the HTTP server, or issued a command via the serial console.
// 
// Register a callback using SetHandler(...). We will try to call your callback every 'msec' milliseconds.
#define NUM_PERIODIC_EVENTS (10)

namespace PeriodicEvent {
  unsigned long msec_[NUM_PERIODIC_EVENTS];
  bool (*callback_[NUM_PERIODIC_EVENTS])();
  unsigned long next_[NUM_PERIODIC_EVENTS];
  unsigned long min_interval_ = 10000000;
  unsigned long last_check_;
  
  // If the callback returns true, this indicates some global state has changed and the main control loop should restart.
  void SetHandler(int pos, unsigned long msec, bool (*callback)()) {
    msec_[pos] = msec;
    callback_[pos] = callback;
    next_[pos] = millis() + msec;
    if (msec < min_interval_) {
      min_interval_ = msec;
      Serial.print("Min interval: ");
      Serial.println(min_interval_);
    }
  }
  
  // Time to run?
  bool Check() {
    bool state_change = false;
    unsigned long last_check_ = millis();
    for (int i = 0; i < NUM_PERIODIC_EVENTS; ++i) {
      if (next_[i] > 0 && next_[i] <= last_check_ && callback_[0]) {
        Serial.print("Calling interrupt: ");
        Serial.println(i);
        if ((*callback_[i])()) {
          state_change = true;
        }
        next_[i] = millis() + msec_[i];
      }
    }
    return state_change;
  }
  
  // Similar to builtin delay(msec), but handles collaboration while waiting. May return early if there was a global state change.
  bool Delay(unsigned long msec) {
    unsigned long sleep_remaining = msec;
    while (sleep_remaining > 0) {
      int sleep_time = (sleep_remaining > min_interval_) ? min_interval_ : sleep_remaining;
      sleep_remaining -= sleep_time;
      Serial.print("delaying: ");
      Serial.println(sleep_time);
      delay(sleep_time);
      if (millis() > last_check_ + min_interval_) { 
        if (Check()) { 
          return true;
        }
      }
    }
    return false;
  } 
}  // namespace PeriodicEvent

#endif  // _MULTITASKING_H_
