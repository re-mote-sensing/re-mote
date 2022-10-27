/*
 Name:		TurtleTrackers_LoRa_Flooding.cpp
 Created:	8/8/2019 11:01:55 AM
 Author:	tyrre
 Editor:	http://www.visualmicro.com
*/

#include "TurtleTrackers_LoRa_Flooding.h"

TurtleTrackers_LoRa_Flooding::TurtleTrackers_LoRa_Flooding(HardwareSerial& debugPort) {
	_debugPort = &debugPort;
}

bool TurtleTrackers_LoRa_Flooding::init(RH_RF95& rf95, RHReliableDatagram& RHReliableDatagramManager) {


	_rf95 = &rf95;
	_RHReliableDatagramManager = &RHReliableDatagramManager;

	// Initialize the RFM95 Driver and Manager
	if (!_RHReliableDatagramManager->init()) {
		return false;
	}

	// Set the various Parameters of the LoRa Module
	_rf95->setTxPower(TX_POWER, false);
	_rf95->setFrequency(FREQUENCY);
	_rf95->setPreambleLength(PREAMBLE_LENGTH);
	//_rf95->setModemConfig(RH_RF95::Bw125Cr48Sf4096);
	
	// Initialize the global variables accessed using the getters and setters to 0
	setMessageIdArrayCounter(0);
	setMessagesSentCounter(0);

	_rf95->sleep();

	return true;
}

bool TurtleTrackers_LoRa_Flooding::sendData(char* data) {

	// Initilize the Arrays (pointers) for the various strings that will be sent/received

	// These are arrays for values received
	char* ackRx;
	char* messageLifespanRx;
	char* messageIdRx;
	char* messageRx;

	// These are arrays for values to be sent
	char ackTx[RH_MESSAGE_ACK_LEN] = "0";
	char messageLifespanTx[RH_MESSAGE_LIFESPAN_LEN] = "0";
	char messageIdTx[RH_MESSAGE_ID_LEN];
	//char messageTx[RH_MESSAGE_LEN] = "0.0000000, 0.0000000, 123456789";

	// Ensures a unique ID will be given to all messages sent
	int id = ((MAX_NUM_END_NODES * getMessagesSentCounter()) + NODE_ID);
	sprintf(messageIdTx, "%d", id);
//	_debugPort->println(messageIdTx);

	// Construct the payload
	strcpy(_payload, ackTx);
	strcat(_payload, "|");
	strcat(_payload, messageLifespanTx);
	strcat(_payload, "|");
	strcat(_payload, messageIdTx);
	strcat(_payload, "|");
	strcat(_payload, data);

	// Display the Payload
	_debugPort->println("Payload:    ");
	_debugPort->println(_payload);
//	_debugPort->println(sizeof(_payload));

	// Add the messageId to the Message Array, and increment the Array Counter. If counter exceeds the size of the array, reset it (this will ensure, once full, the oldest ID is deleted first)
	uint8_t count = getMessageIdArrayCounter();
	strncpy(_messageIdArray[count], messageIdTx, RH_MESSAGE_ID_LEN);
	/*
	Serial.println("messageAckArray:   ");
	for (int i = 0; i < 10; i++) {
		Serial.print(i);
		Serial.print(":   ");
		Serial.println(messageIdArray[i]);
	}
	*/
	count++;
	if (count == MESSAGE_ID_ARRAY_SIZE)
		count = 0;
	setMessageIdArrayCounter(count);

	// Broadcast the message
	_RHReliableDatagramManager->sendto(_payload, sizeof(_payload), RH_BROADCAST_ADDRESS);
	setMessagesSentCounter(getMessagesSentCounter() + 1);
	_debugPort->println(F("SENT"));

	long waitAck = millis();
	//Stay in this loop until the ACK is received or timeout occurs. It is possible this Node will receive a broadcast from another node of the message it originally sent (hence the additional while loop)
	while (millis() - waitAck < ACK_WAIT_TIME) {
		// wait untill the ACK is sent back from the Gateway. Also, CAD won't run until the previous message has finished sending
		do {
			_rf95->sleep();
//			_debugPort->println(F("Loop"));
			delay(CAD_SLEEP);
		} while (!_rf95->isChannelActive());

		_debugPort->println(F("Out of Loop"));

		// Starts the Driver receiver and blocks until a valid received message is available
		_RHReliableDatagramManager->waitAvailableTimeout(WAIT_AVAILABLE_TIMEOUT);

		// Check if message is available for reading. Unpack it and see if it is the message ACK
		uint8_t len = sizeof(_buf);
		uint8_t from;

		/*
		 Turns the receiver on if it not already on. If there is a valid message available for this node, copy it to buf and return true
		 The SRC address is placed in *from if present and not NULL.
		 The DEST address is placed in *to if present and not NULL.
		 If a message is copied, *len is set to the length. You should be sure to call this function frequently enough to not miss any messages
		 */
		if (_RHReliableDatagramManager->recvfrom(_buf, &len, &from)) {

			// These pointers point to where the pointer buf points to. Any changes done using these pointers will change buf (such as the strtok function)
			ackRx = strtok(_buf, "|");
			messageLifespanRx = strtok(NULL, "|");
			messageIdRx = strtok(NULL, "|");
			messageRx = strtok(NULL, "|");

			// Display the received values
			/*_debugPort->println(F("Received Message:   "));
			_debugPort->println(ackRx);
			_debugPort->println(messageLifespanRx);
			_debugPort->println(messageIdRx);
			_debugPort->println(messageRx);
*/
			// See if message matches any message ID's
			for (int i = 0; i < MESSAGE_ID_ARRAY_SIZE; i++) {
				if (strcmp(_messageIdArray[i], messageIdRx) == 0) {
					// Check if it is an ACK
					if (strcmp(ackRx, "0") == 0) {
						_debugPort->println(F("Message ID match & ACK = 0"));
					}
					else if (strcmp(ackRx, "1") == 0) {
						_debugPort->println(F("Message ID match & ACK = 1"));
						return true;
					}
					break;
				}
	//			else
	//				_debugPort->println(F("Message ID NOT a match"));
			}
		}
	}
	return false;
}
void TurtleTrackers_LoRa_Flooding::relayData() {


	// Don't need to allocate memory since these pointers will just be pointing to where buf points to
	char* ackTx;
	char* messageLifespanTx;
	char* messageIdTx;
	char* messageTx;

	char* ackRx;
	char* messageLifespanRx;
	char* messageIdRx;
	char* messageRx;

	_debugPort->println(F("MAIN LOOP"));

	// Wait until a message arrives. Also, CAD won't run until the previous message has finished sending
	do {
		_rf95->sleep();
//		_debugPort->println(F("Loop"));
		delay(CAD_SLEEP);
	} while (!_rf95->isChannelActive());

	_debugPort->println(F("Out of Loop"));

	// Preamble detected. Enter receiver mode and wait for the message
	_RHReliableDatagramManager->waitAvailableTimeout(WAIT_AVAILABLE_TIMEOUT);

	// Message received. Unpack it and alter it accordingly
	uint8_t len = sizeof(_buf);
	uint8_t from;

	/*
		 Turns the receiver on if it not already on. If there is a valid message available for this node, copy it to buf and return true
		 The SRC address is placed in *from if present and not NULL.
		 The DEST address is placed in *to if present and not NULL.
		 If a message is copied, *len is set to the length. You should be sure to call this function frequently enough to not miss any messages
	*/
	if (_RHReliableDatagramManager->recvfrom(_buf, &len, &from)) {
		_debugPort->println(F("RECEIVED"));

		// These pointers point to where the pointer buf points to. Any changes done using these pointers will change buf (such as the strtok function)
		ackRx = strtok(_buf, "|");
		messageLifespanRx = strtok(NULL, "|");
		messageIdRx = strtok(NULL, "|");
		messageRx = strtok(NULL, "|");

		// Display the received values
		//_debugPort->println(ackRx);
		//_debugPort->println(messageLifespanRx);
		//_debugPort->println(messageIdRx);
		//_debugPort->println(messageRx);

		// messageLifespanTx now points to where messageLifespanRx points to
		messageLifespanTx = messageLifespanRx;

		// Check Lifespan counter. Increment lifespan if needed or, if value exceeds limit, do not relay message
		if (strcmp(messageLifespanRx, RH_MESSAGE_LIFESPAN) >= 0) {
			goto doNotRelayMessage;
		}

		// Loop through array containing all previously seen message ID's
		for (int i = 0; i < MESSAGE_ID_ARRAY_SIZE; i++) {
			// Check if recieved Message ID matches a previous ID
			if (strcmp(_messageIdArray[i], messageIdRx) == 0) {
				// We've seen this message ID before. Only relay if this is the ACK, and we have not re-broadcasted the ACK yet
				if (strcmp(ackRx, "1") == 0 && strcmp(_messageAckArray[i], "1") != 0) {
					strncpy(_messageAckArray[i], "1", RH_MESSAGE_ACK_LEN);
					goto relayMessage;
				}
				goto doNotRelayMessage;

				//// Message ID matches. Check if received message ACK is 0 or 1;
				//if (strcmp(ackRx, "0") == 0) {

				//	// ACK is 0. Check to see if we've seen an ACK for this message already.
				//	if (strcmp(messageAckArray[i], "0") == 0) {
				//		// Haven't seen the ACK yet. Relay message
				//		goto relayMessage;
				//	}
				//	// ACK is 0. Check to see if we've seen an ACK for this message already.
				//	else if (strcmp(messageAckArray[i], "1") == 0) {
				//		// We've seen an ACK for this message already. Do not Relay the message (it is expired)
				//		goto doNotRelayMessage;
				//	}
				//}
				//// Message ID matches. Check if received message ACK is 0 or 1;
				//else if (strcmp(ackRx, "1") == 0) {
					// ACK is 1. Relay the message
				/*	if (strcmp(messageAckArray[i], "1") != 0)
						strncpy(messageAckArray[i], "1", RH_MESSAGE_ACK_LEN);
					goto relayMessage;
				}*/
				//exit loop
			}
		}
		//Haven't seen this message before. Add message to Array and Relay Message
		uint8_t count = getMessageIdArrayCounter();
		strncpy(_messageIdArray[count], messageIdRx, RH_MESSAGE_ID_LEN);
		strncpy(_messageAckArray[count], ackRx, RH_MESSAGE_ACK_LEN);
		count++;

		if (count == MESSAGE_ID_ARRAY_SIZE)
			count = 0;
		setMessageIdArrayCounter(count);

		goto relayMessage;


	relayMessage:
		// reconstruct payload and send it
		int lifeSpan = (int)* messageLifespanTx;
		lifeSpan += 1;
		*messageLifespanTx = (char)lifeSpan;

		ackTx = ackRx;
		messageIdTx = messageIdRx;
		messageTx = messageRx;

		strcpy(_payload, ackTx);
		strcat(_payload, "|");
		strcat(_payload, messageLifespanTx);
		strcat(_payload, "|");
		strcat(_payload, messageIdTx);
		strcat(_payload, "|");
		strcat(_payload, messageTx);

		_debugPort->println("Payload:    ");
		_debugPort->println(_payload);
//		_debugPort->println(sizeof(_payload));

		// In case 2 nodes are exactly in sync in terms of receiving a message, introduce a slight delay. Otherwise, they will broadcast at the same time,
		// and they won't enter CAD until after their own message has been sent (in which case they'd have missed the other nodes message)
		//delay(100 * RELAY_ID);
		delay(3000);
		_RHReliableDatagramManager->sendto(_payload, sizeof(_payload), RH_BROADCAST_ADDRESS);
		_debugPort->println(F("SENT"));


	doNotRelayMessage:
		_debugPort->println(F("MEH"));
	}

}
void TurtleTrackers_LoRa_Flooding::receiveData() {

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

	_debugPort->println(F("MAIN LOOP"));

	do {
		_rf95->sleep();
//		_debugPort->println(F("Loop"));
		delay(CAD_SLEEP);
	} while (!_rf95->isChannelActive());

	//// Wait until a message arrives. Also, CAD won't run until the previous message has finished sending
	//while (!_rf95->isChannelActive()) {
	//	_rf95->sleep();
	//	_debugPort->println(F("Loop"));
	//	delay(CAD_SLEEP);
	//}
	_debugPort->println(F("Out of Loop"));

	// Preamble detected. Enter receiver mode and wait for the message
	_RHReliableDatagramManager->waitAvailableTimeout(WAIT_AVAILABLE_TIMEOUT);

	// Message received. Unpack it and alter it accordingly
	uint8_t len = sizeof(_buf);
	uint8_t from;

	/*
		 Turns the receiver on if it not already on. If there is a valid message available for this node, copy it to buf and return true
		 The SRC address is placed in *from if present and not NULL.
		 The DEST address is placed in *to if present and not NULL.
		 If a message is copied, *len is set to the length. You should be sure to call this function frequently enough to not miss any messages
	*/
	if (_RHReliableDatagramManager->recvfrom(_buf, &len, &from)) {
		_debugPort->println(F("RECEIVED"));

		// These pointers point to where the pointer buf points to. Any changes done using these pointers will change buf (such as the strtok function)
		ackRx = strtok(_buf, "|");
		messageLifespanRx = strtok(NULL, "|");
		messageIdRx = strtok(NULL, "|");
		messageRx = strtok(NULL, "|");

		// Display the received values
		//_debugPort->println(ackRx);
		//_debugPort->println(messageLifespanRx);
		//_debugPort->println(messageIdRx);
		//_debugPort->println(messageRx);

		// Loop through array containing all previously seen message ID's
		for (int i = 0; i < MESSAGE_ID_ARRAY_SIZE; i++) {
			// Check if recieved Message ID matches a previous ID
			if (strcmp(_messageIdArray[i], messageIdRx) == 0) {
				// Message ID matches. Message has already been processsed. Do not do anything
				goto doNothing;
			}
		}

		//Haven't seen this message before. Add message to Array, and send ACK back
		uint8_t count = getMessageIdArrayCounter();
		strncpy(_messageIdArray[count], messageIdRx, RH_MESSAGE_ID_LEN);
		count++;

		if (count == MESSAGE_ID_ARRAY_SIZE)
			count = 0;
		setMessageIdArrayCounter(count);

		// construct ACK payload and send it. Reset Lifespan, set ACK  to 1, and payload can be empty
		strncpy(messageIdTx, messageIdRx, RH_MESSAGE_ID_LEN);
		strncpy(messageTx, messageRx, RH_MESSAGE_LEN);

		strcpy(_payload, ackTx);
		strcat(_payload, "|");
		strcat(_payload, messageLifespanTx);
		strcat(_payload, "|");
		strcat(_payload, messageIdTx);
		strcat(_payload, "|");
		strcat(_payload, messageTx);

		_debugPort->println("Payload:    ");
		_debugPort->println(_payload);
//		_debugPort->println(sizeof(_payload));

		_RHReliableDatagramManager->sendto(_payload, sizeof(_payload), RH_BROADCAST_ADDRESS);
		_debugPort->println(F("SENT"));


	doNothing:
		_debugPort->println(F("MEH"));
	}
}

//
//void CadCycle() {
//	do {
//		_rf95->sleep();
//		_debugPort->println(F("Loop"));
//		delay(CAD_SLEEP);
//	} while (!_rf95->isChannelActive());
//}

void TurtleTrackers_LoRa_Flooding::setMessageIdArrayCounter(uint8_t count) {
	_messageIdArrayCounter = count;
}
uint8_t TurtleTrackers_LoRa_Flooding::getMessageIdArrayCounter() {
	return _messageIdArrayCounter;
}

void TurtleTrackers_LoRa_Flooding::setMessagesSentCounter(uint8_t count) {
	_messagesSentCounter = count;

	if (_messagesSentCounter > MAX_SEND_MESSAGES)
		_messagesSentCounter = 0;
}
uint8_t TurtleTrackers_LoRa_Flooding::getMessagesSentCounter() {
	return _messagesSentCounter;
}

bool TurtleTrackers_LoRa_Flooding::setNodeId(uint8_t nodeId) {

	_debugPort->println(F("Setting nodeId..."));

	EEPROM.write(0, nodeId);
	_debugPort->print(F("Set nodeId = "));
	_debugPort->println(nodeId);

	uint8_t readVal = EEPROM.read(0);

	_debugPort->print(F("Read nodeId: "));
	_debugPort->println(readVal);

	if (nodeId == readVal) {
		_debugPort->println(F("Node ID set Successfully"));
		return true;
	}

	_debugPort->println(F("Node ID set Failure"));
	return false;
}
uint8_t TurtleTrackers_LoRa_Flooding::getNodeId() {

	_nodeId = EEPROM.read(0);
	return _nodeId;
}