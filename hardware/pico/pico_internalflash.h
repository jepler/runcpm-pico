#include <SdFat.h> // SDFat - Adafruit Fork
#include <Adafruit_SPIFlash.h>
#include <Adafruit_TinyUSB.h>
#include <PicoDVI.h>
#include "../../console.h"
#include "../../arduino_hooks.h"

#undef USE_DISPLAY
#define USE_DISPLAY (1)

#if USE_DISPLAY
DVItext1 display(DVI_RES_800x240p30, adafruit_feather_dvi_cfg);
#endif
DedicatedSpiCard spisd;
FatFileSystem SD;     // Filesystem object from SdFat
Adafruit_USBD_MSC usb_msc; // USB mass storage object

// =========================================================================================
// Define Board-Data
// GP25 green onboard LED
// =========================================================================================
#define LED 13  // GPIO13
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

#if USE_DISPLAY
uint16_t underCursor = ' ';

void putch_display(uint8_t ch) {
    auto x = display.getCursorX();
    auto y = display.getCursorY();
    display.drawPixel(x, y, underCursor);
    if(ch == 8) {
        if(x > 0) {
            display.setCursor(--x, y);
            display.drawPixel(x, y, ' ');
        }
    } else {
        display.write(ch);
    }
    x = display.getCursorX();
    y = display.getCursorY();
    underCursor = display.getPixel(x, y);
    display.drawPixel(x, y, 0xDB);
}
#endif

bool port_init_early() {
#if USE_DISPLAY
  // vreg_set_voltage(VREG_VOLTAGE_1_30);
  if (!display.begin()) { return false; }
  _putch_hook = putch_display;
#endif
  // USB mass storage / filesystem setup (do BEFORE Serial init)
  if (!flash.begin()) { _puts("!flash_begin()"); return false; }
  // Set disk vendor id, product id and revision
  usb_msc.setID("Adafruit", "Internal Flash", "1.0");
  // Set disk size, block size is 512 regardless of spi flash page size
  _puthex32(flash.pageSize());
  _putcon('x');
  _puthex32(flash.numPages());

  usb_msc.setCapacity(flash.pageSize() * flash.numPages() / 512, 512);
  usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);
  usb_msc.setUnitReady(true); // MSC is ready for read/write
  if (!usb_msc.begin()) {
      _puts("!usb_msc.begin()"); return false;
  }
  return true;
}

bool port_flash_begin() {
  if (!spisd.begin(&spiconfig)) {
      _puts("!spisd.begin()"); return false;
  }
  if (!SD.begin(&spisd)) { // Start filesystem on the flash
      _puts("!SD.begin()"); return false;
  }
  return true;
}

