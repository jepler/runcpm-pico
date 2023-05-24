// =========================================================================================
// Define SdFat as alias for SD
// =========================================================================================
#include <SdFat.h>        // One SD library to rule them all - Greinman SdFat from Library Manager
#include <Adafruit_SPIFlash.h>
#include <Adafruit_TinyUSB.h>
#include "../../console.h"

Adafruit_FlashTransport_RP2040 flashTransport;
Adafruit_SPIFlash flash(&flashTransport);
FatFileSystem SD;     // Filesystem object from SdFat
Adafruit_USBD_MSC usb_msc; // USB mass storage object

// =========================================================================================
// Define Board-Data
// GP25 green onboard LED
// =========================================================================================
#define LED 25  // GPIO25
#define LEDinv 0
#define board_pico
#define board_analog_io
#define board_digital_io
#define BOARD "Raspberry Pi Pico (internal storage)"

// FUNCTIONS REQUIRED FOR USB MASS STORAGE ---------------------------------

static bool msc_changed = true; // Is set true on filesystem changes

// Callback on READ10 command.
int32_t msc_read_cb(uint32_t lba, void *buffer, uint32_t bufsize) {
  return flash.readBlocks(lba, (uint8_t *)buffer, bufsize / 512) ? bufsize : -1;
}

// Callback on WRITE10 command.
int32_t msc_write_cb(uint32_t lba, uint8_t *buffer, uint32_t bufsize) {
  digitalWrite(LED_BUILTIN, HIGH);
  return flash.writeBlocks(lba, buffer, bufsize / 512) ? bufsize : -1;
}

// Callback on WRITE10 completion.
void msc_flush_cb(void) {
  flash.syncBlocks();   // Sync with flash
  SD.cacheClear(); // Clear filesystem cache to force refresh
  digitalWrite(LED_BUILTIN, LOW);
  msc_changed = true;
}

void _puthex32(uint32_t x) {
    _puthex16(x >> 16);
    _puthex16(x & 0xffff);
}

bool port_init_early() {
  _puts("port_init_early()");
  _puts("X");
  // USB mass storage / filesystem setup (do BEFORE Serial init)
  if (!flash.begin()) { _puts("!flash_begin()"); return false; }
  _puts("Y");
  // Set disk vendor id, product id and revision
  usb_msc.setID("Adafruit", "Internal Flash", "1.0");
  _puts("Z");
  // Set disk size, block size is 512 regardless of spi flash page size
  _puthex32(flash.pageSize());
  _putcon(' ');
  _puthex32(flash.numPages());

  usb_msc.setCapacity(flash.pageSize() * flash.numPages() / 512, 512);
  _puts("A");
  usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);
  _puts("B");
  usb_msc.setUnitReady(true); // MSC is ready for read/write
  _puts("C");
  if (!usb_msc.begin()) {
      _puts("!usb_msc.begin()"); return false;
  } else {
      _puts("usb_msc.begin() OK");
  }
  _puts("D");
  return true;
}

bool port_flash_begin() {
  _puts("port_flash_begin()");
  if (!SD.begin(&flash)) { // Start filesystem on the flash
      _puts("!SD.begin()"); return false;
  }
  _puts("E");
  _puts("port_flash_begin success");
  return true;
}

