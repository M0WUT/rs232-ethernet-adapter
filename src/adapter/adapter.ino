#include <SPI.h>
#include <Ethernet.h>

#include "uart_config.h"

// IP Configuration
const bool USE_DHCP = 1;

// Don't forget to update for each board!!
uint8_t mac_address[] = {0x90, 0xA2, 0xDA, 0x0D, 0xA3, 0x13};
// These will only be used if USE_DHCP is not True
// so we connect with a static IP
IPAddress ip_address(10,59,73,99);
uint8_t subnet_mask[] = {255,255,255,0};


// UART Configuration (imported from uart_config.h)
uartConfig_t config = AGILENT_E4433B;

// Definitions the user is unlikely to need to chang4e
#define MAX_CLIENTS 1
#define SERVER_PORT 23  // VISA uses port 23
#define PHY_CS_PIN 17
#define PHY_RESET_PIN 20

// Spare pins that also get level converted either used
// for RTS/CTS if 4 wire, otherwise, unused but set
// to defined state
#define SPARE_TX_PIN 4
#define SPARE_RX_PIN 5

EthernetServer server(SERVER_PORT);
EthernetClient clients[MAX_CLIENTS];


void setup() {
  delay(1000);
  pinMode(PHY_RESET_PIN, OUTPUT);
  digitalWrite(PHY_RESET_PIN, LOW);
  delay(200);
  digitalWrite(PHY_RESET_PIN, HIGH);
  Ethernet.init(PHY_CS_PIN);
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // UARTs
  Serial1.begin(config.baud_rate, config.stop_bits == 2 ? SERIAL_8N2 : SERIAL_8N1);
  pinMode(SPARE_TX_PIN, OUTPUT);
  // If in 4-wire mode, set low to indicate we're OK to receive data
  // otherwise, set high so it's in an idle state
  digitalWrite(SPARE_TX_PIN, !config.four_wire);
  pinMode(SPARE_RX_PIN, INPUT_PULLUP);
}

void get_ip_address(){
  if (USE_DHCP) {
    Serial.println("Attempting to get IP address over DHCP");
    // This returns 1 on successful connection, 0 if connection failed
    while(Ethernet.begin(mac_address) != 1){
      delay(500);
    }

    
    Serial.print("Got IP throught DHCP of ");
    Serial.println(Ethernet.localIP());
  } else {
    Ethernet.begin(mac_address, ip_address, subnet_mask);
    Serial.print("Initialising with static IP of ");
    Serial.println(ip_address);
  }

  Serial.println("Starting server...");
  server.begin();
  delay(1000);
  Serial.println("Server started.");
}


void loop() {
  static bool link_up = 0;
  if (!link_up) {
    digitalWrite(LED_BUILTIN, LOW);
    if (Ethernet.linkStatus() == LinkON) {
      Serial.println("Ethernet connected");
      get_ip_address();
      link_up = 1;
    }
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
    if (Ethernet.linkStatus() == LinkOFF) {
      link_up = 0;
      Serial.println("Ethernet disconnected");
    } else {
      Ethernet.maintain();  // Keeps DHCP leases active

      EthernetClient client = server.available();
      if (client) {
        String tx_string = "";
        while (client.available()){
          char x = char(client.read());
          if ((x != '\r') && (x != '\n')){
            tx_string += x;
          }
        }
        tx_string += config.termination_str;

        if (config.four_wire) {
          // Wait for equipment to indicate it's ready for data
          while(digitalRead(SPARE_TX_PIN) == 1){
            delay(20);
          }
        }

        Serial1.print(tx_string);
      }
      
      // Return data
      if (Serial1.available()) {
        String rx_string = "";
        while(Serial1.available()){
          rx_string += char(Serial1.read());
          delay(10);
        }
        rx_string += '\n';
        server.print(rx_string);
      }
    }
  }
}


