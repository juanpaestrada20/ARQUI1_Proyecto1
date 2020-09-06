#include <EEPROM.h>
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

/* La variable cantidadUsuarios, registrada al principio de la eeprom determina el numero de usuarios que tenemos, esta variable nos ayudara a leer n cantidad de usuarios, 
   para no borrar en esta memoria  */
unsigned int cantidadUsuarios, conteoIntentos;

//Struct para la representacion de los usuarios
struct usuario {
  char id[5];
  char password[5];
};

//Creamos un auxiliar para obtener el auxEntrada
String auxEntrada = "", idUser = "", pwdUser = "";

bool errorContrasenia = false, sesionIniciada = false, esRegistro = false;

//Pines de la luces del acceso
#define UserPermitido 11
#define UserBloqueo 12
#define Bocina 13

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

  delay(1000);

  //Teclado de empieza xd
  teclado.begin();


  //Pines del Porton
  pinMode(Rojo, OUTPUT);
  pinMode(Amarillo, OUTPUT);
  pinMode(pinServo, OUTPUT);
  pinMode(Abrir, INPUT);
  pinMode(Cerrar, INPUT);

  //Pines de laboratorio
  pinMode(Lab1, INPUT);
  pinMode(Lab2, INPUT);

  //Pines de los leds del usuario y la Bocina
  pinMode(Bocina, OUTPUT);
  pinMode(UserPermitido, OUTPUT);
  pinMode(UserBloqueo, OUTPUT);

  //Pines de Stepper con Driver
  /*pinMode(ST1B, OUTPUT);
  pinMode(ST2B, OUTPUT);
  pinMode(ST3B, OUTPUT);
  pinMode(ST4B, OUTPUT);*/

  // Asignamos la velocidad en RPM (Revoluciones por Minuto)
  stepper1.setSpeed(10);
  stepper2.setSpeed(10);


  //Esta funcion nos ayudara unicamente a ver por primera vez si la EEPROM esta completamente vacia, si hay o no cantidades de usuarios

  /*CUIDADO: SI ESTA SENTENCIA NO ESTA PUESTA COMO COMEnTARIO, SOLO DEFINIRA QUE NO HAY USUARIOS*/
  //EEPROM.put(0, 0);

  //Pines de las luces
  pinMode(ledLab1,OUTPUT);
  pinMode(ledLab2,OUTPUT);
  pinMode(ledCamiones,OUTPUT);
  pinMode(ledEmpleados,OUTPUT);

  cantidadUsuarios = 0;
  EEPROM.get(0, cantidadUsuarios);
  
  if (cantidadUsuarios == 0) {
    //Seteamos la existencia de un unico usuario (ADMIN)
    cantidadUsuarios = 1;
    //Escribimos la cantidad de usuarios
    EEPROM.put(0, cantidadUsuarios);

    //Creamos al ADMIN
    usuario admin = {
      "2018",
      "0106"
    };

    int tamanioCant = sizeof(cantidadUsuarios);
    EEPROM.put(tamanioCant, cantidadUsuarios);
  }

}

void loop() {
  //Aqui inicia la sesion xd
  if (!sesionIniciada) {
    
    if (esRegistro) {
      if (pwdUser == "") {
        lcd.setCursor(0,0);
        lcd.write("Ingrese su nueva");
        lcd.setCursor(0,1);
        lcd.write("contrasena");
      } else {
        lcd.setCursor(0,0);
        lcd.write("Confirme la contrasena");
      }
    } else if (idUser != ""){
      lcd.setCursor(0,0);
      lcd.write("Ingrese su contrasena");
    } else {
      lcd.setCursor(0,0);
      lcd.write("Ingrese su id");
    }

    //Validaciones sobre el teclado
    teclado.tick();
    if(teclado.available()){
      keypadEvent e = teclado.read();
      if (e.bit.EVENT == KEY_JUST_PRESSED) auxEntrada += (char)e.bit.KEY;
    }

    lcd.setCursor(0,1);
    lcd.print(auxEntrada);

    //Contraseña y validaciones
    if (auxEntrada.length() == 4) {
      //Vemos si no se ha intentado registrar poniendo 0000, aqui empieza el registro
      if (auxEntrada == "0000" && !esRegistro  && idUser == ""){
        esRegistro = true;
      }
      //Aqui empieza el login del sistema
      else if (esRegistro) {
        if(pwdUser == "") {
          pwdUser = auxEntrada;
        }
        else if ( auxEntrada ==  pwdUser ){
          nuevoUsuario(auxEntrada);
          pwdUser = "";
          esRegistro = false;
        }
        else {
          lcd.setCursor(0,0);
          lcd.write("La contrasena debe coincidir");
          delay(1500);
        }
      }
      //Aqui empieza el login del sistema
      else if (idUser == "") {
        idUser = auxEntrada;
        
      } 
      else {
          bool esCorrecto = buscarUsuario(idUser, auxEntrada);

          idUser = "";

          if (esCorrecto) {
            sesionIniciada = true;
            conteoIntentos = 0;

            sonarBocina(2000);

            digitalWrite(UserPermitido, HIGH);
          } else {
            conteoIntentos++;
            Serial.println("Contraseña incorrecta");
          }

      }

    //Siempre limpiamos la entrada
      lcd.clear();
      auxEntrada = "";
      if(conteoIntentos >= 3) {
        
        conteoIntentos = 0;
        
        sonarBocina(5000);

        digitalWrite(UserBloqueo, HIGH);

        String auxAdmin = "";
        while(true) {
          //Teclado admin
          teclado.tick();
          if(teclado.available()){
            keypadEvent e = teclado.read();
            if (e.bit.EVENT == KEY_JUST_PRESSED) auxAdmin += (char)e.bit.KEY;
          }

          if (auxAdmin.length() == 4) {
            if (auxAdmin == "0106") break;
            else auxAdmin = "";
          }
        }

        digitalWrite(UserBloqueo, LOW);
          
      }
    }


  } 
  else {
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
    //Control App
    controladorAplicacion();
  }
  
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

bool buscarUsuario(String id, String password) {
  for (int indexUsuario = 0; indexUsuario  < cantidadUsuarios; indexUsuario++)
  {
    //Leemos cada usuario disponible
    usuario user;
    EEPROM.get(sizeof(cantidadUsuarios) + sizeof(user) * indexUsuario, user);

    String IdUser(user.id);
    String PwdUser(user.password);
    if (IdUser == id && PwdUser == password) return true;
  }

  return false;
}

void nuevoUsuario(String password) {
  //Creamos una variable de tipo string para poder hacer mejor el id
  String idString; 
  if (cantidadUsuarios < 9) idString = "000";
  else if(cantidadUsuarios < 99) idString = "00";
  else if (cantidadUsuarios < 999) idString = "0";
  else idString = "";

  idString += "" + (cantidadUsuarios+1);
  
  //Ahora guardamos el Id en un array de bytes
  char idB[5];
  idString.toCharArray(idB, sizeof(idB));

  //Creamos un variable para guardar la password
  char pwd[5];
  password.toCharArray(pwd, sizeof(pwd));

  //Creamos un nuevo usuario
  usuario user = {
    {idB},
    {pwd}
  };

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("El admin debe");
  lcd.setCursor(0,1);
  lcd.print("autorizar");

  //Mostramos el nuevo id creado usando el lcd para ello
  String auxAdmin = "";
  while(true) {
    //Teclado admin
    teclado.tick();
    if(teclado.available()){
      keypadEvent e = teclado.read();
      if (e.bit.EVENT == KEY_JUST_PRESSED) auxAdmin += (char)e.bit.KEY;
    }

    if (auxAdmin.length() == 4) {
      if (auxAdmin == "0106") break;
      else auxAdmin = "";
    }

  }

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Registro exitoso");
  lcd.setCursor(0,1);
  lcd.print("ID: ");
  lcd.print(idString);

  unsigned long auxTiempo = millis();
  while (abs(millis() - auxTiempo) < 1500)
  {
    delay(1);
  }

  //Guardamos su posicion segun el espacio indicado por la cantidad actual de usuarios debido que el elemento An esta en n-1
  EEPROM.put(sizeof(cantidadUsuarios) + sizeof(user) * cantidadUsuarios, user);

  //Incrementamos y guardamos el valor de la cantidad de usuarios
  EEPROM.put(0, ++cantidadUsuarios);
  
}

//La funcion hace funcionar la bocina n cantidad de ms
void sonarBocina(int tiempo){
  unsigned long auxTiempo = millis();
  while ( abs(millis() - auxTiempo) < tiempo ) {
    digitalWrite(Bocina, HIGH);
    delay(1);
  }

  digitalWrite(Bocina, LOW);
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