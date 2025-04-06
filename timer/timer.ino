#include <B31DGMonitor.h>

#include <Ticker.h>

Ticker ticker;
B31DGCyclicExecutiveMonitor monitor;
typedef void (*TaskFunction)(); //write this at the start of your code
//==========================
// Pin Assignments
//==========================
// Task 1: Digital Signal 1 output pin
const int digSig1Pin = 14;  
// Task 2: Digital Signal 2 output pin
const int digSig2Pin = 15;  

// Task 3: Frequency measurement F1 input pin
const int F1Pin = 27;   
volatile unsigned int freq1 = 0;    
// Task 4: Frequency measurement F2 input pin
const int F2Pin = 25;   
volatile unsigned int freq2 = 0;     

// Task 6: LED indicator for frequency threshold
const int LEDIndicatorPin = 18;  
// Task 7: LED toggled by button press
const int toggleLedPin = 19;     
// Task 7: Push button input pin (using internal pull-up)
const int pushButtonPin = 32;    

//==========================
// Variables for button debounce
//==========================
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // 50 ms debounce delay
int lastButtonState = HIGH;
int buttonState = HIGH;
int currentToggleState = LOW; // current state of the toggle LED

unsigned int measureFrequency(int pin, unsigned long timeout) {
  unsigned long highTime = pulseIn(pin, HIGH, timeout);
  if (highTime == 0) {
    return 0;  // Timeout or no pulse detected
  }
  unsigned long period = 2 * highTime;
  unsigned int frequency = 1000000UL / period;
  return frequency;
}

void JobTask1() {
  monitor.jobStarted(1);
  digitalWrite(digSig1Pin, HIGH);
  delayMicroseconds(250);
  digitalWrite(digSig1Pin, LOW);
  delayMicroseconds(50);
  digitalWrite(digSig1Pin, HIGH);
  delayMicroseconds(300);
  digitalWrite(digSig1Pin, LOW);

  monitor.jobEnded(1);
}

void JobTask2() {
  monitor.jobStarted(2);
  digitalWrite(digSig2Pin, HIGH);
  delayMicroseconds(100);
  digitalWrite(digSig2Pin, LOW);
  delayMicroseconds(50);
  digitalWrite(digSig2Pin, HIGH);
  delayMicroseconds(200);
  digitalWrite(digSig2Pin, LOW);

  monitor.jobEnded(2);
}

void JobTask3() {
  monitor.jobStarted(3);
  freq1 = measureFrequency(F1Pin, 765);
  monitor.jobEnded(3);
}

void JobTask4() {
  monitor.jobStarted(4);
  freq2 = measureFrequency(F2Pin, 615);
  monitor.jobEnded(4);
}

void JobTask5() {
  monitor.jobStarted(5);

  monitor.doWork();

  monitor.jobEnded(5);
}

void JobTask6() {
  unsigned int freqSum = freq1 + freq2;
  if (freqSum > 1500)
    digitalWrite(LEDIndicatorPin, HIGH);
  else
    digitalWrite(LEDIndicatorPin, LOW);
}

// void JobTask7() {
//   int reading = digitalRead(pushButtonPin);
  
//   // If the reading has changed, reset the debounce timer
//   if (reading != lastButtonState) {
//     lastDebounceTime = millis();
//   }
  
//   // If the reading is stable for longer than debounceDelay, consider it valid
//   if ((millis() - lastDebounceTime) > debounceDelay) {
//     if (reading != buttonState) {
//       buttonState = reading;
//       // When button goes from HIGH to LOW (pressed)
//       if (buttonState == LOW) {
//         currentToggleState = !currentToggleState;  // Toggle LED state
//         digitalWrite(toggleLedPin, currentToggleState);
//         monitor.doWork();  // Call the monitoring library function
//         Serial.println("Button pressed: Toggle LED changed, doWork() called.");
//       }
//     }
//   }
  
//   lastButtonState = reading;
// }

void setup() {

  Serial.begin(115200);
  while (!Serial);  // Wait for serial connection
  
  // Set pin modes for digital signal outputs
  pinMode(digSig1Pin, OUTPUT);
  pinMode(digSig2Pin, OUTPUT);
  
  // Set pin modes for frequency measurement (inputs)
  pinMode(F1Pin, INPUT);
  pinMode(F2Pin, INPUT);
  
  // Set pin modes for LEDs
  pinMode(LEDIndicatorPin, OUTPUT);
  pinMode(toggleLedPin, OUTPUT);
  
  // Set button pin as input with internal pull-up resistor
  pinMode(pushButtonPin, INPUT_PULLUP);
  
  monitor.startMonitoring();
}


TaskFunction taskList[] = {JobTask1, JobTask2, JobTask3,
  JobTask4, JobTask5};
const int numTasks = sizeof(taskList) / sizeof(taskList[0]);

void loop() 
{
  // Loop over each function in the taskList array.
  for (int taskIndex = 0; taskIndex < numTasks; taskIndex++) {
    // Record the start time in microseconds.
    unsigned long startTime = micros();

    // Call the current function 1000 times.
    for (int i = 0; i < 1000; i++) {
      taskList[taskIndex]();
    }
    
    // Calculate how long it took to execute 1000 calls.
    float duration = micros() - startTime;
    
    // Output the result.
    Serial.print("Duration for function ");
    Serial.print(taskIndex);
    Serial.print(" = ");
    Serial.println(duration/1000);
  }
  
  // Exit the loop once done.
  exit(0);
}
