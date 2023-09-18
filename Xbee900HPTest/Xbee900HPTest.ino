#include <XBee.h>

uint8_t payload[] = { 0, 0 };

XBee xbee = XBee();


void setup() {
  xbee.begin(9600);
}
