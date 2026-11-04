// Arduino Leonardo
// Wiring Connections
// - Connect external module TX to Leonardo Pin 0 (RX)
// - Connect external module RX to Leonardo Pin 1 (TX)
// - Connect shared Ground (GND)

void setup() {
    // Start the USB serial monitor (wait for connection if needed)
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for native USB port to connect
    }
    Serial.println("oasys30_kb_test2 start\n");

    // Start the hardware serial port on pins 0 and 1
    Serial1.begin(9600, SERIAL_8E1);
}

// 同時に処理できるキーコードの数
#define MAX_KEYS 8
uint8_t rdata[MAX_KEYS] = {0};

// 0が書き込まれているrdata[]のインデックスを返す
int8_t getZeroIndex() {
    for (int i = 0; i < MAX_KEYS; i++) {
        if (rdata[i] == 0) {
            return i;
        }
    }
    return -1; // 0が見つからなかった場合
}

// 引数と同じ値が書き込まれているrdata[]のインデックスを返す
int8_t getMatchingIndex(uint8_t value) {
    for (int i = 0; i < MAX_KEYS; i++) {
        if (rdata[i] == value) {
            return i;
        }
    }
    return -1; // 見つからなかった場合
}

// 0以外が書き込まれているrdata[]のインデックスを返す
int8_t getNonZeroIndex() {
    for (int i = 0; i < MAX_KEYS; i++) {
        if (rdata[i] != 0) {
            return i;
        }
    }
    return -1; // 0が見つからなかった場合
}

// rdata[]を全部クリアする
void clearRdata() {
    for (int i = 0; i < MAX_KEYS; i++) {
        rdata[i] = 0;
    }
}

uint8_t attr = 0;   // 属性
uint8_t code = 0;   // キーコード
bool attrReceived = false;

void loop() {
    // attrとcodeのペアで送られてくるので、まずattrか来たかどうかを判定する
    // 取り敢えず来たデータをを読み込む
    if (!attrReceived && Serial1.available() > 0) {
        attr = code;    // 1バイトシフトする
        code = Serial1.read();
        //Serial.print(attr, HEX); Serial.print(" "); 
        //Serial.print(code, HEX); Serial.print("\n"); 
        if (attr == 0x80  || attr == 0x90 || attr == 0xF2 || attr == 0xF1) {
            if (code >= 0x00 && code < 0x80) {
                attrReceived = true;
            } else {
                attrReceived = false;
            }
        }
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
                    } else {
                        // 0が見つからなかった場合は、rdata[]が満杯であることを示す
                        Serial.println(": rdata[] is full");
                        break;
                    }
                } else {
                    // このコードは既に書き込まれている(リピート)
                    break;
                }
                Serial.print(code, HEX); Serial.print(" "); 
                Serial.println(": Pressed");
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
                        Serial.print(code, HEX); Serial.print(" "); 
                        Serial.println(": Released all pressed code");
                    } else {
                        Serial.print(code, HEX); Serial.print(" "); 
                        Serial.println(": No code to release");
                    }
                    clearRdata();       // すべてのコードをクリアする
                    break;
                }

                // すでにrdata[]に書き込まれているコードかどうかを確認する
                int8_t matchingIndex = getMatchingIndex(code);
                if (matchingIndex != -1) {
                    // 書き込まれている場合は、0にクリアする
                    rdata[matchingIndex] = 0;
                } else {
                    // 見つからなかった場合は、不正なシーケンスであることを示す
                    Serial.println(": Invalid sequence");
                    break;
                }
                Serial.print(code, HEX); Serial.print(" "); 
                Serial.println(": Released");
                break;
            }
            case 0xF2: // arrow pressed
            case 0xF1: // arrow released
            {
                //Serial.print(code, HEX); Serial.print(" "); 
                switch (code) {
                    case 0x00:
                        Serial.println(": Arrow Up");
                        code = 0x4D; // Up arrow code
                        break;
                    case 0x03:
                        Serial.println(": Arrow Down");
                        code = 0x50; // Down arrow code
                        break;
                    case 0x02:
                        Serial.println(": Arrow Left");
                        code = 0x4F; // Left arrow code
                        break;
                    case 0x04:
                        Serial.println(": Arrow Right");
                        code = 0x51; // Right arrow code
                        break;
                    default:
                        Serial.println(": Unknown arrow code");
                        code = 0x00; // Right arrow code
                }
				if (attr == 0xF2 ) {
					// Arrow key pressed
	                if (getMatchingIndex(code) == -1) {
	                    // 書き込まれていなければ、0が書き込まれているインデックスを取得する
	                    int8_t zeroIndex = getZeroIndex();
	                    if (zeroIndex != -1) {
	                        rdata[zeroIndex] = code;
                        Serial.print(code, HEX); Serial.print(" "); 
                        Serial.println(": Arrow key Pressed");
	                    } else {
	                        // 0が見つからなかった場合は、rdata[]が満杯であることを示す
	                        Serial.println(": rdata[] is full");
	                    }
	                }
				} else { // (attr == 0xF1)
					// Arrow key released
	                // すでにrdata[]に書き込まれているコードかどうかを確認する
                    int8_t matchingIndex = getMatchingIndex(code);
	                if (matchingIndex != -1) {
	                    // 書き込まれている場合は、0にクリアする
	                    rdata[matchingIndex] = 0;
                        Serial.print(code, HEX); Serial.print(" "); 
                        Serial.println(": Arrow key Released");
	                } else {
	                    // 見つからなかった場合は、不正なシーケンスであることを示す
	                    Serial.println(": Invalid sequence");
	                    break;
	                }
				}
                break;
            }
            default:
                Serial.print(attr, HEX); Serial.print(" "); 
                Serial.print(code, HEX); Serial.print(" "); 
                Serial.println(": Discarded");
                break;
        }
    }
}
