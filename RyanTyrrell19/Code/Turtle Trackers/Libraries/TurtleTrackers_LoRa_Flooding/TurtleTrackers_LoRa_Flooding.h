/*
 Name:		TurtleTrackers_LoRa_Flooding.h
 Created:	8/8/2019 11:01:55 AM
 Author:	tyrre
 Editor:	http://www.visualmicro.com
*/

#ifndef _TurtleTrackers_LoRa_Flooding_h
#define _TurtleTrackers_LoRa_Flooding_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#endif

#include <TurtleTracker_Config.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>


class TurtleTrackers_LoRa_Flooding {

public:
	/*
	*	Default Constructor
	*
	*	debugPort:	Serial Port used for providing helpful output on the state of the program
	*/
	TurtleTrackers_LoRa_Flooding(HardwareSerial& debugPort);

	/*
	* Used to initiate the RFM95 LoRa Module, along with the Manager used to send reliable messages. In addition, the LoRa Module is configured here.
	*
	* rf95: reference to the RFM95 object initiated in the main script
	* RHReliableDatagramManager: reference to the RHReliableDatagram object initiated in the main script
	*/
	bool init(RH_RF95& rf95, RHReliableDatagram& RHReliableDatagramManager);

	/*
	*	Used by the End-Node (e.g. Turtle Trackers) to trasnmit their data to the gateway
	*
	*	payload: the data to be transmitted (e.g. GPS coordinates)
	*
	*	returns: whether or not an ACK from the Gateway was received (i.e whther the transmission was successful or not)
	*/
	bool sendData(char* payload);

	/*
	*	Used by intermediate nodes to relay an data received, either from the End-Nodes, other Relayer's, or a Gateway
	*/
	void relayData();

	/*
	*	Used by the Gateway to receive any messages, and re-broadcast an ACK
	*/
	void receiveData();

	/*
	*	Used to set the ENd-Nodes ID by saving it to the microcontrollers EEPROM
	*
	*	nodeId: the ID of the node to save
	*	
	*	returns: whether the DI was saved successfully
	*/
	bool setNodeId(uint8_t nodeId);

	/*
	*	Retrieves the Nodes ID from the microcontrollers EEPROM
	*
	*	returns: the Node's ID
	*/
	uint8_t getNodeId();


private:
	/*
	* Setter/Getter function to set/get the global varaible, _messageIdArrayCounter
	*/
	void setMessageIdArrayCounter(uint8_t count);
	uint8_t getMessageIdArrayCounter();

	/*
	* Setter/Getter function to set/get the global varaible, _messagesSentCounter. Resets to zero if a threshold is exceeded
	*/
	void setMessagesSentCounter(uint8_t count);
	uint8_t getMessagesSentCounter();

	uint8_t _nodeId;

	RH_RF95* _rf95;
	RHReliableDatagram* _RHReliableDatagramManager;

	
	//Used to keep track of the end of _messageIdArray so it is known where to save the next ID
	uint8_t _messageIdArrayCounter;

	//Used to keep track of how many message ID's have already been seen, by saving their ID's. THis array can be thought of as a circular buffer, where the first entry to be\
	//erased is the oldest
	char _messageIdArray[MESSAGE_ID_ARRAY_SIZE][RH_MESSAGE_ID_LEN];

	// Keeps track of which messages seen are an ACK message. The indexes of this array line up with that of _messageIdArray (so you know which ACK goes with which message)
	char _messageAckArray[MESSAGE_ID_ARRAY_SIZE][RH_MESSAGE_ACK_LEN];

	// keeps track of messages sent to determine the newest messageId
	uint8_t _messagesSentCounter;									

	// This is the payload that is tranmistted
	char _payload[RH_PAYLOAD_LEN];

	// Any received data is read into this varaible
	char _buf[RH_PAYLOAD_LEN];

	// Pointer to the Serial object defined in the main program and passed via the constructor
	// Used for providing helpful output on the state of the program
	HardwareSerial* _debugPort;
};