#include <B31DGMonitor.h>

B31DGMonitor monitor;

void setup(void)
{
  Serial.begin(9600); // Starts the serial communication and sets baud rate to 9600
  
  // ... setup code
  
  monitor.startMonitoring(); // all the tasks should be released after this, this time can be queried using getTimeStart()
}

void taskA() {
   monitor.jobStarted(i);  // i between [1..5], i.e. 1 is digital output, 5 is serial print
   // ... perform taskA
   monitor.jobEnded(i);
}

void taskB() {
   monitor.taskStart(i);
   // ... perform taskB
   monitor.jobEnded(i);
}

// simple example of cyclic executive (version not using Ticker)
void loop(void) 
{
   // this cyclic executive executes taskA and then taskB in each new frame
   ...
   taskA();
   ...
   taskB();

   delayMicroseconds(SOME_DELAY); 
}
