/* Copyright 2021 Frank Adams
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at
       http://www.apache.org/licenses/LICENSE-2.0
   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
// I refuse to pay $50 for a third party phone app to let me know I forgot to plug in my Tesla. Hopefully Tesla will add 
// this function to their phone app in the future but in the mean time, I've built a simple circuit based on an ATTiny85.  
// It will turn on a buzzer if the Tesla has been sitting in the garage for 30 minutes or more and the charge cables
// are still coiled on the wall hook. There is a push button switch at the base of the wall hook that is depressed by the 
// weight of the cables. The switch is monitored by the ATTiny. 
// The ATTiny controls an HC-SR04 ultrasonic sensor which is aimed at the Tesla that is about 3 feet away when in the garage. 
// If the Tesla is not in the garage, the HC-SR04 measures over 10 feet to the far wall. An LED is controlled by the ATTiny 
// that blinks 1 to 9 times to indicate a distance of 1 to 9 feet. Blink rate is 1 second. If the distance is 10 feet or more, 
// the LED blinks slowly at a rate of 5 seconds "on" & 5 seconds "off". The ATTiny repeats the loop every 10 seconds. 
// The circuit is powered by a 5 Volt 1 Amp wall wart supply.     
//
// Release History
// Dec 23, 2021  Original Release

// ATTiny Pins (Reset on pin 1 is not used and is internaly pulled up to 5 volts)
#define Trig 3 // PB3 = Pin 2; Logic output to the Trig pin of the HC-SR04. (10us high pulse to trigger)
#define Echo 4 // PB4 = Pin 3; Logic input from the Echo pin of the HC-SR04 (High pulse duration is round trip time)
#define Switch 0 // PB0 = Pin 5; Logic input from the pulled up push button switch in the charge cable wall hook (1=Open)
#define Buzzer 1 // PB1 = Pin 6; Logic output to turn on the active buzzer (thru an NPN transistor). (1=On)
#define LED 2 // PB2 = Pin 7; Logic output to turn on an LED (1=On)
// Globals
int feet; // Distance to car in feet
int last_feet = 3; // previous loop feet value
long roundtrip; // Trig signal pulse duration (over and back)
long time_count = 0; // counter
//
// Function reads the distance from the HC-SR04
int measure(void) 
{ 
  digitalWrite(Trig, HIGH); // Send HC-SR04 trigger signal high
  delayMicroseconds(10); // Hold the trigger signal high for 10 usec
  digitalWrite(Trig, LOW); // Send HC-SR04 trigger signal LOW
  int roundtrip = pulseIn(Echo, HIGH); // measure echo pin high pulse width
  return (roundtrip / 1776); // distance to car in feet
}
//
// Function to blink the LED based on the measured distance in feet. This function takes a total time of 10 seconds.
void blink_led(int feet)   
{
  if (feet < 1) { // Do not blink if under 1 foot
    digitalWrite(LED, LOW);
    delay(10000); // wait 10 seconds with the led off
  }
  else if (feet > 9) { // Give long single blink to indicate 10 or more feet
    digitalWrite(LED, HIGH);
    delay(5000); // 5 seconds turned on
    digitalWrite(LED, LOW);
    delay(5000); // 5 seconds turned off
  }
  else {  // the number of LED blinks equals the number of feet 
    for (int i=0; i<feet; i++) {
    digitalWrite(LED, HIGH); // turn on LED
    delay(500); // 1/2 second turned on
    digitalWrite(LED, LOW); // turn off LED
    delay(500); // 1/2 second turned off
    }
    delay(10000-(feet*1000)); // Delay 10 seconds - (feet) seconds
  }
}
//
void setup()
{
  pinMode(Echo, INPUT); // Define HC-SR04 Echo signal as an input
  pinMode(Switch, INPUT); // Define signal from push button switch as an input
  pinMode(Trig, OUTPUT); // Define trigger signal to HC-SR04 as an output
  pinMode(LED, OUTPUT); // Define LED signal as an output
  pinMode(Buzzer, OUTPUT); // Define buzzer signal as an output
  digitalWrite(Trig, LOW); // Send HC-SR04 trigger signal to the resting LOW state
  digitalWrite(LED, LOW); // turn off led
  time_count = 0; // initialize counter to zero
  // Sound Buzzer to confirm operation
  digitalWrite(Buzzer, HIGH); // turn on buzzer
  delay(1000); // wait 1 second 
  digitalWrite(Buzzer, LOW); // turn off buzzer
}
   
void loop() // This loop repeats every 10 seconds 
{
  feet = measure(); // measure the distance in feet to the car (or the far wall)
  if (!digitalRead(Switch) && (feet <= 7)) {  // check if not plugged in and car is in the garage
    time_count++; // increment the counter (a bad distance reading will increment the counter but get zero'ed on the next loop)
  }
  else if (digitalRead(Switch)) {
    time_count = 0; // zero the counter because the car is plugged in
  }
  else if ((last_feet >= 8) && (feet >= 8))  { 
    time_count = 0; // zero the counter  if 2 loops in a row show car is out of the garage (won't zero if 1 bad distance reading)
  }
//
  if (time_count >= 180) { // 180 loops equals 30 minutes
    digitalWrite(Buzzer, HIGH); // turn on buzzer
  }
  else {
    digitalWrite(Buzzer, LOW); // turn off buzzer
  }
//
  blink_led(feet); // blink the led to indicate the distance in feet. This takes 10 seconds.
//
  last_feet = feet; // save measurement for next loop in case of a bad/noisy read
//
}
