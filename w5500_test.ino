#include <SPI.h>


#define SPI_DEFAULT_FREQ 32000000  // 32MHz (W5500 supports up to 80 MHz)
#define W5500_CS_PIN     10        // Chip Select (CS/SS) pin

// --- Common Register Addresses (16-bit) ---
#define W5500_MR         0x0000    // Mode Register (1 byte)
#define W5500_GAR        0x0001    // Gateway IP Address (4 bytes)
#define W5500_SUBR       0x0005    // Subnet Mask (4 bytes)
#define W5500_SHAR       0x0009    // Source MAC Address (6 bytes)
#define W5500_SIPR       0x000F    // Source IP Address (4 bytes)
#define W5500_PHYCFGR    0x002E    // PHY Configuration / Link Status (1 byte)
#define W5500_VERSIONR   0x0039    // Chip Version Register (always returns 0x04)

// --- Socket Register Addresses (16-bit) ---
// Socket 0 Register Offsets (16-bit)
#define W5500_S0_MR      0x0000
#define W5500_S0_CR      0x0001
#define W5500_S0_SR      0x0003
#define W5500_S0_PORT    0x0004

#define W5500_S0_DIPR    0x000C
#define W5500_S0_DPORT   0x000E

#define W5500_S0_TX_FSR  0x0020
#define W5500_S0_TX_RD   0x0022
#define W5500_S0_TX_WR   0x0024

#define W5500_S0_RX_RSR  0x0026
#define W5500_S0_RX_RD   0x0028

// Socket 0 Buffer Masks & Control Bytes
#define W5500_S0_RX_MASK       0x07FF
#define W5500_S0_TX_MASK       0x07FF
#define W5500_S0_RX_CP_RD_VAR  0x18

// Socket commands
#define S0_CMD_OPEN      0x01
#define S0_CMD_SEND      0x20
#define S0_CMD_RECV      0x40

#define S0_MODE_UDP      0x02
#define S0_SR_UDP        0x22

// Socket 0 Control Phase Bytes
#define W5500_S0_CP_RD_1B      0x09
#define W5500_S0_CP_RD_2B      0x0A
#define W5500_S0_CP_WR_1B      0x0D
#define W5500_S0_CP_WR_2B      0x0E
#define W5500_S0_CP_WR_4B      0x0F

#define W5500_S0_TX_CP_WR_VAR  0x14

// Socket 2 Control Phase Bytes (Block 0x08 << 3 = 0x40)
#define W5500_S2_CP_RD_1B  0x41  // 0x40 | 0x01
#define W5500_S2_CP_RD_2B  0x42  // 0x40 | 0x02
#define W5500_S2_CP_WR_1B  0x45  // 0x40 | 0x05
#define W5500_S2_CP_WR_2B  0x46  // 0x40 | 0x06

// --- Control Phase Bytes (Common Register Block) ---
#define W5500_CP_RD_1B   0x01      // 0b00000001: Read 1 byte
#define W5500_CP_RD_2B   0x02      // 0b00000010: Read 2 bytes
#define W5500_CP_RD_4B   0x03      // 0b00000011: Read 4 bytes
#define W5500_CP_RD_VAR  0x00      // 0b00000000: Read N bytes (variable length)

#define W5500_CP_WR_1B   0x05      // 0b00000101: Write 1 byte
#define W5500_CP_WR_2B   0x06      // 0b00000110: Write 2 bytes
#define W5500_CP_WR_4B   0x07      // 0b00000111: Write 4 bytes
#define W5500_CP_WR_VAR  0x04      // 0b00000100: Write N bytes (variable length)

byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x9E, 0x9C };

uint8_t cmd[3] = {
 highByte(W5500_VERSIONR), // 0x00
 lowByte(W5500_VERSIONR), // 0x39
 W5500_CP_RD_1B // 0x01
};

uint8_t dhcp;
uint8_t dhcp0hi;
uint8_t dhcp0lo;
uint8_t status;
uint8_t read_ptr_rx_hi;
uint8_t read_ptr_rx_lo;
uint8_t read_rx_hi;
uint8_t read_rx_lo;
uint8_t read_rx_size_hi;
uint8_t read_rx_size_lo;
uint8_t read_ptr_tx_hi;
uint8_t read_ptr_tx_lo;
uint8_t read_tx_hi;
uint8_t read_tx_lo;
uint8_t read_tx_size_hi;
uint8_t read_tx_size_lo;
uint8_t version;

void setup() {
    // put your setup code here, to run once:
    Serial.begin(9600);
    SPI.begin();
    SPI.beginTransaction(SPISettings(SPI_DEFAULT_FREQ, MSBFIRST, SPI_MODE0));
    pinMode(W5500_CS_PIN, OUTPUT);
    digitalWrite(W5500_CS_PIN, HIGH);
    SPI.endTransaction();

    w5500_pwr_reset();
    w5500_init();
    w5500_version_read();
    w5500_link_status();
    w5500_socket_n_read_rx_ptr();
    w5500_socket_n_read_rx_size();
    w5500_socket_n_read_tx_ptr();
    w5500_socket_n_read_tx_size();
}


void loop() {
    // put your main code here, to run repeatedly:

    delay(1000);
}

void w5500_pwr_reset() {
    SPI.beginTransaction(SPISettings(SPI_DEFAULT_FREQ, MSBFIRST, SPI_MODE0));
    digitalWrite(W5500_CS_PIN, LOW);

    SPI.transfer(highByte(W5500_MR)); // 0x00
    SPI.transfer(lowByte(W5500_MR)); // 0x00
    SPI.transfer(W5500_CP_WR_1B); // 0x05
    SPI.transfer(0x80);
    Serial.println("Reset is done.");

    digitalWrite(W5500_CS_PIN, HIGH);
    SPI.endTransaction();
    delay(1000);
}

void w5500_init() {
    SPI.beginTransaction(SPISettings(SPI_DEFAULT_FREQ, MSBFIRST, SPI_MODE0));
    digitalWrite(W5500_CS_PIN, LOW);

    SPI.transfer(highByte(W5500_MR)); // 0x00
    SPI.transfer(lowByte(W5500_MR)); // 0x00
    SPI.transfer(W5500_CP_WR_1B); // 0x05
    SPI.transfer(0x00); 

    digitalWrite(W5500_CS_PIN, HIGH);
    SPI.endTransaction();
    delay(1000);
}

void w5500_link_status(uint8_t link_status) {
    SPI.beginTransaction(SPISettings(SPI_DEFAULT_FREQ, MSBFIRST, SPI_MODE0));
    digitalWrite(W5500_CS_PIN, LOW);

    SPI.transfer(highByte(W5500_PHYCFGR)); // 0x00
    SPI.transfer(lowByte(W5500_PHYCFGR)); // 0x2E
    SPI.transfer(W5500_CP_RD_1B); // 0x01
    status = SPI.transfer(0x00);
    Serial.print("Status 0x");
    Serial.println(status, HEX);

    digitalWrite(W5500_CS_PIN, HIGH);
    SPI.endTransaction();
    delay(1000);
    return link_status
}

void w5500_socket_n_config(uint8_t socket, uint8_t mode, uint16_t port) {
    SPI.beginTransaction(SPISettings(SPI_DEFAULT_FREQ, MSBFIRST, SPI_MODE0));
    digitalWrite(W5500_CS_PIN, LOW);



    digitalWrite(W5500_CS_PIN, HIGH);
    SPI.endTransaction();
    delay(1000);
    return socket, mode, port;
}

void w5500_socket_n_read_rx_ptr(uint8_t socket_ptr_read_rx) {
    SPI.beginTransaction(SPISettings(SPI_DEFAULT_FREQ, MSBFIRST, SPI_MODE0));
    digitalWrite(W5500_CS_PIN, LOW);

    SPI.transfer(highByte(W5500_S0_RX_RD)); // 0x00
    SPI.transfer(lowByte(W5500_S0_RX_RD)); // 0x28
    SPI.transfer(W5500_S0_CP_RD_2B); // 0x0A
    read_ptr_rx_hi = SPI.transfer(0x00);
    read_ptr_rx_lo = SPI.transfer(0x00);
    Serial.print("read_ptr_rx_hi 0x");
    Serial.println(read_ptr_rx_hi, HEX);
    Serial.print("read_ptr_rx_lo 0x");
    Serial.println(read_ptr_rx_lo, HEX);

    digitalWrite(W5500_CS_PIN, HIGH);
    SPI.endTransaction();
    return socket_ptr_read_rx;

}



void w5500_socket_n_read_rx_size(uint8_t read_rx_size) {
    SPI.beginTransaction(SPISettings(SPI_DEFAULT_FREQ, MSBFIRST, SPI_MODE0));
    digitalWrite(W5500_CS_PIN, LOW);

    SPI.transfer(highByte(W5500_S0_RX_RSR)); // 0x00
    SPI.transfer(lowByte(W5500_S0_RX_RSR)); // 0x26
    SPI.transfer(W5500_S0_CP_RD_2B); // 0x0A
    read_rx_size_hi = SPI.transfer(0x00);
    read_rx_size_lo = SPI.transfer(0x00);
    Serial.print("read_rx_size_hi 0x");
    Serial.println(read_rx_size_hi, HEX);
    Serial.print("read_rx_size_lo 0x");
    Serial.println(read_rx_size_lo, HEX);

    digitalWrite(W5500_CS_PIN, HIGH);
    SPI.endTransaction();
    delay(1000);
}

void w5500_socket_n_read_tx_ptr() {
    SPI.beginTransaction(SPISettings(SPI_DEFAULT_FREQ, MSBFIRST, SPI_MODE0));
    digitalWrite(W5500_CS_PIN, LOW);

    SPI.transfer(highByte(W5500_S0_TX_RD)); // 0x00
    SPI.transfer(lowByte(W5500_S0_TX_RD)); // 0x22
    SPI.transfer(W5500_S0_CP_RD_2B); // 0x0A
    read_ptr_tx_hi = SPI.transfer(0x00);
    read_ptr_tx_lo = SPI.transfer(0x00);
    Serial.print("read_ptr_tx_hi 0x");
    Serial.println(read_ptr_tx_hi, HEX);
    Serial.print("read_ptr_tx_lo 0x");
    Serial.println(read_ptr_tx_lo, HEX);

    digitalWrite(W5500_CS_PIN, HIGH);
    SPI.endTransaction();
    delay(1000);
}

void w5500_socket_n_read_tx_size() {
    SPI.beginTransaction(SPISettings(SPI_DEFAULT_FREQ, MSBFIRST, SPI_MODE0));
    digitalWrite(W5500_CS_PIN, LOW);

    SPI.transfer(highByte(W5500_S0_TX_FSR)); // 0x00
    SPI.transfer(lowByte(W5500_S0_TX_FSR)); // 0x20
    SPI.transfer(W5500_S0_CP_RD_2B); // 0x0A
    read_tx_size_hi = SPI.transfer(0x00);
    read_tx_size_lo = SPI.transfer(0x00);
    Serial.print("read_tx_size_hi 0x");
    Serial.println(read_tx_size_hi, HEX);
    Serial.print("read_tx_size_lo 0x");
    Serial.println(read_tx_size_lo, HEX);

    digitalWrite(W5500_CS_PIN, HIGH);
    SPI.endTransaction();
    delay(1000);
}



void w5500_socket_n_write_tx(uint8_t socket_write_tx) {
    SPI.beginTransaction(SPISettings(SPI_DEFAULT_FREQ, MSBFIRST, SPI_MODE0));
    digitalWrite(W5500_CS_PIN, LOW);



    digitalWrite(W5500_CS_PIN, HIGH);
    SPI.endTransaction();
    delay(1000);
}

void w5500_version_read() {
    SPI.beginTransaction(SPISettings(SPI_DEFAULT_FREQ, MSBFIRST, SPI_MODE0));
    digitalWrite(W5500_CS_PIN, LOW);

    SPI.transfer(highByte(W5500_VERSIONR)); // 0x00
    SPI.transfer(lowByte(W5500_VERSIONR)); // 0x39
    SPI.transfer(W5500_CP_RD_1B); // 0x01
    version_read = SPI.transfer(0x00);

    digitalWrite(W5500_CS_PIN, HIGH);
    SPI.endTransaction();

    Serial.print("Version 0x");
    Serial.println(version, HEX);
    delay(1000);
}