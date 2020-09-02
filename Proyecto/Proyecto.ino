#include <LiquidCrystal.h>
#include <Adafruit_Keypad.h>

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

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(20, 2);
  // Print a message to the LCD.
  lcd.print("Bienvenido");

  //Teclado de empieza xd
  teclado.begin();

  //Nos posicionamos en la columna 0 y fila 1
  lcd.setCursor(0, 1);
}

void loop() {
  teclado.tick();

  while(teclado.available()){
    keypadEvent e = teclado.read();
    if (e.bit.EVENT == KEY_JUST_PRESSED) lcd.print((char)e.bit.KEY);
  }

  

}
