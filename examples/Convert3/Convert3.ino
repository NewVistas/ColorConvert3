#include <ColorConvert3.h>

ColorConvert converter;
ColorRgb input;
ColorRgbw output;

void setup() {

  Serial.begin(9600);
  input.red = 20;
  input.green = 30;
  input.blue = 40;
  
  converter.setRgbWhiteEquiv(255, 188, 100, 124);
  converter.setOverdrive(0.6);

  output = converter.rgbToRgbw(input);

  Serial.println("Red: ");
  Serial.println(output.red);
  Serial.println("Green: ");
  Serial.println(output.green);
  Serial.println("Blue: ");
  Serial.println(output.blue);
  Serial.println("White: ");
  Serial.println(output.white);
}

void loop() {};
