#include "libpax_helpers.h"


// libpax payload
struct count_payload_t count_from_libpax;

int8_t getRSSI() {
    // Placeholder implementation to retrieve RSSI value
    // This should be replaced with actual logic to get RSSI from Wi-Fi or BLE
    return -60; // Example value
}

void init_libpax(void) {
  libpax_counter_init(setSendIRQ, &count_from_libpax, cfg.sendcycle * 2,
                      cfg.countermode);
  libpax_counter_start();
}