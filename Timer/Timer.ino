/*
 Repeat timer example

 This example shows how to use hardware timer in ESP32. The timer calls onTimer
 function every second. The timer can be stopped with button attached to PIN 0
 (IO0).

 This example code is in the public domain.
 */

// Stop button is attached to PIN 0 (IO0)
#define BTN_STOP_ALARM    0
#define LED_BUILTIN 4

hw_timer_t * timer0 = NULL;
volatile SemaphoreHandle_t timerSemaphore;

uint32_t isrCounter = 0;
uint32_t lastIsrAt = 0;

// Alarm to call onTimer function
void IRAM_ATTR onTimer(){
  // Increment the counter and set the time of ISR
  isrCounter++;
  lastIsrAt = millis();
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
  // It is safe to use digitalRead/Write here if you want to toggle an output

  // digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));     // Toggle the LED            
}

void setup() {
  Serial.begin(115200);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Set BTN_STOP_ALARM to input mode
  pinMode(BTN_STOP_ALARM, INPUT_PULLUP);

  // Create semaphore to inform us when the timer has fired
  timerSemaphore = xSemaphoreCreateBinary();

  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
  // info).
  timer0 = timerBegin(0, 80, true);

  // Attach onTimer function to our timer.
  timerAttachInterrupt(timer0, &onTimer, true);

  // Set alarm to call onTimer function every second (value in microseconds).
  // Repeat the alarm (third parameter)
  timerAlarmWrite(timer0, 1000000, true);

  // Start the alarm
  timerAlarmEnable(timer0);
}

void loop() {
  // If Timer has fired
  if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE){
    // Print it
    Serial.print("onTimer no. ");
    Serial.print(isrCounter);
    Serial.print(" at ");
    Serial.print(lastIsrAt);
    Serial.println(" ms");
  }
  // Stop timer if button is pressed
  if (digitalRead(BTN_STOP_ALARM) == LOW) {
    // If timer is still running
    if (timer0) {
      // Stop and free timer
      timerEnd(timer0);
      timer0 = NULL;
    }
  }
}
