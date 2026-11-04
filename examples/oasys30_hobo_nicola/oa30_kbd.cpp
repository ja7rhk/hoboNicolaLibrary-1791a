/**
 * 
  ps2_kbd.cpp  PS/2 Keyboard interface of "Hobo-nicola PS/2 adapter" using Hobo-nicola keyboard and adapter library.
  Copyright (c) 2018,2023 Takeshi Higasa
  
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

//#include "ps2_kbd.h"
#include "oa30_kbd.h"
#include "hid_keycode.h"

// PRO MICRoのLEDは、LOW で点灯することに注意。

static const uint8_t KBD_BUFFER_SIZE = 12;
static uint8_t kbd_buffer[KBD_BUFFER_SIZE];  //  fifo key buffer 

static volatile uint8_t kbd_in, kbd_out;
static uint8_t led_port;

static void clear_buffer() {
	kbd_in = kbd_out = 0;
}

static bool put_buffer(uint8_t s) {
	uint8_t rx = kbd_in + 1;
	if (rx >= KBD_BUFFER_SIZE) 
		rx = 0;
	if (rx == kbd_out) 
		return false; // buffer is full.  
	kbd_buffer[kbd_in] = s;
	kbd_in = rx;    
	return true;  
}

static uint8_t get_buffer() {
	if (kbd_in == kbd_out)
		return 0;  // empty.
	uint8_t s = kbd_buffer[kbd_out];
	if (++kbd_out >= KBD_BUFFER_SIZE)
		kbd_out = 0;
	return s;    
}

static const uint8_t LCTL = HID_L_CTRL;
static const uint8_t LSFT = HID_L_SHIFT;
static const uint8_t LALT = HID_L_ALT;
static const uint8_t LGUI = HID_L_GUI;
static const uint8_t LOYA = HID_MUHENKAN;	//左親指キー
static const uint8_t ROYA = HID_SPACE;    //右親指キー
//static const uint8_t LOYA = HID_F23;	//左親指キー
//static const uint8_t ROYA = HID_F24;    //右親指キー
static const uint8_t MENU = HID_APP;		//MENU as Fnキー
static const uint8_t PTSC = HID_PRNTSCRN;	//PrtSc

#define ____	0x00
static const uint8_t table[] = {
//   	0    1     2     3     4     5     6     7        8      9     a     b     c     d    e    f
//-----------------------------------------------------------------------------------------------------
//(0)	--   --    1     2     3     4     5     6        7      8     9     0     -     @    --   BS
//(1)	TAB  Q     W     E     R     T     Y     U        I      O     P     :     --    ENT  A    S
//(2)	D    F     G     H     J     K     L     ;        :      BS    Z     X     C     V    B    N
//(3)	M    ,     .     /     --    --    SPC   --       --     --    --    --    --    --   --   --
//(4)	--   --    --    --    --    --    --    --       Ins    --    --    --    --    ⇡   End   ←
//(5)	↓   →    LCTL  LSFT  --    --    --   英数      かな   --    --   LALT   --    F2   F3   F4
//(6)	F5   F6    --    F7    F8    F9    --   LOYA      ROYA  LALT  MENU  PgUp   Home  PgDn --   IME_OFF
//(7)	--   DEL   ESC   PENT  --    --    --    --       --     --    --    --    ESC  PrtSc --   --
//
//  0        1       2       3       4       5       6      7      8             9        a      b          c      d       e       f
//--------------------------------------------------------------------------------------------------------------------------------------
    0,       0,      HID_1,  HID_2,  HID_3,  HID_4,  HID_5, HID_6, HID_7,        HID_8,   HID_9, HID_0, HID_MINUS, HID_J_AT, ____,  0x2a,
    0x2b,    HID_Q,  HID_W,  HID_E,  HID_R,  HID_T,  HID_Y, HID_U, HID_I,        HID_O,   HID_P, HID_J_COLON, ____,  0x28, HID_A, HID_S,
    HID_D,   HID_F,  HID_G,  HID_H,  HID_J,  HID_K,  HID_L, HID_SEMICOLON, HID_J_COLON, 0x2a, HID_Z, HID_X, HID_C, HID_V,  HID_B, HID_N,
    HID_M,   0x36,   0x37,   0x38,   ____,   0x2C,   ____,  ____,  ____,         ____,    ____,  ____,      ____,  ____,   ____,  ____,
    ____,    ____,   ____,   ____,   ____,   ____,   ____,  ____,  0x49,         ____,    ____,  ____,      ____,  0x52,   0x4d,  0x50,
    0x51,    0x4f,   LCTL,   LSFT,   ____,   ____,   ____, HID_LANG2, HID_HIRA,  ____,    ____,  LALT,      ____,  HID_F2, HID_F3,HID_F4,
    HID_F5,  HID_F6, ____,   HID_F7, HID_F8, HID_F9, ____,  LOYA,  ROYA,         HID_F10, MENU,  0x4b,      0x4a,  0x4e,   ____,  HID_LANG2,
    ____,    0x4c,   0x29,   0x58,   ____,   ____,   ____,  ____,  ____,         ____,    ____,  ____,      0x29,  PTSC,   ____,  ____
};

#define has_F0  0x02

#define SET_F0(a)	(a |= has_F0)

#define HAS_F0(a) (bool)(flag & has_F0)

//***************************************************************
// ここからは、oasys30のキーボードを使うためのコード。
//***************************************************************
static oa30_kbd* pOA30This = 0;

oa30_kbd* oa30_kbd::getInstance() {
	if (!pOA30This)
		pOA30This = new oa30_kbd;
	return pOA30This;    
}

oa30_kbd::oa30_kbd() {
    notifier = 0;
    attr = 0;
    code = 0;
    attrReceived = false;
    clearRdata();
}

bool oa30_kbd::begin(keyboard_notify* p, uint8_t led_pin) {
	if (p)
		notifier = p;
	led_port = led_pin;    
	if (led_port != 0xff) {
		pinMode(led_port, OUTPUT);
		digitalWrite(led_port, 1);	// LED off.
	}
	::clear_buffer();
	return true;
}

// 同時に処理できるキーコードの数
int8_t oa30_kbd::getZeroIndex() {
	for (int i = 0; i < MAX_KEYS; i++) {
		if (rdata[i] == 0) {
			return i;
		}
	}
	return -1; // 0が見つからなかった場合
}

// 引数と同じ値が書き込まれているrdata[]のインデックスを返す
int8_t oa30_kbd::getMatchingIndex(uint8_t value) {
	for (int i = 0; i < MAX_KEYS; i++) {
		if (rdata[i] == value) {
			return i;
		}
	}
	return -1; // 見つからなかった場合
}

// 0以外が書き込まれているrdata[]のインデックスを返す
int8_t oa30_kbd::getNonZeroIndex() {
	for (int i = 0; i < MAX_KEYS; i++) {
		if (rdata[i] != 0) {
			return i;
		}
	}
	return -1; // 0が見つからなかった場合
}

// rdata[]を全部クリアする
void oa30_kbd::clearRdata() {
	for (int i = 0; i < MAX_KEYS; i++) {
		rdata[i] = 0;
	}
}

void oa30_kbd::kbd_loop() {
	// attrとcodeのペアで送られてくるので、まずattrか来たかどうかを判定する
	// 取り敢えず来たデータをを読み込む
//	if (!attrReceived && Serial1.available() > 0) {
    if (Serial1.available() > 0) {
		attr = code;    // 1バイトシフトする
		code = Serial1.read();
		if (attr == 0x80  || attr == 0x90 || attr == 0xF2|| attr == 0xF1) {
			if (code >= 0x00 && code < 0x80) {
				attrReceived = true;
			} else {
				attrReceived = false;
			}
		}
        //Serial.print(code, HEX); Serial.print(' '); //**debug**
	}
	// attrとcodeが有効ならキーコードのPress/Releaseの解析をする
	if (attrReceived) {
		attrReceived = false;
		switch (attr) {
			case 0x80: // pressed
			{
				// すでにrdata[]に書き込まれているコードかどうかを確認する
				if (getMatchingIndex(code) == -1) {
					// 書き込まれていなければ、0が書き込まれているインデックスを取得する
					int8_t zeroIndex = getZeroIndex();
					if (zeroIndex != -1) {
						rdata[zeroIndex] = code;
						put_buffer(code);	// ** Pressed code **
                        //Serial.print(code, HEX); Serial.print(' '); //**debug**
					}
				}
				break;
			}
			case 0x90: // released
			{
				// コードが0x7Fだった場合、書き込まれている最後のコードをリリースする。
				if (code == 0x7F) {
					int8_t nonZeroIndex = getNonZeroIndex();
					if (nonZeroIndex != -1) {
						code = rdata[nonZeroIndex]; // 最後に書き込まれているコードを取得する
						rdata[nonZeroIndex] = 0;
						put_buffer(0xF0);	// ** Released **
						put_buffer(code);	// ** code **
                        //Serial.print("F0 "); Serial.print(code, HEX); Serial.print(' '); //**debug**
					}
					clearRdata();       // すべてのコードをクリアする
					break;
				}

				// すでにrdata[]に書き込まれているコードかどうかを確認する
				if (getMatchingIndex(code) != -1) {
					// 書き込まれている場合は、0にクリアする
					int8_t matchingIndex = getMatchingIndex(code);
					rdata[matchingIndex] = 0;
					put_buffer(0xF0);	// ** Released **
					put_buffer(code);	// ** code **
				}
				break;
			}
			case 0xF2: // arrow pressed
            case 0xF1: // arrow released
			{
				//Serial.print(code, HEX); Serial.print(" "); 
				switch (code) {
					case 0x00: code = 0x4D; break; // Up arrow code
					case 0x03: code = 0x50; break; // Down arrow code
					case 0x02: code = 0x4F; break; // Left arrow code
					case 0x04: code = 0x51; break; // Right arrow code
					default:   code = 0x00; break; // Invalid arrow code
				}
				if ( code == 0x00) {
					break; // Unknown arrow code, do nothing
				}
				if (attr == 0xF1 ) {	// Arrow key pressed
					put_buffer(0xF0);	// ** Released **
				}
				put_buffer(code);		// ** code **
				break;
			}
			default:
				break;
		}
	}
}

uint8_t oa30_kbd::hid_code(uint8_t k, bool& released) {
    released = false;
    static uint8_t flag = 0;
    uint8_t code = 0;
    if (k == 0xf0) {
        SET_F0(flag);
        return 0;
    }
    if (!code && (k > 0 && k < 0x80) ) {
        //code = table[k - 1];
        code = table[k];
    }
    if (code) {
        released =  (bool)(flag & has_F0) ;
        flag = 0;
    }
    return code;
}

void oa30_kbd::task(uint8_t led) {
    static uint8_t last_hid = 0;
    uint8_t s = ::get_buffer();
    if (s) {
        if (notifier)
            notifier->raw_key_event(s);
        bool released;      
        uint8_t hid = hid_code(s, released);
        if (hid == 0 || notifier == 0)
            return;
		if (released) {
            notifier->key_released(hid, modifier_state);
            last_hid = 0;
            //Serial.print(hid, HEX); Serial.print(": released\n"); //**debug**
        } else {
            if (last_hid != hid) {
                notifier->key_pressed(hid, modifier_state); 
                last_hid = hid;
                //Serial.print(hid, HEX); Serial.print(": pressed\n"); //**debug**
            }
            if (hid == HID_PAUSE) // PAUSEはBREAKがないので。
                notifier->key_released(HID_PAUSE, modifier_state);
        }
    }
}
