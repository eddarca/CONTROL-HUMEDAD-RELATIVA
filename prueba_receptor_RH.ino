// Include RadioHead Amplitude Shift Keying Library
#include <RH_ASK.h>
// Include dependant SPI Library 
#include <SPI.h> 
 
// Create Amplitude Shift Keying Object
RH_ASK rf_driver;
//..............................
// pin digital 2 rele arduino
#define actuador 2
//..............................
 
void setup() {
  // Initialize ASK Object
  rf_driver.init();
  // Setup Serial Monitor
  Serial.begin(9600);
}
 
void loop() {
  // Set buffer to size of expected message
  uint8_t buf[5];
  uint8_t buflen = sizeof(buf);
  // Check if received packet is correct size
  if (rf_driver.recv(buf, &buflen)) {
    String str = (char*)buf;
    float valor = str.toFloat();
    // Message received with valid checksum
//    Serial.print("%HR: ");
//    Serial.println((char*)buf);
    Serial.print("Humedad =  ");
    Serial.println(valor);
  }
  
}
