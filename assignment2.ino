#include <Ticker.h>
#include <B31DGMonitor.h>

Ticker ticker;
B31DGCyclicExecutiveMonitor monitor;

unsigned long frameCounter = 0;

//==========================pins
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
const int LEDIndicatorPin = 16;  
// Task 7: LED toggled by button press 
const int toggleLedPin = 19;     
// Task 7: Push button input pin 
const int pushButtonPin = 32;    

//button debounce
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; 
int lastButtonState = HIGH;
int buttonState = HIGH;
int currentToggleState = LOW;

// use pulseIn() measure frequency
unsigned int measureFrequency(int pin, unsigned long timeout) {
  unsigned long highTime = pulseIn(pin, HIGH, timeout);
  if (highTime == 0) return 0;
  unsigned long period = 2 * highTime; 
  unsigned int frequency = 1000000UL / period;
  return frequency;
}

//--------------------------
// Task 1: Output Digital Signal 1
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

// Task 2: Output Digital Signal 2
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

// Task 3: Measure frequency F1
void JobTask3() {
  monitor.jobStarted(3);
  freq1 = measureFrequency(F1Pin, 1200);
  monitor.jobEnded(3);
}

// Task 4: Measure frequency F2
void JobTask4() {
  monitor.jobStarted(4);
  freq2 = measureFrequency(F2Pin, 1000);
  monitor.jobEnded(4);
}

// Task 5: Work simulation (e.g., call doWork)
void JobTask5() {
  monitor.jobStarted(5);
  monitor.doWork();
  monitor.jobEnded(5);
}

// Task 6: LED indicator control: if (freq1 + freq2) > 1500, LED ON; else OFF.
void JobTask6() {
  unsigned int freqSum = freq1 + freq2;
  if (freqSum > 1500)
    digitalWrite(LEDIndicatorPin, HIGH);
  else
    digitalWrite(LEDIndicatorPin, LOW);
}

// Task 7: Button monitoring and toggle LED
void JobTask7() {
  int reading = digitalRead(pushButtonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) {  // Button pressed
        currentToggleState = !currentToggleState;
        digitalWrite(toggleLedPin, currentToggleState);
        monitor.doWork();
        Serial.println("Button pressed: Toggle LED changed, doWork() called.");
      }
    }
  }
  lastButtonState = reading;
}

//==========================
// Slot Scheduling，2ms，in total 60ms superperiod，switch-case
void frame() {
  unsigned int slot = frameCounter % 30;
  switch (slot) {
    case 0:  JobTask5(); JobTask2();  break;
    case 1:  JobTask4(); JobTask1();  break;
    case 2:  JobTask3(); JobTask2();  break;
    case 3:  JobTask5(); JobTask1();  break;
    case 4:  JobTask2(); JobTask1();  break;
    case 5:  JobTask2(); JobTask4();  break;
    case 6:  JobTask2(); JobTask5();  break;
    case 7:  JobTask3(); JobTask1();  break;
    case 8:  JobTask2(); JobTask5();  break;
    case 9:  JobTask1(); JobTask2();  break;
    case 10: JobTask5(); JobTask3();  break;
    case 11: JobTask2(); JobTask1();  break;
    case 12: JobTask4();              break;
    case 13: JobTask1(); JobTask2();  break;
    case 14: JobTask5(); JobTask2();  break;
    case 15: JobTask2(); JobTask1();  break;
    case 16: JobTask4(); JobTask3();  break;
    case 17: JobTask5(); JobTask2(); JobTask1();  break;
    case 18: JobTask5();              break;
    case 19: JobTask2(); JobTask1();  break;
    case 20: JobTask5(); JobTask2();  break;
    case 21: JobTask2(); JobTask1();  break;
    case 22: JobTask3(); JobTask1();  break;
    case 23: JobTask2(); JobTask4();  break;
    case 24: JobTask2(); JobTask5();  break;
    case 25: JobTask1(); JobTask3();  break;
    case 26: JobTask2(); JobTask5();  break;
    case 27: JobTask4(); JobTask1();  break;
    case 28: JobTask5(); JobTask2();  break;
    case 29: JobTask2(); JobTask1();  break;
    default: break;
  }
  frameCounter++;
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  pinMode(digSig1Pin, OUTPUT);
  pinMode(digSig2Pin, OUTPUT);
  pinMode(F1Pin, INPUT);
  pinMode(F2Pin, INPUT);
  pinMode(LEDIndicatorPin, OUTPUT);
  pinMode(toggleLedPin, OUTPUT);
  pinMode(pushButtonPin, INPUT_PULLUP);

  // Using slot：per 2ms for one frame
  ticker.attach_ms(2, frame);

  // Start monitoring
  monitor.startMonitoring();

  frameCounter = 0;
  frame();
}

void loop() {

  JobTask7(); // button detection and switching LED
  JobTask6(); // threshold LED
  
  
}

