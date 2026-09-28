#include <LiquidCrystal.h>

// Initialize LCD with your pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// --- Pin Definitions ---
const int voltagePin    = A0;   // Voltage Potentiometer
const int tempPin       = A1;   // TMP36 Temperature Sensor
const int currentPin    = A2;   // Current Potentiometer
const int flamePin      = A3;   // Photoresistor (Flame Sensor)
const int gasPin        = A4;   // MQ Gas Sensor (Smoke)
const int transistorPin = 8;    // NPN Transistor (Motor Control)
const int greenLed      = 9;    // Safe LED
const int redLed        = 10;   // Fault LED
const int buzzer        = 13;   // Alarm

// --- Safety Thresholds ---
const float MAX_VOLTAGE = 4.2;
const float MIN_VOLTAGE = 3.0;
const float MAX_TEMP    = 40.0;
const float MAX_CURRENT = 20.0;
const int MAX_GAS       = 400;  // Adjust based on your clean-air baseline
const int MAX_FLAME     = 600;  // Adjust based on room brightness

void setup() {
  // Configure Pins
  pinMode(transistorPin, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(gasPin, INPUT);
  pinMode(flamePin, INPUT);

  // Initialize Serial Monitor & LCD
  Serial.begin(9600);          // Start Serial Communication for the Plotter
  lcd.begin(16, 2);

  // Boot Screen
  lcd.setCursor(1, 0);         
  lcd.print("EV BMS MONITOR"); 
  delay(1000);                 
  lcd.clear();                 
}

void loop() {
  // --- 1. READ SENSORS ---
  float voltage = analogRead(voltagePin) * (5.0 / 1023.0); 
  float tempVolts = analogRead(tempPin) * (5000.0 / 1024.0);
  float temperatureC = (tempVolts - 500) / 10.0;
  float currentAmps = analogRead(currentPin) * (30.0 / 1023.0);
  
  int gasValue = analogRead(gasPin); 
  int flameValue = analogRead(flamePin); 

  // --- 2. PRINT TO SERIAL PLOTTER ---
  // Graphing ONLY Voltage, Current, and Temperature
  Serial.print(voltage);
  Serial.print(",");
  Serial.print(currentAmps);
  Serial.print(",");
  Serial.println(temperatureC); // println adds the new line to push the data point

  // --- 3. DISPLAY DATA ON LCD ---
  lcd.setCursor(0, 0);
  lcd.print("V:"); lcd.print(voltage, 1); lcd.print("V ");
  
  lcd.setCursor(9, 0);
  lcd.print("I:"); lcd.print(currentAmps, 0); lcd.print("A  ");

  lcd.setCursor(0, 1);
  lcd.print("T:"); lcd.print(temperatureC, 0); lcd.print("C   ");

  // --- 4. PROTECTION LOGIC ---
  bool voltFault = (voltage > MAX_VOLTAGE || voltage < MIN_VOLTAGE);
  bool tempFault = (temperatureC > MAX_TEMP);
  bool currentFault = (currentAmps > MAX_CURRENT);
  bool gasFault = (gasValue > MAX_GAS); 
  bool flameFault = (flameValue > MAX_FLAME); 

  // If ANY fault is detected, trigger the safety cutoff
  if (voltFault || tempFault || currentFault || gasFault || flameFault) {
    
    // === FAULT MODE ===
    digitalWrite(transistorPin, LOW); // Cut Power
    digitalWrite(greenLed, LOW);      // Green OFF
    digitalWrite(redLed, HIGH);       // Red ON
    
    lcd.setCursor(9, 1);
    
    // Prioritize Fire/Smoke warnings
    if (flameFault) {
      lcd.print("FLAME! ");
      tone(buzzer, 932);              // Pulsing Fire Tone             
    } else if (gasFault) {
      lcd.print("SMOKE! ");
      tone(buzzer, 932);              // Pulsing Fire Tone             
    } else {
      lcd.print("Warning");
      digitalWrite(buzzer, HIGH);     // Solid Alarm for Electrical Faults     
    }
    
  } else {
    
    // === SAFE MODE ===
    digitalWrite(transistorPin, HIGH); 
    digitalWrite(greenLed, HIGH);      
    digitalWrite(redLed, LOW);         
    
    noTone(buzzer);                    
    digitalWrite(buzzer, LOW);         
    
    lcd.setCursor(9, 1);
    lcd.print("Sys:OK ");
  }
  
  delay(150); // Small delay to stabilize the graph and screen
}