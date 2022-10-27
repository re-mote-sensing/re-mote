/*
 Name:		RFM95_MESH___Relay.ino
 Created:	7/2/2019 12:00:45 PM
 Author:	tyrre
*/

/*
OVERVIEW:
The LoRa RFM95 Mesh Network works as follows:
1. End-Node requests to send a message. A valid route must be determined. This is essentially done by broadcasting to all Relay-Nodes, and having the Relay-Nodes re-broadcast until the Gateway is found. The route to the Gateway is send back to the End-Node
2. The initial broadcast to find a route causes the Relay-Nodes to break the CAD cycle and have them wait in Receiver mode.
3. Once a route has been found, the End-Node transmits the data to the Gateway.
4. The Gateway receives the data and sends back an Acknowledgement
5. All device's go back to sleep

ISSUE:

TODO:

NOTES:
1. The timeouts chosen are based on the size of the network and air time of the message. They are the max value to ensure messages are not missed. AS the network grows, so must these values.

See Markdown Notes of this Protocol for additional info

All messages sent and received by this RH_RF95 Driver conform to this packet format:

LoRa mode:
8 symbol PREAMBLE
Explicit header with header CRC (handled internally by the radio)
4 octets HEADER: (TO, FROM, ID, FLAGS)
0 to 251 octets DATA
CRC (handled internally by the radio)
*/


// Mesh has much greater memory requirements, and you may need to limit the
// max message length to prevent weird crashes
#define RH_MESH_MAX_MESSAGE_LEN 30

#include <RHRouter.h>
#include <RHMesh.h>
#include <RH_RF95.h>
#include <LowPower.h>

// Pin declarations for RFM95 Module
#define RFM95_SLAVE 10
#define RFM95_RESET 9
#define RFM95_INTERUPT 2

// Timeout values for various blocking functions included in the Mesh Network Protocol
// Edit "RH_MESH_ARP_TIMEOUT = 15000" in RHMesh.h to change the Timeout of Route Discovery
#define ACK_SEND_TIMEOUT 20000						
#define ACK_MESH_NODE_MESSAGE_TIMEOUT 10000			
#define ACK_MESH_RELAY_MESSAGE_TIMEOUT 15000

// ID's for the Network
#define NODE_ID 1
#define RELAY_ID 2
#define GATEWAY_ID 3


RH_RF95 rf95(RFM95_SLAVE, RFM95_INTERUPT);							// Singleton instance of the radio driver
RHMesh RHMeshManager(rf95, GATEWAY_ID);								// Class to manage message delivery and receipt, using the driver declared above

// Checks how much memory has not been allocated yet and so is available to be used
int freeMem() {
	extern int __heap_start, * __brkval;
	int v;
	return (int)& v - (__brkval == 0 ? (int)& __heap_start : (int)__brkval);
}

void setup() {
	randomSeed(analogRead(0));
	Serial.begin(9600);
	while (!Serial); // Wait for serial port to be available

	// Initialize the RFM95 Driver and Manager
	if (!RHMeshManager.init()) {
		Serial.println(F("init failed"));
	}
	else {
		Serial.println(F("done"));
	}

	// Set the various Parameters of the LoRa Module
	rf95.setTxPower(23, false);
	rf95.setFrequency(915.0);
	RHMeshManager.setTimeout(ACK_SEND_TIMEOUT);
	rf95.setPreambleLength(12);
	rf95.setModemConfig(RH_RF95::Bw125Cr48Sf4096);

	Serial.println(F("RF95 ready"));

	Serial.print(F("mem = "));
	Serial.println(freeMem());

	delay(5000);
}

uint8_t data[] = "Hello Back2";				// Data to be tranmsitted
uint8_t buf[RH_MESH_MAX_MESSAGE_LEN];		// Data received is read into this varaible

void loop() {

	uint8_t len = sizeof(buf);
	uint8_t from;

	Serial.println(F("MAIN LOOP"));

	// Wait until a message arrives.
	do {
		rf95.sleep();
		Serial.println(F("Loop"));
		delay(500);
	} while (!rf95.isChannelActive());

	Serial.println(F("Out of Loop"));

	// Set the module into Receiver Mode
	rf95.setModeRx();

	/*
	Starts the receiver if it is not running already, processes and possibly routes any received messages addressed to other nodes and delivers any messages addressed to this node.
	If there is a valid application layer message available for this node (or RH_BROADCAST_ADDRESS), send an acknowledgement to the last hop address (blocking until this is complete),
	then copy the application message payload data to buf and return true else return false. If a message is copied, *len is set to the length..
	If from is not NULL, the originator SOURCE address is placed in *source. If to is not NULL, the DEST address is placed in *dest.
	This might be this nodes address or RH_BROADCAST_ADDRESS. This is the preferred function for getting messages addressed to this node.
	If the message is not a broadcast, acknowledge to the sender before returning.
	*/
	if (RHMeshManager.recvfromAckTimeout(buf, &len, ACK_MESH_RELAY_MESSAGE_TIMEOUT, &from))
	{
		Serial.print(F("got request from : 0x"));
		Serial.print(from, HEX);
		Serial.print(F(": "));
		Serial.println((char*)buf);

		// Send a reply back to the originator client
	/*
	Sends a message to the destination node. Initialises the RHRouter message header (the SOURCE address is set to the address of this node, HOPS to 0) and calls route() which looks
	up in the routing table the next hop to deliver to. If no route is known, initiates route discovery and waits for a reply. Then sends the message to the next hop.
	Then waits for an acknowledgement from the next hop (but not from the destination node (if that is different).
	*/
		if (RHMeshManager.sendtoWait(data, sizeof(data), from) != RH_ROUTER_ERROR_NONE)
			Serial.println(F("sendtoWait failed"));
		else
			Serial.println(F("GOOD"));
	}
	// To keep all devices in sync, if this node exits sendtoWait() earlier, delay untill the predetermined timeout time is exceeded
	uint8_t runtime = millis();
	if ((ACK_MESH_RELAY_MESSAGE_TIMEOUT - (millis() - runtime)) > 10)
		delay(ACK_MESH_RELAY_MESSAGE_TIMEOUT - (millis() - runtime));

	Serial.println(F("END LOOP"));
}