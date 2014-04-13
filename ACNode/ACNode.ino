#include <EtherCard.h>
#include <SPI.h>
#include <MFRC522.h>


#define REQUEST_RATE 5000 // milliseconds

// ethernet interface mac address
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
// remote website name
const char website[] PROGMEM = "rock";

byte Ethernet::buffer[700];
static long timer;
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);	// Create MFRC522 instance.

// called when the client request is complete
static void my_result_cb (byte status, word off, word len) {
	Serial.print("<<< reply ");
	Serial.print(millis() - timer);
	Serial.println(" ms");
	Serial.println((const char*) Ethernet::buffer + off);
}

void setup () {
	Serial.begin(9600);
	SPI.begin();			// Init SPI bus
	Serial.println("Setting up scanner...");
	mfrc522.PCD_Init();	// Init MFRC522 card
	Serial.println("Scan PICC to see UID and type...");

	Serial.println("\n[getDHCPandDNS]");

	if (ether.begin(sizeof Ethernet::buffer, mymac) == 0)
		Serial.println( "Failed to access Ethernet controller");

	if (!ether.dhcpSetup())
		Serial.println("DHCP failed");

	ether.printIp("My IP: ", ether.myip);
	// ether.printIp("Netmask: ", ether.mymask);
	ether.printIp("GW IP: ", ether.gwip);
	ether.printIp("DNS IP: ", ether.dnsip);

	if (!ether.dnsLookup(website))
		Serial.println("DNS failed");
	ether.printIp("Server: ", ether.hisip);


	Serial.println("\n[Ready]");
	timer = - REQUEST_RATE; // start timing out right away
}

boolean req_sent = false;

void loop () {


	// Look for new cards
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		goto ether;
	}

	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
		goto ether;
	}
	if (!req_sent) {
		Serial.println("Found a card");


		Serial.print("Card UID:");
		for (byte i = 0; i < mfrc522.uid.size; i++) {
			Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
			Serial.print(mfrc522.uid.uidByte[i], HEX);
		}
		ether.browseUrl(PSTR("/auth/"), "test", website, my_result_cb);
		req_sent = !req_sent;
	}

	// Dump debug info about the card. PICC_HaltA() is automatically called.
	//mfrc522.PICC_DumpToSerial(&(mfrc522.uid));

ether:
	ether.packetLoop(ether.packetReceive());
	//if (millis() > timer + REQUEST_RATE) {
	//  timer = millis();
	//  Serial.println("\n>>> REQ");

	//}
}
