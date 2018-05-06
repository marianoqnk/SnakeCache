#include <Wire.h> //libreria I2C
#include "Adafruit_LEDBackpack.h" //Led 8X8
#include "Adafruit_GFX.h"

#define LONG_MAX 15  //maxima longitud de la serpiente
#define DERECHA 5 //pin de la tecla derecha
#define IZQUIERDA 2 //pin de la tecla izquierda
#define LEDD 4
#define LEDI 3
#define VMAX 650.0 //Velocidad máxima de la serpiente
#define VMIN 180.0 //velocidad mínima
#define STEPVEL (VMAX-VMIN)/LONG_MAX


Adafruit_8x8matrix matrix = Adafruit_8x8matrix();
struct {
  char fila;
  char columna;
} serpiente[LONG_MAX], movimiento, fruta;
enum {Left = 1, Right = 2, Up = 3, Down = 4} direccion;
boolean hayFruta = false;
int intervaloFrutaActual;
char longActual = 1;
int velocidad = VMAX;//250
char coordenadas[] = "N 40 19.755  W 03 43.412 GAME OVER";

static const uint8_t PROGMEM
smile_bmp[] =
{ B00111100,
  B01000010,
  B10100101,
  B10000001,
  B10100101,
  B10011001,
  B01000010,
  B00111100
},
neutral_bmp[] =
{ B00111100,
  B01000010,
  B10100101,
  B10000001,
  B10111101,
  B10000001,
  B01000010,
  B00111100
},
frown_bmp[] =
{ B00111100,
  B01000010,
  B10100101,
  B10000001,
  B10011001,
  B10100101,
  B01000010,
  B00111100
};

void setup() {

  //Serial.begin(9600);
  matrix.begin(0x70);  // direccion I2C display
  pinMode(DERECHA, INPUT_PULLUP);
  pinMode(IZQUIERDA, INPUT_PULLUP);
  pinMode(LEDD, OUTPUT);
  pinMode(LEDI, OUTPUT);
  digitalWrite(LEDD, HIGH);
  digitalWrite(LEDI, HIGH);
  randomSeed(analogRead(4));
  matrix.setRotation(1);
  matrix.setTextSize(1);
  matrix.setTextWrap(false);  // we dont want text to wrap so it scrolls nicely
  matrix.setTextColor(LED_ON);
  /*scrollMensaje("Bienvenido a ARDUINO GAMES I");
    scrollMensaje("SNAKE ");
    scrollMensaje("Tienes que comerte al menos 10 manzanas");
    scrollMensaje("GO");*/
  //escribe();
  iniJuego();
}

void loop() {

  //LEER 250MSG

  long tiempo = millis();
  //Serial.println(tiempo);
  boolean teclaDer,teclaIzda;
  while ((((millis() - tiempo) < velocidad) && (teclaDer=digitalRead(DERECHA)) && (teclaIzda=digitalRead(IZQUIERDA)))); //espera a pulsar o timeout
  if (!teclaDer || !teclaIzda) // si es porque se ha pulsado 
    direccion = (direccion == Right || direccion == Left) ? (teclaDer ?Down : Up) : (teclaDer ? Left : Right);  
  mueve();
  dibujaSerpiente();
  frutas();
  int resto = velocidad - (millis() - tiempo);
  delay(20);//delay((resto>20)?resto:20);//conviene esperar al resto o vale con soltar
  tiempo = millis(); //espera a soltar
  while ((((millis() - tiempo) < 250) && ((!digitalRead(DERECHA)) || (!digitalRead(IZQUIERDA))))); //espera dejar pulsar
  delay(20);
}

void iniJuego() //Comienzo del juego
{
  //Serial.println("Ini Juego");
  serpiente[0].fila = serpiente[0].columna = 4;
  direccion = Right;
  movimiento.fila = 1;
  movimiento.columna = 0;
  longActual = 1;
  hayFruta = false;
  velocidad = VMAX;
}

inline void mueve() //Mueve la serpiente
{
  //Serial.println("mueve" + String ((int)longActual));
  //Serial.println ("fila:" + String((int)serpiente[0].fila) + " Col:" + String((int)serpiente[0].columna));
  for (char n = longActual; n >= 1; n--) serpiente[n] = serpiente[n - 1];
  serpiente[0].fila +=(direccion == Up || direccion == Down) ? ((direccion == Up) ? 1 : -1) : 0;
  serpiente[0].columna +=(direccion == Right || direccion == Left) ? ((direccion == Right) ? 1 : -1) : 0;
  if (fueraTablero() || estaEnSerpiente(serpiente[0].fila, serpiente[0].columna, 1 )) //si se sale o choca se acaba
  {
    matrix.clear();
    matrix.drawBitmap(0, 0, frown_bmp, 8, 8, LED_ON);
    matrix.writeDisplay();
    delay(3000);
    iniJuego();
  }
}
inline void dibujaSerpiente()//pinta la serpiente
{
  //Serial.println("Dibuja");
  matrix.clear();      // clear display
  for (char n = 0; n < longActual; n++)matrix.drawPixel(serpiente[n].fila, serpiente[n].columna, LED_ON);
  if (hayFruta)matrix.drawPixel(fruta.fila, fruta.columna, LED_ON); //dibuja fruta
  matrix.writeDisplay();

}
inline void frutas()//Se encarga de ir sacando las frutas
{

  if (hayFruta)
  {
    if ((serpiente[0].fila == fruta.fila) && (serpiente[0].columna == fruta.columna))
    {
      longActual++; //si se la ha comido crece la serpiente
      velocidad -= STEPVEL;
      if (longActual == LONG_MAX)hasGanado(); //Si es lo suficientemente grande se acabo el juego
      hayFruta = false;
    }

  }
  else
  {
    do {
      fruta.fila = random(8);
      fruta.columna = random(8);
    } while (estaEnSerpiente(fruta.fila, fruta.columna, 0));
    hayFruta = true;
  }

}

inline boolean fueraTablero() //true si el punto esta fuera del tablero
{
  return (serpiente[0].columna > 7) || (serpiente[0].columna < 0) || (serpiente[0].fila > 7) || (serpiente[0].fila < 0);
}

inline boolean estaEnSerpiente(char fila, char columna, char inicio) //true si el punto esta dentro de la serpiente inicio=0 incluye cabeza 1 sin cabeza
{
  for (char n = inicio; n < longActual; n++)if ((serpiente[n].fila == fila) && (serpiente[n].columna == columna))return true;
  return (false);
}
inline void hasGanado()  //Funcion cuando has ganado la  partida sonrie y saca coordenadas
{
  matrix.clear();
  matrix.drawBitmap(0, 0, smile_bmp, 8, 8, LED_ON);
  matrix.writeDisplay();
  delay(3000);
  while (1)  scrollMensaje(coordenadas);
}

void scrollMensaje(char mensaje[])
{
  int longitud = 0;
  while (mensaje[longitud])longitud++;
  int lineas = longitud * 6;
  for (int x = 0; x >= -lineas; x--) {
    matrix.clear();
    matrix.setCursor(x, 0);
    matrix.print(mensaje);
    matrix.writeDisplay();
    delay(100);
  }
}

void escribe()
{
  char nombre[] = "AAAAAA";
  char n = 0;
  char letra = 65;
  while (1) {
    matrix.clear();
    matrix.setCursor(0, 0);
    matrix.write(letra);
    matrix.writeDisplay();
    while ((digitalRead(DERECHA)) && (digitalRead(IZQUIERDA)));
    if (digitalRead(IZQUIERDA)) { //Siguiente letra
      if (letra == 91)n--; //vuelta a empezar
      nombre[n] = letra;
      n++;
      if (n == 6) break;
    } else
    {
      letra++;
      if (letra == 92)letra = 65;
    }
    delay(500);
  }
  nombre[6] = 0;
  while (1)scrollMensaje(nombre);
  while ((digitalRead(DERECHA)) && (digitalRead(IZQUIERDA)));
  if (digitalRead(IZQUIERDA))n = 6; else n = 9;
  //65 a 93
}

