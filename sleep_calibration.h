// This code is designed to self-calibrate the delays used in the 
// G-35's proprietary serial bus protocol. I'm not actually using 
// it at the moment, though...
//
// TODO(chsnow): clean this up and start using it.
/*
#define ITERATIONS (100)
unsigned long RunEmptyCalibrationLoop() {
  unsigned long start, end, total;
  total = 0;
  for (int i = 0; i < ITERATIONS; ++i) { 
    start = micros();
    end = micros();
    total += end-start;
  }
  return total;
}
unsigned long RunCalibrationLoop(int time_us) {
  unsigned long start, end, total;
  total = 0;
  for (int i = 0; i < ITERATIONS; ++i) { 
    start = micros();
    DELAY_US(time_us);
    digitalWrite(led_pin, LOW);
    end = micros();
    total += end-start;
  }
  return total;
}

int CalibrateSleep(int desired_us) {
  int calibrated_us = desired_us;
  unsigned long target_total = ITERATIONS * desired_us;
  unsigned long noop_time = RunEmptyCalibrationLoop();
  unsigned long first = RunCalibrationLoop(calibrated_us) - noop_time;
  int step;
  unsigned long delta = abs(first - target_total);
  if (first < target_total) {
    step = +1;
  } else {
    step = -1;
  }
  bool done = false;
  while (!done) {
    int try_us = calibrated_us + step;
    unsigned long new_total = RunCalibrationLoop(try_us) - noop_time;
    unsigned long new_delta = abs(new_total - target_total);
    if (new_delta < delta) {
      delta = new_delta;
      calibrated_us = try_us;
      done = false;
    } else {
      done = true;
    }
  }
  
  Serial.print("Calibrated ");
  Serial.print(desired_us);
  Serial.print(" to ");
  Serial.println(calibrated_us);
  return calibrated_us;
}

*/
