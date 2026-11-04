/**
 * 
  ps2_kbd.h  PS/2 Keyboard interface of "Hobo-nicola PS/2 adapter" using Hobo-nicola keyboard and adapter library.
  Copyright (c) 2018 Takeshi Higasa
  
  This file is part of "Hobo-nicola keyboard and adapter".

  "Hobo-nicola keyboard and adapter" is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  "Hobo-nicola keyboard and adapter" is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with "Hobo-nicola keyboard and adapter".  If not, see <http://www.gnu.org/licenses/>.

	Included in hoboNicola 1.6.4.		Feb. 1. 2023.

*/

/*
  - Added class oa30_kbd for the OASYS30‑AX301 keyboard.
  - Renamed the files to "oa30.cpp" and "oa30.h".

   sumio.koseki (JA7RHK)	2026.8/29 
*/

#include <Arduino.h>

class keyboard_notify {
public:
	virtual void key_pressed(uint8_t k, uint8_t mod) {}; // notify key pressed with hid usage id.
	virtual void key_released(uint8_t k, uint8_t mod) {}; // notify key released with hid usage id.
	virtual void modifier_changed(uint8_t before, uint8_t after) {}; // notify any modifier-key is changed.
	virtual void raw_key_event(uint8_t scan, bool released = false) {};  // notify all key event with raw scancode.
	virtual void key_released_all() {}; // all keys are released.
};

#define MAX_KEYS 8

class oa30_kbd {
public:
	static oa30_kbd* getInstance();
	bool begin(keyboard_notify* p = 0, uint8_t led = 0xff);
	void task(uint8_t led = 0);  // do everything.
	void kbd_loop();

private:
	oa30_kbd(); 
	uint8_t rdata[MAX_KEYS] = {0};
	uint8_t attr = 0;   // 属性
	uint8_t code = 0;   // キーコード
	bool attrReceived = false;

	uint8_t modifier_state;
	keyboard_notify* notifier;

	int8_t getZeroIndex();
	int8_t getMatchingIndex(uint8_t value);
	int8_t getNonZeroIndex();
	void clearRdata();
    uint8_t hid_code(uint8_t k, bool& released);
};
