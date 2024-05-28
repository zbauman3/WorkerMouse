#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <time.h>
#include <util/atomic.h>
#include <util/delay.h>

#include "usbdrv/usbdrv.h"

const PROGMEM char
    usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
        0x05, 0x01, // USAGE_PAGE (Generic Desktop)
        0x09, 0x02, // USAGE (Mouse)
        0xa1, 0x01, // COLLECTION (Application)
        0x09, 0x01, //   USAGE (Pointer)
        0xa1, 0x00, //   COLLECTION (Physical)
        // Not using the buttons for anything. But if needed, they can be added
        // back, then update USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH and make sure
        // reportBuffer is 3 bytes long, with the buttons' presses first
        // 0x05, 0x09, //     USAGE_PAGE (Button)
        // 0x19, 0x01, //     USAGE_MINIMUM (Button 1)
        // 0x29, 0x03, //     USAGE_MAXIMUM (Button 3)
        // 0x15, 0x00, //     LOGICAL_MINIMUM (0)
        // 0x25, 0x01, //     LOGICAL_MAXIMUM (1)
        // 0x95, 0x03, //     REPORT_COUNT (3)
        // 0x75, 0x01, //     REPORT_SIZE (1)
        // 0x81, 0x02, //     INPUT (Data, Variable, Absolute)
        // 0x95, 0x01, //     REPORT_COUNT (1)
        // 0x75, 0x05, //     REPORT_SIZE (5)
        // 0x81, 0x03, //     INPUT (Constant)
        0x05, 0x01, //     USAGE_PAGE (Generic Desktop)
        0x09, 0x30, //     USAGE (X)
        0x09, 0x31, //     USAGE (Y)
        0x15, 0x81, //     LOGICAL_MINIMUM (-127)
        0x25, 0x7f, //     LOGICAL_MAXIMUM (127)
        0x75, 0x08, //     REPORT_SIZE (8)
        0x95, 0x02, //     REPORT_COUNT (2)
        0x81, 0x06, //     INPUT (Data, Variable, Relative)
        0xc0,       //   END_COLLECTION
        0xc0        // END_COLLECTION
};

static uchar reportBuffer[2];
volatile unsigned long timer0_millis = 0;
static unsigned long lastMillis = 0;
static unsigned long millisDelay = 1000;
static int16_t toX = 0;
static int16_t toY = 0;
static int16_t currentX = 0;
static int16_t currentY = 0;
static int8_t speedX = 10;
static int8_t speedY = 10;

// increment the millis counter
ISR(TIMER0_COMPA_vect) { timer0_millis++; }

// no op. Just using usbInterruptIsReady and usbSetInterrupt in the main loop
uchar usbFunctionSetup(uchar data[8]) { return 0; }

unsigned long millis(void) {
  unsigned long millis_return;
  ATOMIC_BLOCK(ATOMIC_FORCEON) { millis_return = timer0_millis; }
  return millis_return;
}

int8_t randBetweenInt8(int8_t min, int8_t max) {
  return rand() % (max + 1 - min) + min;
}

uint8_t hasData() {
  if (currentX != toX || currentY != toY) {
    return 1;
  }

  unsigned long nowMillis = millis();

  if (lastMillis == 0) {
    lastMillis = nowMillis;
    // random between 100 and 7000
    millisDelay = rand() % (7000 + 1 - 100) + 100;
    return 0;
  }

  if (nowMillis - lastMillis < millisDelay) {
    return 0;
  }

  lastMillis = 0;

  toX = randBetweenInt8(-125, 125);
  toY = randBetweenInt8(-125, 125);
  speedX = randBetweenInt8(1, 30);
  speedY = randBetweenInt8(1, 30);

  return 1;
}

int8_t getMoveAmnt(int16_t cur, int16_t to, int8_t speed) {
  if (to < cur) {
    if (cur - to < speed) {
      return -(cur - to);
    }
    return -speed;
  } else {
    if (to - cur < speed) {
      return (to - cur);
    }
    return speed;
  }
}

void buildReport() {
  reportBuffer[0] = 0;
  reportBuffer[1] = 0;

  if (currentX != toX) {
    int8_t moveX = getMoveAmnt(currentX, toX, speedX);
    reportBuffer[0] = moveX;
    currentX += moveX;
  }

  if (currentY != toY) {
    int8_t moveY = getMoveAmnt(currentY, toY, speedY);
    reportBuffer[1] = moveY;
    currentY += moveY;
  }
}

void loop() {
  if (usbInterruptIsReady() && hasData() == 1) {
    buildReport();
    usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
  }
}

int main(void) {
  srand(time(NULL));

  // setup the timer for the `millis` function
  // 16MHz, every 250 clock cycles, prescaled by 64.
  // 16,000,000/(250 × 64) = CTC at 1000Hz
  OCR0A = 249;                        // 250 clock cycles, counting 0
  TCCR0A = (1 << WGM01);              // CTC
  TCCR0B = (1 << CS01) | (1 << CS00); // clk/64
  TIMSK = (1 << OCIE0A);              // interrupt on OCR0A match

  usbDeviceDisconnect();
  _delay_ms(150);
  usbDeviceConnect();

  usbInit();
  sei();

  for (;;) {
    usbPoll();
    loop();
  }

  return 0;
}
