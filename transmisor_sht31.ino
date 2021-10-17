  #include <Arduino.h>
  #include <Wire.h>
  #include "Adafruit_SHT31.h"
  #include <avr/sleep.h>
  #include <avr/wdt.h>
  #include <avr/power.h>
  // Include RadioHead Amplitude Shift Keying Library
  #include <RH_ASK.h>
  // Include dependant SPI Library 
  #include <SPI.h>
  
  #define N 5 // se utiliza como comparador de la variable 'counter' - modifica el lapso de tiempo en que duerme el chip -
  #define BAT A0 
  #define led 3
  unsigned int counter; // contador para deshabilitar el modo 'deep sleep'
  unsigned int analog_bat;
  char humedad[7];
  char volt[7];
  const int power = A3; // pin que activa y desactiva los modulos SHT31 y RF433MHz
  float Val_calibracion, N_H;
  float vbat;
  float v_source = 6;
  
  void sht31_module(void); // funcion para tomar lectura de la HUMEDAD RELATIVA y decidir si esta se envia o no
  
  Adafruit_SHT31 sht31 = Adafruit_SHT31(); // crea el objeto Adafruit_SHT31();
  RH_ASK rf_driver; // creates an Amplitude Shift Keying Object

enum {
  WDT_1_SEC = 0b000110,
  WDT_2_SEC = 0b000111,
  WDT_4_SEC = 0b100000,
  WDT_8_SEC = 0b100001,
};

ISR (WDT_vect) {
  wdt_disable(); // desabilitar el Watchdog
}

void myWatchdogEnable (const byte interval) {
  // secuencia temporizada
  noInterrupts(); // desabilita todas las interrupciones

  // se limpian algunas 'reset' flags
  MCUSR &= ~(1<<WDRF);
  // se permiten modificaciones, se desabilita el 'reset'
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  // configura el modo de interrupcion y un intervalo
  WDTCSR = bit (WDIE) | interval;    // configura WDIE y el retardo (delay) solicitado
  wdt_reset(); // se actualiza el 'watchdog' para que no produzca un reinicio

  // desabilitar el ADC
  byte old_ADCSRA = ADCSRA;
  ADCSRA = 0;
  ADMUX = 0;  // apaga la referencia interna (Vref)

  // apagar varios modulos
  power_all_disable(); // desabilita todos los modulos

  // Registro de desactivacion de entrada digital en pines analogicos
  DIDR0 = bit (ADC0D) | bit (ADC1D) | bit (ADC2D) | bit (ADC3D) | bit (ADC4D) | bit (ADC5D);
  DIDR1 = bit (AIN1D) | bit (AIN0D);

  // listo para dormir o modo 'sleep'
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);
  sleep_enable();

  // desactivar la habilitacion por caida de voltaje en software
  // BODS debe ser configurado en '1' y BODSE se configura a '0' (zero) dentro de cuatro ciclos de reloj
  MCUCR = bit (BODS);
  interrupts (); // un ciclo
  sleep_cpu (); // un ciclo

  // cancelar sleep como precaucion
  sleep_disable();
  // encender modulos internos
  power_all_enable();
  // reactivar el ADC
  ADCSRA = old_ADCSRA;
}

void setup() {
  // Deshabilita el watchdog timer
  wdt_disable();
}   

void loop() {
  // duerme por 1 segundos 
  myWatchdogEnable (WDT_1_SEC);
  counter++;
  
  if (counter >= N) {
    sht31_module();
    counter = 0;
  }
}

void sht31_module() {
  pinMode(power, OUTPUT);
  pinMode(led, OUTPUT);
  digitalWrite(power, LOW);
  digitalWrite(led,LOW);
  delay(100);
  digitalWrite(led  ,HIGH);
  delay(100);
  
  
  // Initialize ASK Object
  rf_driver.init();
  
  // SHT31 - pines de conexion SDA = A4
  // SHT31 - pines de conexion SCL = A5
  Serial.begin(9600);
  while (!Serial)
    delay(10);     // will pause Zero, Leonardo, etc until serial console opens

  Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.print("Couldn't find SHT31");
    while (1) delay(1);
  }
  
  float h = sht31.readHumidity(); // lee la señal de HUMEDAD RELATIVA

  //ecuacion de linealizacion por calibracion del sensor
  Val_calibracion = 0.27*(h)-17.9;
  N_H = Val_calibracion + h; 

  dtostrf(N_H,0,2,humedad); // la funcion dtostrf() convierte un valor 'float' a 'string' con ciertas condiciones
                            // en este caso la variable float 'h' a un string 'humedad'
  
  const char *msg = humedad;
  rf_driver.send((uint8_t *)msg, strlen(msg));
  rf_driver.waitPacketSent();
  delay(1000);
  
  analogReference(INTERNAL);
  analog_bat = analogRead(BAT);
  analog_bat = analogRead(BAT);

  vbat=(v_source/1023.0)*(analog_bat);
  Serial.println(vbat);
  dtostrf(vbat,0,2,volt);
  
  const char *msg1 = volt;
  rf_driver.send((uint8_t *)msg1, strlen(msg1));
  rf_driver.waitPacketSent();
  delay(1000);
  
  digitalWrite(power, HIGH);
  
}
