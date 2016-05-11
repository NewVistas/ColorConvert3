/*
	ColorConvert3.h - Library for converting between RGB and RGBW values for use in multicolored LEDs.
	Created by H. Tracy Hall, 4-22-2016.
	Converted to Arduino library by Nate Hopkins, 5-2-2016.
	Adapted to support 3-part manufacturer calibration by H. Tracy Hall, 5-4-2016.
	Error fixes by Nate Hopkins, 5-5-2016.
*/

#include "Arduino.h"
#include "ColorConvert3.h"

// Construct a ColorConvert3 object with some default values.
ColorConvert::ColorConvert()
{
	rEquiv = 625.0;
	gEquiv = 400.0;
	bEquiv = 223.0;
	wEquiv = 625.0;
	over = 0.6;
}

// Set a basic RGB-to-white equivalence. The input values should be the result of
// a calibration showing that the RGBW quadruple (rgbValue, rgbValue, rgbValue, 0)
// is approximately as bright as the RGBW quadruble (0, 0, 0, whiteValue).
// The numbers should be strictly positive, and in particular trying to use 0.0
// for any of them can give a division by zero error.
void ColorConvert::setWhiteEquiv(float rgbValue, float whiteValue)
{
	wEquiv = 255.0*rgbValue/whiteValue;
	rEquiv = wEquiv;
	gEquiv = wEquiv;
	bEquiv = wEquiv;
}

// Set a more precise manufacturer-specific RBG-to-white equivalence. The input
// values should be the result of a calibration showing that the RGBW quadruple
// (redValue, greenValue, blueValue, 0) is the same brightness and color as the
// RGBW quadruple (0, 0, 0, whiteValue). All values should be positive and in particular
// trying to use 0.0 can result in division by zero.
void ColorConvert::setRgbWhiteEquiv(float redValue, float greenValue, float blueValue, float whiteValue)
{
	rEquiv = 255.0*redValue/whiteValue;
	gEquiv = 255.0*greenValue/whiteValue;
	bEquiv = 255.0*blueValue/whiteValue;
	wEquiv = max(rEquiv, max(gEquiv, bEquiv));
}

// Set a value in the range from slightly negative to 1.0 which specifies how much of the additional
// brightness gamut of RGBW vs. RGB to exploit. If overdrive is 0.0, then colors should look mostly
// the same before and after conversion, except that the white point will be more reliably white
// regardless of part-to-part variation ("white is white"). If overdrive is 1.0, then less saturated
// (whiter) colors will be as bright as possible, brighter than just RGB can do ("white is bright").
// If the calibration results yield rgbValue > whiteValue, then pure white without part-to-part
// variation requires a slightly negative value of overdrive, decreasing the overall gamut and
// possible brightness to less than what is possible even with just RGB. In general, if the
// quadruple (rgbValue, rgbValue, rgbValue, 0) is as bright as (0, 0, 0, whiteValue), then setting
// overdrive to any value up to 1 - whiteValue/rgbValue ensures that pure white is displayed without
// using the RGB sector, and setting it to exactly that value makes RGB (255, 255, 255) display as
// RGBW (0, 0, 0, 255).
void ColorConvert::setOverdrive(float overdrive)
{
	over = overdrive;
}

// For the following description, consider a led strip where white at 255 outshines the brightest white you can
// achieve with rgb alone. This function called with your calibrated white equivalent value will return the
// highest overdrive value you can set with setOverdrive(), that will give a pure white output (no r,g,b).
// Once you exceed this overdrive value, rgbToRgbw will start adding rgb leds to the (full power) white to
// output more light.
float ColorConvert::getMaxUnstretchedOverdrive(float whiteValue)
{
	return (1-(whiteValue/255));
}

// Convert a theoretical RGB value to a device RGBW value
 ColorRgbw ColorConvert::rgbToRgbw(ColorRgb inColor) {
	// The return value
	ColorRgbw outColor;

	// max and min values
	int high;
	float thisShift;
	// Replaced max with if-chain to set thisShift at the same time
	if (inColor.green > inColor.red)
	{
		high = inColor.green;
		thisShift = wEquiv/gEquiv;
	}
	else
	{
		high = inColor.red;
		thisShift = wEquiv/rEquiv;
	}
	if (inColor.blue > high)
	{
		high = inColor.blue;
		thisShift = wEquiv/bEquiv;
	}
	// int high = max(inColor.red, max(inColor.green, inColor.blue));
	// Pre-empt division by zero.
	if (high < 1) {
		outColor.red = 0;
		outColor.green = 0;
		outColor.blue = 0;
		outColor.white = 0;
		return outColor;
	}
	int low = min(inColor.red, min(inColor.green, inColor.blue));

	float saturation = (high - low) / (float) high;

	// First we expand the RGB gamut out to a ficticious range whose brightest point,
	// in RGB space, looks the same as (255, 255, 255, overdrive*255) in RGBW space.
	float ficR, ficG, ficB;

	// How far out in the minimum color direction does the expanded gamut stretch?
	float maxShift = over * wEquiv;

	// The most saturated colors cannot be stretched as far into the new gamut.
	// This transformation makes maximal use of the allowed expanded gamut, but it
	// has a discontinuous derivative, which could cause noticeable artifacts when
	// applied to pixels of a smooth gradient that crosses the saturation cutoff.
	float stretch;
	if (saturation * (255 + maxShift) < thisShift * 255.0){ // low saturation, max stretch
		stretch = (255 + maxShift) / 255.0;
	}
	else{ // high saturation, limited stretch
		stretch = thisShift / saturation;
	}
	ficR = stretch * inColor.red;
	ficG = stretch * inColor.green;
	ficB = stretch * inColor.blue;

	// Now we replace as much RGB as possible with the equivalent amount of W, using
	// the fact that the kernel vector (-wEquiv, -wEquiv, -wEquiv, 255)
	// can be added in any amount without affecting appearance.

	// This will be the amount to shift the RGB values. Take min to prevent W overflow.
	float ficLow = min(stretch * low, wEquiv);

	// We also shrink from ideal RGB values to device values. The result should take
	// values that may exceed 255 down to 255 at the most.
	outColor.red = int((ficR - ficLow)*rEquiv/wEquiv + 0.5);
	outColor.green = int((ficG - ficLow)*gEquiv/wEquiv + 0.5);
	outColor.blue = int((ficB - ficLow)*bEquiv/wEquiv + 0.5);
	outColor.white = int(ficLow * 255.0 / wEquiv + 0.5);
	
	return outColor;
 }