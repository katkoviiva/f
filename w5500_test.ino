#include <SPI.h>


#define SPI_DEFAULT_FREQ 100000  // 100kHz (W5500 supports up to 80 MHz)
#define W5500_CS_PIN     10        // Change to your Chip Select (CS/SS) pin

// --- Common Register Addresses (16-bit) ---
#define W5500_MR         0x0000    // Mode Register (1 byte)
#define W5500_GAR        0x0001    // Gateway IP Address (4 bytes)
#define W5500_SUBR       0x0005    // Subnet Mask (4 bytes)
#define W5500_SHAR       0x0009    // Source MAC Address (6 bytes)
#define W5500_SIPR       0x000F    // Source IP Address (4 bytes)
#define W5500_PHYCFGR    0x002E    // PHY Configuration / Link Status (1 byte)
#define W5500_VERSIONR   0x0039    // Chip Version Register (always returns 0x04)

// --- Control Phase Bytes (Common Register Block) ---
#define W5500_CP_RD_1B   0x01      // 0b00000001: Read 1 byte
#define W5500_CP_RD_2B   0x02      // 0b00000010: Read 2 bytes
#define W5500_CP_RD_4B   0x03      // 0b00000011: Read 4 bytes (your original W5500_CP!)
#define W5500_CP_RD_VAR  0x00      // 0b00000000: Read N bytes (variable length)

#define W5500_CP_WR_1B   0x05      // 0b00000101: Write 1 byte
#define W5500_CP_WR_2B   0x06      // 0b00000110: Write 2 bytes
#define W5500_CP_WR_4B   0x07      // 0b00000111: Write 4 bytes
#define W5500_CP_WR_VAR  0x04      // 0b00000100: Write N bytes (variable length)

uint8_t value;
uint8_t cmd[3] = {
  highByte(W5500_VERSIONR), // 0x00
  lowByte(W5500_VERSIONR),  // 0x39
  W5500_CP_RD_1B            // 0x01
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SPI.begin();
  SPI.beginTransaction(SPISettings(SPI_DEFAULT_FREQ, MSBFIRST, SPI_MODE0));
  pinMode(W5500_CS_PIN, OUTPUT);
  digitalWrite(W5500_CS_PIN, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(W5500_CS_PIN, LOW);
  SPI.transfer(cmd, 3);
  value = SPI.transfer(0x00);
  Serial.println(value, HEX);
  digitalWrite(W5500_CS_PIN, HIGH);
  delay(1000);
}
