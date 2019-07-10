/*
 Name:		RFM95_Flooding___Relayer.ino
 Created:	7/9/2019 10:37:36 AM
 Author:	tyrre
*/


/*
OVERVIEW:
The Flooding Mesh Network works as follows:
The End-Node boradcast its message, and all relayer's within range rebroadcast the message. Eventually, the message will be received by the Gateway.
The Gatetway then sends an ACK back. Precations are taken to avoid broadcast storms and discard stale messages.


ISSUES:
Message may be missed if Node/Relay/Gateway is currently processing another message
This issue would exists with any Mexh Network, so how does the RadioHead Mesh Network Protocol fix it?
I think maybe if I just have certain nodes delay equal to the process + send time, this should minimize any conflict issues

NOTES:
All messages sent and received by this RH_RF95 Driver conform to this packet format:

LoRa mode:
8 symbol PREAMBLE
Explicit header with header CRC (handled internally by the radio)
4 octets HEADER: (TO, FROM, ID, FLAGS)
0 to 251 octets DATA
CRC (handled internally by the radio)

Message Format - [ACK]|[Lifespan]|[Message ID]|[Message Data]
ACK - 0 or 1 ( 0 = not an ACK, 1 = is an ACK)
Lifespan - Exceeding a threshold will result in the message being terminated
Message ID - Uniquly determined using the Node ID, # of nodes in the network, and the # of messages the node has sent
Message Data - Reason for the message tranmssion. Thsi data could be GPS coordiantes, commands, etc.

Any global pointer arrays I have need to have the memory allocated before hand to ensure defined behaviour occurs. With memory already allocated, I can safely store the actual value
(not the address) without it disappearing. (since the location the pointer points to was defined earlier and will not disappear)

Using pointers in the Loop(). Will that mean every run of the loop will delete where they point to (since they may point to the memory of a local varaible)?
What happens is, every iteration of the loop makes a new pointer that points to a local varaible. However, the local varaibles address is always the same. So all pointers
point to the same location. Calling the value the pointers point to will print the current value of the local variable
*/

#include <RH_RF95.h>
#include <LowPower.h>
#include <RHReliableDatagram.h>

// Pin declarations for RFM95 Module
#define RFM95_SLAVE 10
#define RFM95_RESET 9
#define RFM95_INTERUPT 2

// ID's for the Network
#define NODE_ID 1
#define RELAY_ID 2
#define GATEWAY_ID 3

// Time to wait for message after preamble detected
#define WAIT_AVAILABLE_TIMEOUT 3000

// Various length of values used in the payload
#define RH_PAYLOAD_LEN 30					// all the stuff below, plus "|", plus null character
#define RH_MESSAGE_ACK_LEN 2
#define RH_MESSAGE_LIFESPAN_LEN 2
#define RH_MESSAGE_ID_LEN 5
#define RH_MESSAGE_LEN 20
#define RH_MESSAGE_LIFESPAN "7"


/*
An array of pointers to strings is an array of character pointers where each pointer points to the first character of the string or the base address of the string
So messageIdArray can contain up to MESSAGE_ID_ARRAY_SIZE different pointers
*/
// Used to store previously seen message ID's & their asscociated ACK
#define MESSAGE_ID_ARRAY_SIZE 10
char messageIdArray[MESSAGE_ID_ARRAY_SIZE][RH_MESSAGE_ID_LEN];
char messageAckArray[MESSAGE_ID_ARRAY_SIZE][RH_MESSAGE_ACK_LEN];
uint8_t messageIdArrayCounter;										// keeps track of messages sent to determine the newest messageId

char payload[RH_PAYLOAD_LEN];										// This is the payload that is tranmistted
char buf[RH_PAYLOAD_LEN];											// Any received data is read into this varaible

RH_RF95 rf95(RFM95_SLAVE, RFM95_INTERUPT);							// Singleton instance of the radio driver
RHReliableDatagram RHReliableDatagramManager(rf95, NODE_ID);		// Class to manage message delivery and receipt, using the driver declared above

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
	if (!RHReliableDatagramManager.init()) {
		Serial.println(F("init failed"));
	}
	else {
		Serial.println(F("done"));
	}

	// Set the various Parameters of the LoRa Module
	rf95.setTxPower(23, false);
	rf95.setFrequency(915.0);
	rf95.setPreambleLength(12);
	rf95.setModemConfig(RH_RF95::Bw125Cr48Sf4096);

	// Initialize the global variables accessed using the getters and setters to 0
	setMessageIdArrayCounter(0);

	Serial.println(F("RF95 ready"));

	Serial.print(F("mem = "));
	Serial.println(freeMem());
}

// setter for the Message ID Counter
void setMessageIdArrayCounter(uint8_t count) {
	messageIdArrayCounter = count;
}

// getter for the Message ID Counter
uint8_t getMessageIdArrayCounter() {
	return messageIdArrayCounter;
}

void loop() {
	// Need to allocate memory for these
	// Setup the message to be sent
	char ackTx[RH_MESSAGE_ACK_LEN] = "1";
	char messageLifespanTx[RH_MESSAGE_LIFESPAN_LEN] = "0";
	char messageIdTx[RH_MESSAGE_ID_LEN];
	char messageTx[RH_MESSAGE_LEN];									// Could leave this empty. Could also use for sending commands back to End-Node
	// Need to make sure overall message length is the same. Otherwise, the Gateway will enter CAD slightly before any node that is currently in sync with the Gateway and cause the
	// Gateway to exit CAD

	// Don't need to allocate memory since these pointers will just be pointing to where buf points to
	char* ackRx;
	char* messageLifespanRx;
	char* messageIdRx;
	char* messageRx;

	Serial.println(F("MAIN LOOP"));

	// Wait until a message arrives. Also, CAD won't run until the previous message has finished sending
	while (!rf95.isChannelActive()) {
		rf95.sleep();
		Serial.println(F("Loop"));
		delay(100);
	}
	Serial.println(F("Out of Loop"));

	// Preamble detected. Enter receiver mode and wait for the message
	RHReliableDatagramManager.waitAvailableTimeout(WAIT_AVAILABLE_TIMEOUT);

	// Message received. Unpack it and alter it accordingly
	uint8_t len = sizeof(buf);
	uint8_t from;

	/*
		 Turns the receiver on if it not already on. If there is a valid message available for this node, copy it to buf and return true
		 The SRC address is placed in *from if present and not NULL.
		 The DEST address is placed in *to if present and not NULL.
		 If a message is copied, *len is set to the length. You should be sure to call this function frequently enough to not miss any messages
	*/
	if (RHReliableDatagramManager.recvfrom(buf, &len, &from)) {
		Serial.println(F("RECEIVED"));

		// These pointers point to where the pointer buf points to. Any changes done using these pointers will change buf (such as the strtok function)
		ackRx = strtok(buf, "|");
		messageLifespanRx = strtok(NULL, "|");
		messageIdRx = strtok(NULL, "|");
		messageRx = strtok(NULL, "|");

		// Display the received values
		Serial.println(ackRx);
		Serial.println(messageLifespanRx);
		Serial.println(messageIdRx);
		Serial.println(messageRx);

		// Loop through array containing all previously seen message ID's
		for (int i = 0; i < MESSAGE_ID_ARRAY_SIZE; i++) {
			// Check if recieved Message ID matches a previous ID
			if (strcmp(messageIdArray[i], messageIdRx) == 0) {
				// Message ID matches. Message has already been processsed. Do not do anything
				goto doNothing;
			}
		}

		//Haven't seen this message before. Add message to Array, and send ACK back
		uint8_t count = getMessageIdArrayCounter();
		strncpy(messageIdArray[count], messageIdRx, RH_MESSAGE_ID_LEN);
		count++;

		if (count == MESSAGE_ID_ARRAY_SIZE)
			count = 0;
		setMessageIdArrayCounter(count);

		// construct ACK payload and send it. Reset Lifespan, set ACK  to 1, and payload can be empty
		strncpy(messageIdTx, messageIdRx, RH_MESSAGE_ID_LEN);
		strncpy(messageTx, messageRx, RH_MESSAGE_LEN);

		strcpy(payload, ackTx);
		strcat(payload, "|");
		strcat(payload, messageLifespanTx);
		strcat(payload, "|");
		strcat(payload, messageIdTx);
		strcat(payload, "|");
		strcat(payload, messageTx);

		Serial.println("Payload & Size:    ");
		Serial.println(payload);
		Serial.println(sizeof(payload));

		RHReliableDatagramManager.sendto(payload, sizeof(payload), RH_BROADCAST_ADDRESS);
		Serial.println(F("SENT"));


	doNothing:
		Serial.println(F("MEH"));
	}
}