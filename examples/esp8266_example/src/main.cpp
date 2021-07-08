#include <Arduino.h>
extern "C" {
    #include "microrl.h"
}
#include "esp8266_misc.h"

    extern void microrl_init (microrl_t * pThis, void (*print)(const char*));

// create microrl object and pointer on it
    microrl_t rl;
    microrl_t * prl = &rl;

void setup() {
	// // call init with ptr to microrl instance and print callback
	microrl_init (prl, print);
	// // set callback for execute
	microrl_set_execute_callback (prl, execute);
    Serial.begin(115200);

}

void loop() {
    int temp;

    while(1) {

        if (Serial.available() > 0) {
            temp = Serial.read();
            microrl_insert_char (prl, temp);
        }

    }
}