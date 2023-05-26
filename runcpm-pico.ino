// only AVR and ARM CPU
// #include <MemoryFree.h>

#include "globals.h"

// =========================================================================================
// Guido Lehwalder's Code-Revision-Number
// =========================================================================================
#define GL_REV "GL20230303.0"

#include <SPI.h>

#include <SdFat.h>        // One SD library to rule them all - Greinman SdFat from Library Manager
#include <Adafruit_SPIFlash.h>
#include <Adafruit_TinyUSB.h>


// =========================================================================================
// Board definitions go into the "hardware" folder, if you use a board different than the
// Arduino DUE, choose/change a file from there and reference that file here
// =========================================================================================

// Raspberry Pi Pico - normal (LED = GPIO25)
#include "hardware/pico/feather_dvi.h"

#include "abstraction_arduino.h"
// Raspberry Pi Pico W(iFi)   (LED = GPIO32)
// #include "hardware/pico/pico_w_sd_spi.h"

// =========================================================================================
// Delays for LED blinking
// =========================================================================================
#define sDELAY 100
#define DELAY 1200


// =========================================================================================
// Serial port speed
// =========================================================================================
#define SERIALSPD 115200

// =========================================================================================
// PUN: device configuration
// =========================================================================================
#ifdef USE_PUN
File32 pun_dev;
int pun_open = FALSE;
#endif

// =========================================================================================
// LST: device configuration
// =========================================================================================
#ifdef USE_LST
File32 lst_dev;
int lst_open = FALSE;
#endif

#include "ram.h"
#include "console.h"
#include "cpu.h"
#include "disk.h"
#include "host.h"
#include "cpm.h"
#ifdef CCP_INTERNAL
#include "ccp.h"
#endif

void setup(void) {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW^LEDinv);


// =========================================================================================
// Serial Port Definition
// =========================================================================================
//   Serial =USB / Serial1 =UART0/COM1 / Serial2 =UART1/COM2

   Serial1.setRX(1); // Pin 2
   Serial1.setTX(0); // Pin 1

   Serial2.setRX(5); // Pin 7
   Serial2.setTX(4); // Pin 6

// or

//   Serial1.setRX(17); // Pin 22
//   Serial1.setTX(16); // Pin 21

//   Serial2.setRX(21); // Pin 27
//   Serial2.setTX(20); // Pin 26
// =========================================================================================

  if (!port_init_early()) { return; }

  // _clrscr();
  // _puts("Opening serial-port...\r\n");  
  Serial.begin(SERIALSPD);
  while (!Serial) {	// Wait until serial is connected
    digitalWrite(LED, HIGH^LEDinv);
    delay(sDELAY);
    digitalWrite(LED, LOW^LEDinv);
    delay(DELAY);
  }


#ifdef DEBUGLOG
  _sys_deletefile((uint8 *)LogName);
#endif

  
// =========================================================================================  
// Printing the Startup-Messages
// =========================================================================================

  _clrscr();

  // if (bootup_press == 1)
  //   { _puts("Recognized \e[1m#\e[0m key as pressed! :)\r\n\r\n");
  //   }
  
  _puts("CP/M Emulator \e[1mv" VERSION "\e[0m   by   \e[1mMarcelo  Dantas\e[0m\r\n");
  _puts("----------------------------------------------\r\n");  
  _puts("     running    on   Raspberry Pi [\e[1m Pico \e[0m]\r\n");
  _puts("----------------------------------------------\r\n");
  _puts("\e[0m]\r\n");

	_puts("BIOS              at [\e[1m0x");
	_puthex16(BIOSjmppage);
//	_puts(" - ");
	_puts("\e[0m]\r\n");

	_puts("BDOS              at [\e[1m0x");
	_puthex16(BDOSjmppage);
	_puts("\e[0m]\r\n");

	_puts("CCP " CCPname " at [\e[1m0x");
	_puthex16(CCPaddr);
	_puts("\e[0m]\r\n");

  #if BANKS > 1
	_puts("Banked Memory        [\e[1m");
	_puthex8(BANKS);
    _puts("\e[0m]banks\r\n");
  #else
	_puts("Banked Memory        [\e[1m");
	_puthex8(BANKS);
	_puts("\e[0m]bank\r\n");
  #endif

   // Serial.printf("Free Memory          [\e[1m%d bytes\e[0m]\r\n", freeMemory());

  _puts("CPU-Clock            [\e[1m250Mhz\e[0m]\r\n");


  _puts("Init Storage         [ \e[1m");
  if (port_flash_begin()) {
    _puts("OK \e[0m]\r\n");
    _puts("----------------------------------------------");

                        
    if (VersionCCP >= 0x10 || SD.exists(CCPname)) {
      while (true) {
        _puts(CCPHEAD);
        _PatchCPM();
	Status = 0;
#ifndef CCP_INTERNAL
        if (!_RamLoad((char *)CCPname, CCPaddr)) {
          _puts("Unable to load the CCP.\r\nCPU halted.\r\n");
          break;
        }
        Z80reset();
        SET_LOW_REGISTER(BC, _RamRead(DSKByte));
        PC = CCPaddr;
        Z80run();
#else
        _ccp();
#endif
        if (Status == 1)
          break;
#ifdef USE_PUN
        if (pun_dev)
          _sys_fflush(pun_dev);
#endif
#ifdef USE_LST
        if (lst_dev)
          _sys_fflush(lst_dev);
#endif
      }
    } else {
      _puts("Unable to load CP/M CCP.\r\nCPU halted.\r\n");
    }
  } else {
    _puts("ERR \e[0m]\r\nUnable to initialize SD card.\r\nCPU halted.\r\n");
  }
}

// if loop is reached, blink LED forever to signal error
void loop(void) {
  digitalWrite(LED, HIGH^LEDinv);
  delay(DELAY);
  digitalWrite(LED, LOW^LEDinv);
  delay(DELAY);
  digitalWrite(LED, HIGH^LEDinv);
  delay(DELAY);
  digitalWrite(LED, LOW^LEDinv);
  delay(DELAY * 4);
}
