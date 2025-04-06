#define private public
#include <B31DGMonitor.h>  
#undef private

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

//-----------pin
const int digSig1Pin = 14;      // Task 1
const int digSig2Pin = 15;      // Task 2
const int F1Pin = 27;           // Task 3
const int F2Pin = 25;           // Task 4
const int LEDIndicatorPin = 16; // Task 6
const int toggleLedPin = 19;    // Task 7
const int pushButtonPin = 32;   // Task 7

//------------------------------------
// global variables
volatile unsigned int freq1 = 0;
volatile unsigned int freq2 = 0;

// Semaphore
SemaphoreHandle_t freqMutex;

//------------------------------------
// Button Debouncing
const unsigned long debounceDelay = 50;  // 消抖50ms
int lastButtonState = HIGH;
int buttonState = HIGH;
int currentToggleState = LOW;
unsigned long lastDebounceTime = 0;

//------------------------------------
// Monitor library
B31DGCyclicExecutiveMonitor monitor;


// Task 1
void outputDigitalSignal1() {
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

// Task 2
void outputDigitalSignal2() {
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

// Helper Function for Frequency Measurement
unsigned int measureFrequencyFunc(int pin, unsigned long timeout) {
  unsigned long highTime = pulseIn(pin, HIGH, timeout);
  if (highTime == 0) return 0;
  unsigned long period = 2 * highTime;  
  return 1000000UL / period;
}

// task3
void measureF1() {
  monitor.jobStarted(3);
  unsigned int f = measureFrequencyFunc(F1Pin, 1600);
  xSemaphoreTake(freqMutex, portMAX_DELAY);
  freq1 = f;
  xSemaphoreGive(freqMutex);
  monitor.jobEnded(3);
}

//task4
void measureF2() {
  monitor.jobStarted(4);
  unsigned long tStart = micros();
  unsigned int f = measureFrequencyFunc(F2Pin, 1000);
  unsigned long tExec = micros() - tStart;

  xSemaphoreTake(freqMutex, portMAX_DELAY);
  freq2 = f;
  xSemaphoreGive(freqMutex);
  monitor.jobEnded(4);
}

// task5
void simulateWork() {
  monitor.jobStarted(5);
  monitor.doWork();
  monitor.jobEnded(5);
}

// task6
void updateLEDIndicator() {
  xSemaphoreTake(freqMutex, portMAX_DELAY);
  unsigned int sum = freq1 + freq2;
  xSemaphoreGive(freqMutex);
  if (sum > 1500)
    digitalWrite(LEDIndicatorPin, HIGH);
  else
    digitalWrite(LEDIndicatorPin, LOW);
}

// task7
void checkButtonAndToggleLED() {
  int reading = digitalRead(pushButtonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW) { // button press
        currentToggleState = !currentToggleState;
        digitalWrite(toggleLedPin, currentToggleState);
        monitor.doWork();
      }
    }
  }
  lastButtonState = reading;
}


// Task2 priority
void Task2(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(3);
  for (;;) {
    outputDigitalSignal2();
    vTaskDelayUntil(&xLastWakeTime, period);
  }
}

// Task1
void Task1(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(4);
  for (;;) {
    outputDigitalSignal1();
    vTaskDelayUntil(&xLastWakeTime, period);
  }
}

// Task5
void Task5(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(5);
  for (;;) {
    simulateWork();
    vTaskDelayUntil(&xLastWakeTime, period);
  }
}

// Task4
void Task4(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(10);
  for (;;) {
    measureF2();
    vTaskDelayUntil(&xLastWakeTime, period);
  }
}

// Task3
void Task3(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(10);
  for (;;) {
    measureF1();
    vTaskDelayUntil(&xLastWakeTime, period);
  }
}

// Task7
void Task7(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(10);
  for (;;) {
    checkButtonAndToggleLED();
    vTaskDelayUntil(&xLastWakeTime, period);
  }
}

// Task6
void Task6(void *pvParameters) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(50);
  for (;;) {
    updateLEDIndicator();
    vTaskDelayUntil(&xLastWakeTime, period);
  }
}

// StartMonitorTask
void StartMonitorTask(void *pvParameters) {
  vTaskDelay(pdMS_TO_TICKS(100));  // 延时100ms等待系统稳定
  monitor.startMonitoring();
  Serial.println("Monitor started after delay");
  vTaskDelete(NULL); 
}
//Violation
void MonitorReportTask(void *pvParameters) {
  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(10000)); // 10seconds
    monitor.printSummary();
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { }

  
  pinMode(digSig1Pin, OUTPUT);
  pinMode(digSig2Pin, OUTPUT);
  pinMode(F1Pin, INPUT);
  pinMode(F2Pin, INPUT);
  pinMode(LEDIndicatorPin, OUTPUT);
  pinMode(toggleLedPin, OUTPUT);
  pinMode(pushButtonPin, INPUT_PULLUP);

  // Semaphore
  freqMutex = xSemaphoreCreateMutex();
  if (freqMutex == NULL) {
    Serial.println("Error creating mutex");
    while (1) { }
  }

  
  xTaskCreate(Task2, "Task2", 8192, NULL, 4, NULL); 
  xTaskCreate(Task1, "Task1", 8192, NULL, 3, NULL); 
  xTaskCreate(Task5, "Task5", 8192, NULL, 3, NULL); 
  xTaskCreate(Task4, "Task4", 8192, NULL, 3, NULL); 
  xTaskCreate(Task3, "Task3", 8192, NULL, 2, NULL); 
  xTaskCreate(Task7, "Task7", 8192, NULL, 2, NULL); 
  xTaskCreate(Task6, "Task6", 8192, NULL, 1, NULL); 
  xTaskCreate(StartMonitorTask, "StartMonitor", 8192, NULL, 1, NULL); 
  xTaskCreate(MonitorReportTask, "MonitorReport", 8192, NULL, 1, NULL); 

  
  vTaskStartScheduler();
}

void loop() {
  
}

