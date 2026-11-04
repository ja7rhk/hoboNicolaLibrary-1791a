/**
 * 
	ps2_hobo_nicola.ino Main sketch of "Hobo-nicola PS/2 adapter" using Hobo-nicola keyboard and adapter library.
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
 
  hoboNicolaLibrary 1.7.8   Nov. 30. 2025
*/

/*
  - EAdded class oa30_kbd for the OASYS30-AX301 keyboard.
  - ERenamed the files to "oa30.cpp" and "oa30.h".

   sumio.koseki (JA7RHK)	2026.8/29 
*/

/**koseki(2024.3.21)

	設定モードに入るには 右CTRL + MENU(App) + 100ms -> 'S'キー

	● OASYS30-AX301キーボードの設定モード; *2 + *N
       ^^^^^^^^^^^^^^^^^
	---------------------------------
	|     無変換     |      空白     |	
	---------------------------------
            |  英数  |  かな  |
	        ------------------

	・親指シフトキーを長押しすると以下のコードを送出する。
	---------------------------------
	|     タブ      |      変換      |	
	---------------------------------

	-------------------------------------------
	********************************************************
	*** CPU Arduino Leonardo (Arduino AVR Boards)        ***
	********************************************************

	\作業ディレクトリ---+---\oasys30_hobo_nicola -----+---oa30_hobo_nicola.ino
					    |						      |
					    |						      +---oa30_kbd.cpp
					    |						      |
					    |						      +---oa30_kbd.h
					    |
	                    +---\libraries---+---\Adafruit_TinyUSB_Arduino-3.6.1
	                                     |
	                                     +---\hoboNicolaLibrary_KB611
*/

#include "hobo_nicola.h"
#include "hobo_board_config.h"
#include "hobo_sleep.h"
#include "oa30_kbd.h"

oa30_kbd* oa30;

// Function keys with Fn-key pressed.
/*
static const uint16_t fn_keys[] PROGMEM = {
	//HID_S | WITH_R_CTRL,			FN_SETUP_MODE,
	HID_S | WITH_L_CTRL,			FN_SETUP_MODE,
	HID_M,							FN_MEDIA_MUTE,
	HID_COMMA,						FN_MEDIA_VOL_DOWN,
	HID_PERIOD,						FN_MEDIA_VOL_UP,
	HID_U_ARROW,					FN_MEDIA_VOL_UP,
	HID_D_ARROW,					FN_MEDIA_VOL_DOWN,
	HID_R_ARROW,					FN_MEDIA_SCAN_NEXT,
	HID_L_ARROW,					FN_MEDIA_SCAN_PREV,
	HID_ENTER,						FN_MEDIA_PLAY_PAUSE,
	HID_ESCAPE | WITH_R_CTRL,  		FN_SYSTEM_SLEEP,   // Ctrl + App + Esc 
	0, 0
};
*/
static const uint16_t fn_keys[] PROGMEM = {
	HID_S | WITH_L_CTRL,			FN_SETUP_MODE,
	HID_MINUS,						HID_J_CFLEX,
	HID_MINUS | WITH_L_SHIFT,		HID_J_CFLEX | WITH_L_SHIFT,
	HID_J_AT,						HID_J_BSLASH,
	HID_J_AT | WITH_L_SHIFT,		HID_J_BSLASH | WITH_L_SHIFT,
    HID_J_COLON,					HID_J_LBRACK,
	HID_J_COLON | WITH_L_SHIFT,		HID_J_LBRACK | WITH_L_SHIFT,
	HID_SEMICOLON,					HID_J_RBRACK,
	HID_SEMICOLON | WITH_L_SHIFT,	HID_J_RBRACK | WITH_L_SHIFT,
	HID_SLASH,						HID_J_UL,
	HID_SLASH | WITH_L_SHIFT,		HID_J_UL | WITH_L_SHIFT,
	0, 0
};

class oa30_hobo_nicola : public HoboNicola {
public:
	oa30_hobo_nicola() {}
	const uint16_t* fn_keys_table() { return fn_keys; }
	// if you want to change soft-Fn key, un-comment below. 
	//  const uint8_t fn_key() { return HID_APP; }
};

oa30_hobo_nicola hobo_nicola;

class oa30_event : public keyboard_notify {
	public:
		void raw_key_event(uint8_t key, bool released = false) {
			if (is_usb_suspended()) {
				usb_wakeup();
				//oa30->kbd_reset();
				return;
			}
		};
		void key_pressed  (uint8_t key, uint8_t mod) { hobo_nicola.key_event(key, true);  };
		void key_released  (uint8_t key, uint8_t mod) { hobo_nicola.key_event(key, false); };
};

void setup() {
    // Start the USB serial monitor (wait for connection if needed)
#if 0
    Serial.begin(9600);
    while (!Serial) {
        // wait for native USB port to connect
    }
    Serial.println("oasys30_hobo_nicola start\n");
#endif
	//hobo_device_setup(false);
	HoboNicola::init_hobo_nicola(&hobo_nicola);
	oa30 = oa30_kbd::getInstance();
	if (!oa30->begin(new oa30_event, LED_BUILTIN_RX))
		hobo_nicola.error_blink(400);

    // Start the hardware serial port on pins 0 and 1
    Serial1.begin(9600, SERIAL_8E1);
    delay(500);
    //hobo_nicola.has_dedicated_oyakeys(true);        // 専用の親指キーがある(KB611)
    //hobo_nicola.set_oyayubi_keys(HID_F23, HID_F24); // キーコードを変換済、左親指⇒F23、右親指⇒F24
    //Serial.println("setup() is done\n"); //**debug**
}

void loop() {
	if (is_usb_suspended() ) {
		oa30->task(0);
		all_led_off();
		enter_sleep(500);
	} else {
		oa30->task(hobo_nicola.get_hid_led_state());
		hobo_nicola.idle();
		enter_sleep();
	}
	oa30->kbd_loop();
}
