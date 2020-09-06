#include <LiquidCrystal.h>
#include <Adafruit_Keypad.h>
#include <Servo.h>
#include <Stepper.h>
 
// Esto es el número de pasos por revolución
#define Pasos 20
 
// Constructor, pasamos STEPS y los pines donde tengamos conectado el motor
Stepper stepper1(Pasos, 46, 45, 44, 43);
Stepper stepper2(Pasos,42,41, 40, 39);

//Pines de porton
#define Rojo 53
#define Amarillo 52
#define pinServo 51
#define Abrir 50
#define Cerrar 49

//Pines de laboratorios
#define Lab1 48
#define Lab2 47
Servo servo;

bool moverServo,cierreInesperado,porton;
long tiempoPorton=0,tiempoCierre=0;

/*
    The circuit:
 * LCD RS pin to digital pin 7
 * LCD Enable pin to digital pin 6
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
*/
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


//Colocamos todo lo necesario para el teclado
char keys[4][3] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[4] = {25, 26, 27, 28}; //connect to the row pinouts of the keypad
byte colPins[3] = {22, 23, 24}; //connect to the column pinouts of the keypad

Adafruit_Keypad teclado = Adafruit_Keypad( makeKeymap(keys), rowPins, colPins, 4, 3);

//INFORMACION PARA COMUNICACION CON LA APLICACION MEDIANTE BLUETOOTH

//Variables Bluetooth
int Enviados[] = {0,0}; //Hacemos un arreglo para los datos a enviar
int ledLab1 = 38; //Declaramos el pin de las luces del lab 1
int ledLab2 = 37; //Declaramos el pin de las luces del lab 1
int ledCamiones = 36; //Declaramos el pin de las luces de la entrada de camiones
int ledEmpleados = 35; //Declaramos el pin de las luces de la entrada de empleados
char entradaApp; //Declaramos una variables para los datos de entrada

//Clock 
int periodo = 500; //El tiempo que se demora en enviar un nuevo dato a la aplicacion
unsigned long TiempoAhora = 0; //Variable para determinar el tiempo transcurrido
/*
     Lab1 banda: A -> Encendido
     Lab1 banda: B -> Apagado
     
     Lab2 banda: C -> Encendido
     Lab2 banda: D -> Apagado 

     Abrir porton: E -> Encendido
     Cerrar porton: F -> Apagado 

     Luces Lab1: G -> Encender
     Luces Lab1: H -> Apagar

     Luces Lab2: I -> Encender
     Luces Lab2: J -> Apagar

     Luces SalidaCamiones: K -> Encender
     Luces SalidaCamiones: L -> Apagar

     Luces EntradaEmpleados: M -> Encender
     Luces EntradaEmpleados: N -> Apagar

     Luces General: O -> Encender
     Luces General: P -> Apagar
*/

void setup() {
  moverServo=cierreInesperado=porton=false;
  servo.attach(pinServo);
  Serial.begin(9600);
  pinMode(Rojo,OUTPUT);
  pinMode(Amarillo,OUTPUT);
  digitalWrite(Amarillo, HIGH);
  servo.write(0);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(20, 2);
  // Print a message to the LCD.
  lcd.print("Bienvenido");

  //Teclado de empieza xd
  teclado.begin();

  //Nos posicionamos en la columna 0 y fila 1
  lcd.setCursor(0, 1);


  //Pines del Porton
  pinMode(Rojo, OUTPUT);
  pinMode(Amarillo, OUTPUT);
  pinMode(pinServo, OUTPUT);
  pinMode(Abrir, INPUT);
  pinMode(Cerrar, INPUT);

  //Pines de laboratorio
  pinMode(Lab1, INPUT);
  pinMode(Lab2, INPUT);

  //Pines de Stepper con Driver
  /*pinMode(ST1B, OUTPUT);
  pinMode(ST2B, OUTPUT);
  pinMode(ST3B, OUTPUT);
  pinMode(ST4B, OUTPUT);*/

  // Asignamos la velocidad en RPM (Revoluciones por Minuto)
  stepper1.setSpeed(10);
  stepper2.setSpeed(10);
  //Pines de las luces
  pinMode(ledLab1,OUTPUT);
  pinMode(ledLab2,OUTPUT);
  pinMode(ledCamiones,OUTPUT);
  pinMode(ledEmpleados,OUTPUT);
}

void loop() {
  
  teclado.tick();
  controladorAplicacion();
  while(teclado.available()){
    keypadEvent e = teclado.read();
    if (e.bit.EVENT == KEY_JUST_PRESSED) lcd.print((char)e.bit.KEY);
  }


//-------------------Porton-------------------
   if(digitalRead(Abrir)){
    moverServo=true;
  }
  if(digitalRead(Cerrar)){
    cierreInesperado=true;
    moverServo=false;
  }
  Porton();
  moverStepper(1);
  //Leer informacion enviada por la aplicacion
  

}


void Porton(){
  if(cierreInesperado){
     //Serial.println("Cerrando");
    if(digitalRead(Rojo)&&tiempoCierre==0){
      tiempoPorton=millis();
      digitalWrite(Rojo, LOW);
      tiempoCierre=2100;
      servo.write(0);
      Serial.println("Cerrando");
    }else if(tiempoCierre==0){
      tiempoCierre=millis()-tiempoPorton;
      tiempoPorton=millis();
      servo.write(0);
      Serial.println("Cerrando");
    }
    if (tiempoPorton+tiempoCierre<millis()){
      Serial.println(millis());
      Serial.println("Cerrado");
      digitalWrite(Amarillo, HIGH);
      moverServo=porton=cierreInesperado=false;
      tiempoPorton=tiempoCierre =0;
     }
  }else
  if(!porton && moverServo){//si esta cerrado
     if(tiempoPorton==0){
      digitalWrite(Amarillo, LOW);
      Serial.println(millis());
      Serial.println("Abriendo");
      tiempoPorton=millis();
      servo.write(180);
     }else
     if (tiempoPorton+2100<millis()){
      Serial.println(millis());
      Serial.println("Abierto");
      porton=true;
      tiempoPorton=0;
     }else{
  //    Serial.println(millis());
     }
  }else if(moverServo) {//se abrio
    
    if(tiempoPorton ==0 ){
      tiempoPorton=millis();
      digitalWrite(Rojo, HIGH);
    }else if(tiempoPorton+2000<millis()&& digitalRead(Rojo)){
      digitalWrite(Rojo, LOW);
      servo.write(0);
      Serial.println("Cerrando");
    }else if(millis()>tiempoPorton+4100){
      digitalWrite(Amarillo, HIGH);
      moverServo=porton=false;
      tiempoPorton =0;
      Serial.println("Cerrado");
    }else{
//      Serial.println(millis());
     }
  }
}

void moverStepper(int direccion){
  // Movemos el motor un número determinado de pasos
  stepper1.step(direccion);
  stepper2.step(direccion*-1);
}

/*
*   Luces 
*/

void controladorAplicacion(){

    if(Serial.available()>0){
    entradaApp = Serial.read();
    Serial.println("entrada " + entradaApp);
    switch(entradaApp){
      case 'A': // Lab1 banda: A -> Encendido
        break;
      case 'B': // Lab1 banda: B -> Apagado
        break;
      case 'C': // Lab2 banda: C -> Encendido
        break;
      case 'D': // Lab2 banda: D -> Apagado 
        break;
      case 'E': // Abrir porton: E -> Encendido
        break;
      case 'F': // Cerrar porton: F -> Apagado
        break;
      case 'G': // Luces Lab1: G -> Encender
        digitalWrite(ledLab1, HIGH);
        break;
      case 'H': // Luces Lab1: H -> Apagar
        digitalWrite(ledLab1, LOW);
        break;
      case 'I': // Luces Lab2: I -> Encender
        digitalWrite(ledLab2, HIGH);
        break;
      case 'J': // Luces Lab2: J -> Apagar
        digitalWrite(ledLab2, LOW);
        break;
      case 'K': // Luces SalidaCamiones: K -> Encender
        digitalWrite(ledCamiones, HIGH);
        break; 
      case 'L': // Luces SalidaCamiones: L -> Apagar
        digitalWrite(ledCamiones, LOW);
        break; 
      case 'M': // Luces EntradaEmpleados: M -> Encender
        digitalWrite(ledEmpleados, HIGH);
        break;
      case 'N': // Luces EntradaEmpleados: N -> Apagar
        digitalWrite(ledEmpleados, LOW);
        break;
      case 'O': // Luces General: O -> Encender
        digitalWrite(ledLab1, HIGH);
        digitalWrite(ledLab2, HIGH);
        digitalWrite(ledCamiones, HIGH);
        digitalWrite(ledEmpleados, HIGH);
        break;
      case 'P': // Luces General: P -> Apagar
        digitalWrite(ledLab1, LOW);
        digitalWrite(ledLab2, LOW);
        digitalWrite(ledCamiones, LOW);
        digitalWrite(ledEmpleados, LOW);
        break;   
    }
    
  }
}
