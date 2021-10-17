  //librerias para la pantalla LCD
  #include <LiquidCrystal.h> //libreria pantalla LCD
  LiquidCrystal lcd(7,8,5,4,3,2); //estipulacion de pines
  
  //pin 7 - escritura y lectura de comandos
  //pin 8 - sintonizacion
  //pin 5,4,3,2 - pines de datos
  
byte triste[8] = {
  B00000,
  B00000,
  B01010,
  B00000,
  B01110,
  B10001,
  B10001,
  B00000
};
  byte battery[8] = {
    B01110,
    B01010,
    B10001,
    B10001,
    B10001,
    B10001,
    B11111,
    B11111
};

  //varialbes de muestreo
  int periodo = 5000;
  unsigned long tiempoahora =0;
  
  //...................................................................
  //librerias control fuzzy.h
   
  #include <Fuzzy.h>  //inclusion de la libreria 
  Fuzzy*control = new Fuzzy(); //se crea objeto fuzzy "control"
  
  const int hum1 = 6; //activacion rele canal 2
  const int ENB = 13; //activacion rele canal 1
  
  float Humedad,setpoint,ERROR,voltaje,valor;
  int humidificador_1,ventilacion;
  char lectura;
  String str;
  //....................................................................
  //inclusion de la libreria para transmision modulo RF
  
  // Include RadioHead Amplitude Shift Keying Library
  #include <RH_ASK.h>
  // Include dependant SPI Library 
  #include <SPI.h> 
   
  // Create Amplitude Shift Keying Object
  RH_ASK rf_driver;
  //....................................................................

  void setup() {
  pinMode(ENB, OUTPUT);
  pinMode(hum1, OUTPUT);
    
  Serial.begin(9600);
  lcd.begin(16,2);    //numero de columnas por filas
  lcd.createChar(1,triste);
  lcd.createChar(2,battery);

  // Initialize ASK Object
  rf_driver.init();
  // Setup Serial Monitor
  
  //...............................................................
  //establecimiento de los parametros de control - difusificacion - inferencia - reglas
  //Declaracion del universo del discurso de entrada
  //Correspondiente al error de setpoint-humedad
   
  FuzzyInput*Error = new FuzzyInput(1);
  
  //ingreso de los conjuntos difusso correspondientes al universo del discurso Error
  
  FuzzySet*ENG = new FuzzySet(-100,-100,-5,0); //Error negativo grande trapezoidal
  Error->addFuzzySet(ENG); //se añade el conjunto al universo de entrada 
  //*********************************************************************
  FuzzySet*EC  = new FuzzySet(-2,0,0,2);      // Error cero o centro trinagular
  Error->addFuzzySet(EC);
  //*********************************************************************
  FuzzySet*EPP  = new FuzzySet(0,5,100,100);  // Error positivo pequeño trinagular
  Error->addFuzzySet(EPP);
  //*********************************************************************
  control->addFuzzyInput(Error); //se agrega al conjunto fuzzy el universo con
  //sus respectivos conjuntos
 
 //------------------------------------------------------------------------
  //declaracion de los conjuntos difusos o de membresia de las salidas respectivas
  //ingreso de la salida del sistema humidificador 1
  
  FuzzyOutput*humidificador_1 = new FuzzyOutput(1);
  //ingreso de los conjuntos difusso correspondientes al universo del discurso humidificador 1

  FuzzySet*OFF1 = new FuzzySet(0,0,0,0);            // desactivacion humidificador 1
  humidificador_1->addFuzzySet(OFF1);

  FuzzySet*ON1 = new FuzzySet(255,255,255,255);        // activacion humidificador 1    
  humidificador_1->addFuzzySet(ON1);

  control->addFuzzyOutput(humidificador_1);
  
 //------------------------------------------------------------------------
 //declaracion de los conjuntos difusos o de membresia de las salidas respectivas
  //ingreso de la salida del sistema humidificador 2
  
  FuzzyOutput*ventilacion = new FuzzyOutput(2);
  //ingreso de los conjuntos difusso correspondientes al universo del discurso humidificador 1

  FuzzySet*OFF2 = new FuzzySet(0,0,0,0);            // desactivacion humidificador 2
  ventilacion->addFuzzySet(OFF2);

  FuzzySet*ON2 = new FuzzySet(255,255,255,255);        // activacion humidificador 2    
  ventilacion->addFuzzySet(ON2);

  control->addFuzzyOutput(ventilacion);

  //------------------------------------------------------------------------
  //-----------------------------------------------------------------------
  //------------------------------------------------------------------------
  //establecimiento de las reglas de control inferencia de variables linguisticas
  
  FuzzyRuleAntecedent*si_error_es_ENG = new FuzzyRuleAntecedent(); si_error_es_ENG->joinSingle(ENG);        //antencendente 1
  
  FuzzyRuleConsequent*ent_humi1_es_off  = new FuzzyRuleConsequent(); ent_humi1_es_off->addOutput(OFF1);    //consecuente 1
  FuzzyRuleConsequent*ent_venti_es_on   = new FuzzyRuleConsequent(); ent_venti_es_on->addOutput(ON2);       //consecuente 2    
  
  FuzzyRule*regla_1 = new FuzzyRule(1,si_error_es_ENG,ent_humi1_es_off); control->addFuzzyRule(regla_1);    //asignacion 1
  FuzzyRule*regla_2 = new FuzzyRule(2,si_error_es_ENG,ent_venti_es_on); control->addFuzzyRule(regla_2);     //asignacion 2
   
  //-------------------------------------------------------------------------

  FuzzyRuleAntecedent*si_error_es_EC = new FuzzyRuleAntecedent(); si_error_es_EC->joinSingle(EC);  //antencendente 2

  FuzzyRuleConsequent*ent_humi1_es_off1  = new FuzzyRuleConsequent(); ent_humi1_es_off1->addOutput(OFF1);    //consecuente 3
  FuzzyRuleConsequent*ent_venti_es_off  = new FuzzyRuleConsequent(); ent_venti_es_off->addOutput(OFF2);    //consecuente 4
 
  FuzzyRule*regla_3 = new FuzzyRule(3,si_error_es_EC,ent_humi1_es_off1); control->addFuzzyRule(regla_3);      //asignacion 3
  FuzzyRule*regla_4 = new FuzzyRule(4,si_error_es_EC,ent_venti_es_off); control->addFuzzyRule(regla_4);     //asignacion 4

 //-------------------------------------------------------------------------
 
  FuzzyRuleAntecedent*si_error_es_EPP = new FuzzyRuleAntecedent(); si_error_es_EPP->joinSingle(EPP);  //antencendente 3

  FuzzyRuleConsequent*ent_humi1_es_on     = new FuzzyRuleConsequent(); ent_humi1_es_on->addOutput(ON1);    //consecuente 5
  FuzzyRuleConsequent*ent_venti_es_off2   = new FuzzyRuleConsequent(); ent_venti_es_off2->addOutput(OFF2);    //consecuente 6

  FuzzyRule*regla_5 = new FuzzyRule(5,si_error_es_EPP,ent_humi1_es_on);     control->addFuzzyRule(regla_5);      //asignacion 5
  FuzzyRule*regla_6 = new FuzzyRule(6,si_error_es_EPP,ent_venti_es_off2);   control->addFuzzyRule(regla_6);     //asignacion 6
 
 //-------------------------------------------------------------------------
 
 }
 
  void loop() {
    
  tiempoahora=millis();
  while(millis() < tiempoahora + periodo){
    //realiza la espera de 1 segundo
  }
  
  int tiem = millis()/1000;
  Serial.print(tiem);
  Serial.print("\t");
  
  // Set buffer to size of expected message
  uint8_t buf[5];
  uint8_t buflen = sizeof(buf);
  
  // Check if received packet is correct size
  if (rf_driver.recv(buf, &buflen)) {
    str = (char*)buf;
    valor = str.toFloat();
    // Message received with valid checksum
    if (valor >= 7){
       Humedad = valor;
    }
    else {
       voltaje = valor;
    }
  }

  Serial.print(voltaje);
  Serial.print("\t");
  Serial.print(Humedad);
  Serial.print("\t");

  //condicional proyeccion pantalla LCD
  if(voltaje == 0 && Humedad==0){
    lcd.setCursor(0,0);
    lcd.print("Esperando Envio");
    lcd.setCursor(0,1);
    lcd.print("De Informacion.");
    }
    else{
      if (voltaje !=0 || Humedad !=0){
        
      lcd.clear();
      //visualizacion pantalla LCD
      lcd.setCursor(0,0);
      lcd.print("Hs = ");
      lcd.print(Humedad);
      lcd.print(" %HR");
      
      //visualizacion pantalla LCD
      lcd.setCursor(0,1);
      lcd.print("BT = ");
      lcd.print(voltaje);
      lcd.print(" V");

      }
      if(voltaje > 0 && voltaje <= 3.5 ){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("LOW BATTERY  ");
      lcd.write(2);
      lcd.setCursor(0,1);
      lcd.print("CAMBIAR      ");
      lcd.write(1);
    }
  }
  
   //variables referencia establecimiento de setpoint's por serial
  if(Serial.available()){  //realiza el registro para verificacion de datos por serial
    lectura=Serial.read();  //almacenamiento de caracteres enviados por serial
    if(lectura == '1'){
      setpoint = 60;  //variable de referencia 50
    }
    if(lectura == '2'){
       setpoint = 65;  //variable de referencia 60
    }
    if(lectura == '3'){
      setpoint = 70;  //variable de referencia 70
    }
    if(lectura == '4'){
      setpoint = 75;  //variable de referencia 80  
    }
    if(lectura == '5'){
      setpoint = 80;  //variable de referencia 90  
    }
    if(lectura == '6'){
      setpoint = 85;  //variable de referencia 100
    } 
    if(lectura == '7'){
      setpoint = 90;  //variable de referencia 100
    } 
    if(lectura == '8'){
      setpoint = 95;  //variable de referencia 100
    } 
  }

  Serial.print(setpoint);
  Serial.print("\t");
  
  ERROR=setpoint-Humedad; //establecimiento del error
  control->setInput(1,ERROR); //se asigna al sistma fuzzy control una entrada con el valor de error
  control->fuzzify(); //fusificacion del sistema

  humidificador_1  = control->defuzzify(1); //se desdifusifica el sistema, guardando la salida 1 en la variable de humidificador 1
  ventilacion      = control->defuzzify(2);
  
  analogWrite(hum1,humidificador_1); //actuacion por medio de la accion de control humedad 
  analogWrite(ENB,ventilacion);       //actuacion por medio de la accion de control ventilacion 

//  Serial.print("E = ");
  Serial.print(ERROR);
  Serial.print("\t");
//  Serial.print("pH_1 = ");
  Serial.print(humidificador_1);
  Serial.print("\t");
//  Serial.print("pH_2 = ");
  Serial.println(ventilacion);
 }
