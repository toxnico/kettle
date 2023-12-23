#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <dmtimer.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Bounce2.h>
#include "config.h"
#include "status.h"
#include "assets/assets.h"
#include <Adafruit_NeoPixel.h>

using namespace Bounce2;

Adafruit_SSD1306 *disp = NULL;

KettleStatus_t status;

Button btnProg;
Button btnMinus;
Button btnPlus;
Button btnOnOff;

DMTimer tmrReadTemperature(READ_TEMP_INTERVAL_US);
DMTimer tmrAutoOff(AUTO_OFF_TIMEOUT_US);

int presets[6] = {70, 75, 80, 85, 90, 90};
int presetIdx = 0;

Adafruit_NeoPixel leds = Adafruit_NeoPixel(2, PIN_LEDS, NEO_GRB + NEO_KHZ800);

void initButtons()
{

  btnOnOff.attach(PIN_BTN_ON_OFF, INPUT_PULLUP);
  btnMinus.attach(PIN_BTN_MINUS, INPUT_PULLUP);
  btnPlus.attach(PIN_BTN_PLUS, INPUT_PULLUP);
  btnProg.attach(PIN_BTN_PROG, INPUT_PULLUP);

  btnOnOff.setPressedState(LOW);
  btnMinus.setPressedState(LOW);
  btnPlus.setPressedState(LOW);
  btnProg.setPressedState(LOW);
}

void setup()
{

  presetIdx = 0;
  status.currentTemperature = 72;
  status.targetTemperature = presets[0];
  // status.isHeating = true;

  analogReadResolution(12);

  Wire.setSDA(PIN_SDA);
  Wire.setSCL(PIN_SCL);

  pinMode(25, OUTPUT);

  initButtons();

  disp = new Adafruit_SSD1306(128, 64, &Wire, -1, 800000 * 2, 100000);
  disp->begin(SSD1306_SWITCHCAPVCC, 0x3C);
  disp->setFont(&FreeSans12pt7b);
  disp->setTextColor(WHITE);
  disp->clearDisplay();

  pinMode(PIN_RELAY_A, OUTPUT);
  pinMode(PIN_RELAY_B, OUTPUT);

  digitalWrite(PIN_RELAY_A, false);
  digitalWrite(PIN_RELAY_B, false);
}

void displayPresetDot()
{
  Point_t points[6];
  points[0].x = 2;
  points[0].y = 2;
  points[1].x = 63;
  points[1].y = 2;
  points[2].x = 125;
  points[2].y = 2;
  points[3].x = 125;
  points[3].y = 60;
  points[4].x = 63;
  points[4].y = 60;
  points[5].x = 2;
  points[5].y = 60;

  // find current index:
  int idx = -1;
  for (int i = 0; i < 6; i++)
  {
    if (presets[i] == (int)status.targetTemperature)
    {
      idx = i;
      break;
    }
  }

  if (idx < 0)
    return;

  auto pt = points[idx];

  disp->fillCircle(pt.x, pt.y, 2, WHITE);
}

void display()
{
  disp->clearDisplay();

  disp->setCursor(0, 27);
  disp->printf("%.1f", status.currentTemperature);

  disp->drawLine(0, 32, 64, 32, WHITE);

  disp->setCursor(0, 53);
  disp->printf("%.0f", status.targetTemperature);

  disp->setTextSize(1);
  disp->setCursor(25, 63);
  /*
    int rawTemp = analogRead(PIN_TEMPERATURE);
    if(rawTemp < 200)
      rawTemp = 0;
    rawTemp = (rawTemp / 100) * 100;

    disp->print(rawTemp);
  */

  if (status.isHeating)
  {
    disp->drawBitmap(92, 10, epd_bitmap_kettle_icon, 35, 40, WHITE);
  }

  displayPresetDot();

  disp->display();
}

void enableHeater(bool enable)
{

  digitalWrite(PIN_RELAY_A, enable);
  digitalWrite(PIN_RELAY_B, enable);

  if (status.isHeating)
  {
    leds.setPixelColor(0, leds.Color(200, 0, 0));
    leds.setPixelColor(1, leds.Color(200, 0, 0));
    leds.show();
  }
  else
  {
    leds.setPixelColor(0, leds.Color(0, 0, 0));
    leds.setPixelColor(1, leds.Color(0, 0, 0));
    leds.show();
  }
}

void beep()
{
  int duration = 2000;
  int period = 200;

  for (int i = 0; i < duration / 200; i++)
  {
    digitalWrite(PIN_RELAY_A, 1);
    delay(period / 2);
    digitalWrite(PIN_RELAY_A, 0);
    delay(period / 2);
  }
}

float readTemperature()
{
  int sampleCount = 50;
  int raw = 0;
  for (int i = 0; i < sampleCount; i++)
  {
    raw += analogRead(PIN_TEMPERATURE);
  }
  raw = raw / sampleCount;

  float degrees = ((float)raw) * TEMP_CONVERSION_A + TEMP_CONVERSION_B;

  if (degrees < MINIMUM_VALID_TEMPERATURE)
    degrees = MINIMUM_VALID_TEMPERATURE;
  //return 72;
  return (int)degrees;
}

void off()
{
  status.isOn = false;
  presetIdx = 0;
  status.targetTemperature = presets[presetIdx];
}

void handleBtnProg()
{
  btnProg.update();

  if (!btnProg.pressed())
    return;

  tmrAutoOff.reset();
  Serial.printf("Preset : %d", presetIdx);

  // if we are waking up the kettle, no increment of the current preset
  bool wasOn = status.isOn;

  status.isOn = true;
  if (wasOn)
  {
    presetIdx++;
    if (presetIdx > 5)
      presetIdx = 0;
  }
  status.targetTemperature = presets[presetIdx];
}

void handleBtnOnOff()
{
  btnOnOff.update();

  if (btnOnOff.pressed())
  {
    tmrAutoOff.reset();
    
    if(status.isOn)
      off();
    else
      status.isOn = true;
    
    //status.isOn = !status.isOn;
    
  }
}

void handleBtnPlus()
{
  btnPlus.update();

  if (btnPlus.pressed())
  {
    tmrAutoOff.reset();
    status.targetTemperature = constrain(status.targetTemperature + 5, ABSOLUTE_MIN_TEMPERATURE, ABSOLUTE_MAX_TEMPERATURE);
  }
}

void handleBtnMinus()
{
  btnMinus.update();

  if (btnMinus.pressed())
  {
    tmrAutoOff.reset();
    status.targetTemperature = constrain(status.targetTemperature - 5, ABSOLUTE_MIN_TEMPERATURE, ABSOLUTE_MAX_TEMPERATURE);
  }
}

void displayScreenSaver()
{

  static int movingDotX = 0;
  static int movingDotY = 0;

  movingDotX = (movingDotX + 1) % 127;
  if (movingDotX == 0)
    movingDotY = (movingDotY + 1) % 64;

  disp->clearDisplay();
  disp->drawPixel(movingDotX, movingDotY, WHITE);

  disp->display();
}


/*
bool mustHeat()
{
  //controller is off ?
  if(!status.isOn)
    return false;

  //temperature reached?
  if(status.currentTemperature >= status.targetTemperature)
    return false;

  //temperature below the target (considering hysteresis threshold)
  if(status.currentTemperature <= status.targetTemperature - TEMPERATURE_HYSTERESIS)
    return true;
  
  //by default, for security:
  return false;
}
*/

void loop()
{

  handleBtnProg();
  handleBtnOnOff();
  handleBtnMinus();
  handleBtnPlus();

  //timeout, or kettle not on place
  if (tmrAutoOff.isTimeReached() || readTemperature() <= MINIMUM_VALID_TEMPERATURE)
  {
    off();
  }

  //read temp every few seconds
  if (tmrReadTemperature.isTimeReached())
    status.currentTemperature = readTemperature();

  if (!status.isOn)
  {
    status.isHeating = false;
    enableHeater(false);
    displayScreenSaver();
    return;
  }

  display();

  //status.isHeating = mustHeat();

  //hot enough?
  if (status.currentTemperature > status.targetTemperature)
    status.isHeating = false;

  //too cold?
  if (status.currentTemperature < status.targetTemperature - TEMPERATURE_HYSTERESIS)
    status.isHeating = true;

  //invalid temperature?
  if (status.currentTemperature <= MINIMUM_VALID_TEMPERATURE)
    status.isHeating = false;
  

  enableHeater(status.isHeating);
}
