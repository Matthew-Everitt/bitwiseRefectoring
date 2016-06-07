#pragma once

#include <stdint.h>
#include <U8glib.h>
#include <Encoder.h>


#include "debounce.h"
#include "pinout.h"



class display_t : public U8GLIB_PCD8544 {

private:
	typedef struct {
		uint8_t fontHeight;
		uint8_t fontWidth;
		uint8_t infoBarHeight;
		uint8_t nRows;
		uint8_t nCols;
		uint8_t before;
		uint8_t after;
	} properties_t;

	void updateEncoder();
	void updateMainDisplay();
	void drawInfoBar();

	debounceISR_proto(encButt);
	debounceISR_proto(playBut);
	debounceISR_proto(backBut);

	static Encoder rotaryEncoder;
	


public:
	display_t(uint8_t sck, uint8_t mosi, uint8_t cs, uint8_t a0, uint8_t reset);
	void update();
	properties_t properties;

};

extern display_t display;