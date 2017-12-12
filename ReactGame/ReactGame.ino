// Игра реакции. 
// Для тренировки внимания и быстроты реагирования совмещенная с челночным бегом.
// Креатед бай voltNik (c) в 2017 году нашей эры
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Utility.h>
#include "tones.h"

#define GAMETIME 180000 // длительность игры. 180 секунд. смена в хоккее.
#define LAMP 9       // количество кнопок в игре
#define RES_KEY 12  // номер кнопки для перезагрузки игры. RESET
#define BUZZER_PIN 14 // пин подключения пищалки

LiquidCrystal_I2C lcd(0x3F,16,2);  // обычно на китайских I2C экранах адреса 0x27 или 0x3F

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char hexaKeys[ROWS][COLS] = { // таблица символов клавиатуры. если отнять от кода символа 48, то будет просто номер нажатой цифры. 
  {'0','1','2','3'},          // т.е. номер кнопки. у "0" ascii код 48, минус 48 и получаем 0 цифрой, у "1" код 49 и т.д. костыль, но работает.   
  {'4','5','6','7'},
  {'8','9',':',';'},
  {'<','=','>','?'}
};

byte colPins[ROWS] = {39,37,35,33};  // пины к которым подключена клавиатура от 33 и далее вдоль края MEGA
byte rowPins[COLS] = {41,43,45,47};

boolean lamp_on[LAMP];
boolean steps = true, go_game = true;
int lamp_pin[LAMP] = {2,3,4,5,8,7,6,9,10}; // последовательность пинов к которым подключены светодиоды кнопок. тут можно перенастроить соответствие светодиод-кнопка. как видите они у меня не по порядку. так припаял.

long last_pressed = 0;
long nowMillis = 0;
long toSec = 0;
long gameStart = 0;

int score = 0;
int dlay = 1000;
int butn = 0;
int old_butn = 100; // предыдущая нажатая кнопка
char customKey;

int melody[] = { NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4 }; // мелодия Game Over
int noteDurations[] = { 4, 8, 8, 4, 4, 4, 4, 4 };

Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
//=====================================
void(* resetFunc) (void) = 0;  // функция ресета
//=====================================
void setup()
{
  Serial.begin(9600);
  Serial.println("REACT GAME");
  
  randomSeed(analogRead(0));  // включение случайного random
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_BUILTIN,OUTPUT);
  tone(BUZZER_PIN, 5000, 500);
  foreach (lamp_pin, LAMP, pinMode, OUTPUT);   // выводы на светодиоды кнопок
  foreach (lamp_pin, LAMP, digitalWrite, HIGH); // проверяем работу светодиодов 
  delay(2000);
  foreach (lamp_pin, LAMP, digitalWrite, LOW); 
  delay(500);
  lcd.init();         // заставка
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("   REACT GAME"); 
  delay(500);
  lcd.setCursor(0, 1);
  lcd.print(" START IN 3..."); 
  tone(BUZZER_PIN, 4000, 200);
  delay(500);
  lcd.setCursor(0, 1);
  lcd.print(" START IN 2..."); 
  tone(BUZZER_PIN, 4500, 200);
  delay(500);
  lcd.setCursor(0, 1);
  lcd.print(" START IN 1..."); 
  tone(BUZZER_PIN, 5000, 200);
  delay(500);
  lcd.setCursor(0, 0);
  lcd.print("  GO! GO! GO!"); 
  lcd.setCursor(0, 1);
  lcd.print("  GO! GO! GO!"); 
  for (int i=0; i<5; i++) {
    int frequency = 4000 + i * 300;
    tone(BUZZER_PIN, frequency, 150);
    delay(100);
  }
  //delay(500); 
  
  gameStart = millis();
  nowMillis = gameStart;
  score = 0;
  steps = true;
}
//=====================================
void loop()
{
  while ((nowMillis-gameStart<GAMETIME)and(go_game)) {
    nowMillis = millis();
    if (nowMillis - toSec > 1000) { // обновление экрана каждую секунду
      lcd_print();
      toSec = nowMillis; 
    } 
    if (steps) {   // выбираем новую кнопку для нажатия. любую кроме той же.

      while (old_butn == butn) {   
        butn = random(LAMP);
      }
      old_butn = butn;
      Serial.print("Butn: "); Serial.println(butn);
      sw_led_on(butn);
      steps = false;
    }
    customKey = customKeypad.getKey(); // считывание нажатия кнопки
    if (customKey) Serial.println((int)customKey);
    if (customKey-48 == butn){     // нажата правильная кнопка
       sw_led_off(customKey-48);
       score++;
       steps = true;   // включаем выбор новой кнопки
       lcd_print();
    } else if (customKey-48 == RES_KEY){   // нажат ресет
      gameStart = millis();
      foreach (lamp_pin, LAMP, digitalWrite, LOW); 
      score = 0;
      Serial.println("RESTART!");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("RESTART!");
      delay(500);
      resetFunc();
    } else {   // нажата неверная кнопка
      //lcd.setCursor(8, 1);
      //lcd.print(" WRONG!");
    }
 } // конец цикла игры
 if (go_game) game_over(); // время игры закончилось
 go_game = false;
 customKey = customKeypad.getKey();  
 if (customKey-48 == RES_KEY) { // перезапуск по RESET
   go_game = true;
   //gameStart = millis();
   //nowMillis = gameStart;
   //score = 0;
   //steps = true;

   lcd.clear();
   lcd.setCursor(0, 0);
   lcd.print("RESTART!");
   delay(500);
   resetFunc();
 }
}
//=====================================
void lcd_print(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TIME:");
  int time = GAMETIME/1000-(nowMillis - gameStart)/1000;
  lcd.print(time);
  if (time < 11) { tone(BUZZER_PIN, 3500+time*300, 150); }
  lcd.setCursor(11, 0);
  lcd.print("BTN:");
  lcd.print(customKey);
  lcd.setCursor(0, 1);
  lcd.print("SCORE:");
  lcd.print(score);
}
//=====================================
void game_over() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("    GAME OVER");
  lcd.setCursor(0, 1);
  lcd.print(" SCORE: ");
  lcd.print(score);
  foreach (lamp_pin, LAMP, digitalWrite, HIGH); 

  for (int thisNote = 0; thisNote < 8; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(BUZZER_PIN, melody[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(BUZZER_PIN);
  }
}
//=====================================
void sw_led_on(int num) {
    digitalWrite(lamp_pin[num],1); 
    lamp_on[num] = 1;
}
//=====================================
void sw_led_off(int num) {  
    digitalWrite(lamp_pin[num],0);
    lamp_on[num] = 0;
}
//=====================================
