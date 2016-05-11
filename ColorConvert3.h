/*
	ColorConvert3.h - Library for converting between RGB and RGBW values for use in multicolored LEDs.
	Created by H. Tracy Hall, 4-22-2016.
	Converted to Arduino library by Nate Hopkins, 5-2-2016.
	Adapted to support 3-part manufacturer calibration by H. Tracy Hall, 5-4-2016.
	Error fixes by Nate Hopkins, 5-5-2016.
*/

#ifndef ColorConvert3_h
#define ColorConvert3_h

#include "Arduino.h"

// 0-255 based RGB color triplet
struct ColorRgb{
	unsigned int red;
	unsigned int green;
	unsigned int blue;
};

// 0-255 based RGBW color quadruplet
struct ColorRgbw{
	unsigned int red;
	unsigned int green;
	unsigned int blue;
	unsigned int white;
};

class ColorConvert
{
	public:
		ColorConvert();
		ColorRgbw rgbToRgbw(ColorRgb inColor);
		void setWhiteEquiv(float rgbValue, float whiteValue);
		void setRgbWhiteEquiv(float redValue, float greenValue, float blueValue, float whiteValue);
		void setOverdrive(float overdrive);
		float getMaxUnstretchedOverdrive(float overdrive);
	private:
		float rEquiv;
		float gEquiv;
		float bEquiv;
		float wEquiv;
		float over;
};

#endif