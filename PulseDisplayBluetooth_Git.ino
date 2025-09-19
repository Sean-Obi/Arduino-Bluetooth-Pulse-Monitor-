#include <LiquidCrystal.h>
#include <PulseSensorPlayground.h>
#include <SoftwareSerial.h>

// Pulse sensor pin and object. Taken from PulseSensorPlayground Library
// PulseSensorPlayground object has built-in filtering and smoothing of pulse which was modified and added to in the sketch.
const int PulseWire = 0;               // PulseSensor GREEN WIRE connected to ANALOG PIN 0
const int LED = LED_BUILTIN;           // The on-board Arduino LED, close to PIN 13.
int Threshold = 250;                   // Orignal threshold (550) changed to 250 for threshold filtering. Higher thresholds prone to missing fait heartbeats 
PulseSensorPlayground pulseSensor;     // PulseSensorPlayground object named pulseSensor

// LCD pin. Taken from LiquidCrystal Library
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2; // defines specific arduino pins that connect to control and data pins of LCD 
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); // LiquidCrystal object named lcd

// Bluetooth pin. Taken from SoftwareSerial Library
SoftwareSerial BTSerial(10, 9);        // SoftwareSerial object named BTSerial. Used set-up connect to HC-05 Bluetooth module

// We have set up a smoothing filter to stablise BPM readings from pulse sensor, using our own code.  
const int filterSize = 20;             // Filter size for smoothing BPM readings. Avergaes over last 20 readings to reduce noise and smooth pulse
int bpmReadings[filterSize] = {0};     // Array to stores last `filterSize` BPM readings. Oldest reading is replaced by newest in circular manner
int currentReadingIndex = 0;           // Index for the current reading in the array
float smoothedBPM = 0.0;               // stores value of filtered BPM as a float after applying smoothing filter calculation 

// We have set up weighting factors for additional filtering 
// Type of low pass filtering that assigns more importance (weight) to more recent values in the filter size array
float weights[filterSize];             // Array to store the weights
float totalWeight = 0.0;               // Sum of weights for normalization

void setup() {   
  Serial.begin(115200);                // Initialise Serial Monitor. Sets baud rate to 115200 allowing for fast data transfer to computer 
  BTSerial.begin(9600);                // Initialise software serial communication on the BTSeriaL object. Sets default baud rate of 9600 for HC-05

  // Configure the PulseSensor object. Taken from PulseSensor Library
  pulseSensor.analogInput(PulseWire);  // Defines anologue input pin to read data. Previosuly set to A0 for PulseWire
  pulseSensor.blinkOnPulse(LED);       // Blink Arduino's LED with heartbeat
  pulseSensor.setThreshold(Threshold); // Defines threshold of PulseSensor object as 250

  // Initialize weights with exponential values to improve smoothing. Formula for exponential weight provided by CHATGPT
  for (int i = 0; i < filterSize; i++) { // Iterates over each element in the weight array
    weights[i] = pow(0.9, filterSize - i - 1); // Formula for exponential weights. Sets base of 0.9 raised to a decreasing exponent. 
    totalWeight += weights[i];         // Accumulates sum of all weights for normalisation 
  }

  // Initialize LCD. Taken from LiquidCrystal Library 
  lcd.begin(16, 2);                    // Set up the LCD's number of columns and rows
  lcd.print("HR Monitor");     // Print a message on the first row
  delay(2000);                         // Wait 2 seconds before starting to display BPM
  lcd.clear();                         // Clear the LCD for BPM display

 // Added our own code to SoftwareSerial Library. Checks data can be wirelessly sent
  if (pulseSensor.begin()) {
    BTSerial.print("We created a pulseSensor Object !"); 
  }
}
  // From PulseSensoryPlayground Library
void loop() {
  if (pulseSensor.sawStartOfBeat()) {             // Check if a beat happened
    int myBPM = pulseSensor.getBeatsPerMinute();  // Get the raw BPM reading
    BTSerial.print("♥  A HeartBeat Happened ! "); // If test is "true", print a message "a heartbeat happened" in Bluetooth device

    // Update the bpmReadings array with the new BPM value
    bpmReadings[currentReadingIndex] = myBPM;     // Inserts latest BPM reading (myBPM) into bpmReadings array at position indicated by currentReadingIndex
    currentReadingIndex = (currentReadingIndex + 1) % filterSize; // Modulus operator implements circular buffer.  Move to next index and wrap back round to zero 

    // We have set up the formaula to calculate the smoothed pulse using a Weighted Moving Average (WMA)
    float weightedSum = 0.0;      // Initialise weightedSum to store weighted BPM readings (bpmReadings)
    for (int i = 0; i < filterSize; i++) {        // Loop iterates through each element in bpmReadings and weights up to the filterSize
      int index = (currentReadingIndex + i) % filterSize;  // Use the circular buffer for correct order
      weightedSum += bpmReadings[index] * weights[i];      // Multiply each bpm reading by its weight to return the weightedSum
    }
    // Eqaution we have set up takes 80% of previous smoothedMBPM and adds 20% of incoming smoothedBPM to it for more graduaal and smooth transitions 
    smoothedBPM = smoothedBPM * 0.8 + (weightedSum / totalWeight) * 0.2; // Use WMA to pply low-pass filter to unfiltered BPM 

    // Output for Serial Plotter. Taken from CHATGPT 
    Serial.print(myBPM);                // Section outputs unfiltered (raw) BPM and the filtered (smoothed) BPM in comma-seperated format
    Serial.print(",");                  // Separate values with comma so Serial Plotter recognises 2 distinct lists and generates 2 seperates lines on the graph
    Serial.println(smoothedBPM);        
    
    // Output for Serial Monitor 
    Serial.print("Raw BPM");             // Section outputs unfiltered (raw) BPM and the filtered (smoothed) BPM in regular format
    Serial.print(myBPM);
    Serial.print("   Filtered BPM: ");
    Serial.println(smoothedBPM);

    // Update LCD display with the filtered BPM
    lcd.setCursor(0, 0);               // Set cursor to the beginning of the first row
    lcd.print("Heart Rate:");          // Label for clarity. Take filter BPM as "Heart Rate' on the LCD 
    lcd.setCursor(0, 1);               // Set cursor to the beginning of the second row
    lcd.print("        ");             // Clear any residual characters on the second row before updating with new BPM 
    lcd.setCursor(0, 1);               // Reset cursor to the beginning of the second row 
    lcd.print(smoothedBPM, 1);         // Print filtered BPM with one decimal place
    
    // Send text and data of filtered BPM to wirelessly connected Blutetooh device 
    BTSerial.print("Your heart rate is: ");
    BTSerial.print(smoothedBPM, 1);
    BTSerial.print("\n");              // New line after each BPM transmission for the next BPM reading 
  }

  delay(50);  // Taken from PulseSensorPlayground Library. Delay was increased from 20 to 50 for more stability
}
