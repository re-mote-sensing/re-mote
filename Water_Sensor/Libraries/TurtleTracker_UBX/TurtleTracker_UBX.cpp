/*
 Name:		TurtleTracker_UBX.cpp
 Created:	7/29/2019 3:01:12 PM
 Author:	tyrre
 Editor:	http://www.visualmicro.com
*/

#include "TurtleTracker_UBX.h"

TurtleTracker_UBX::TurtleTracker_UBX(HardwareSerial &debugPort, NeoSWSerial &gpsPort) {
	_debugPort = &debugPort;
	_gpsPort = &gpsPort;
}

void TurtleTracker_UBX::configGPS()
{
	int gps_set_sucess;
	_debugPort->println(F("Configuring GPS..."));

	for (int i = 0; i < CONFIG_SIZE; i++) {
		gps_set_sucess = 0;
		uint8_t* buf = (uint8_t*)malloc(_config_size[i] * sizeof(uint8_t));

		for (uint8_t j = 0; j < _config_size[i]; j++) {
			uint8_t b = pgm_read_byte(&_config[i][j]);			// Special operator must be used to retreive the commands from FLASH memory
			buf[j] = b;
		}

		while (!gps_set_sucess) {
			sendUBX(buf, _config_size[i]);
			gps_set_sucess = getUBX_ACK(buf);
		}

		_debugPort->print(F("Successfully configured: "));
		_debugPort->println(_config_names[i]);
		free(buf);
	}
}

void TurtleTracker_UBX::sendUBX(uint8_t* MSG, uint8_t len) {
	_gpsPort->flush();
	_gpsPort->write(0xFF);
	_delay_ms(500);
	for (int i = 0; i < len; i++) {
		_gpsPort->write(MSG[i]);
	}
}
/*
* This function constructs what the ACK package will look like (part of it is based on the command sent).
* Messages received from the GPS are compared with the first byte of the ACK package. After a successful match, 
* the following message is compared to the next byte of the ACK package. This continues until all bytes in the ACK package are successfully matched,
* in which cause this function returns true.
* Any mismatch will cause this cycle to restart.
*/
bool TurtleTracker_UBX::getUBX_ACK(uint8_t* MSG) {
	uint8_t b;
	uint8_t ackByteID = 0;
	uint8_t ackPacket[10];
	unsigned long startTime = millis();

	// Construct the expected ACK packet
	ackPacket[0] = 0xB5; // header
	ackPacket[1] = 0x62; // header
	ackPacket[2] = 0x05; // class
	ackPacket[3] = 0x01; // id
	ackPacket[4] = 0x02; // length
	ackPacket[5] = 0x00;
	ackPacket[6] = MSG[2]; // ACK class
	ackPacket[7] = MSG[3]; // ACK id
	ackPacket[8] = 0; // CK_A
	ackPacket[9] = 0; // CK_B

   // Calculate the checksums
	for (uint8_t ubxi = 2; ubxi < 8; ubxi++) {
		ackPacket[8] = ackPacket[8] + ackPacket[ubxi];
		ackPacket[9] = ackPacket[9] + ackPacket[8];
	}

	while (1) {
		// Test for success
		if (ackByteID > 9) {
			// All packets in order!
			return true;
		}

		// Timeout if no valid response in 3 seconds
		if (millis() - startTime > 3000) {
			_debugPort->println(F("Failed"));
			return false;
		}

		// Make sure data is available to read
		if (_gpsPort->available()) {
			b = _gpsPort->read();

			// Check that bytes arrive in sequence as per expected ACK packet
			if (b == ackPacket[ackByteID]) {
				ackByteID++;
			}
			else {
				ackByteID = 0; // Reset and look again, invalid order
			}
		}
	}
}



// Below is code-in-progress for reading UBX messages sent fom the GPS
/*
void calcChecksum(uint8_t* CK) {
	memset(CK, 0, 2);
	for (int i = 0; i < (int)sizeof(NAV_POSLLH); i++) {
		CK[0] += ((unsigned char*)(&posllh))[i];
		CK[1] += CK[0];
	}
}

uint32_t computePayloadLength(uint8_t* messageRx) {
	uint8_t l1 = *(messageRx + 4);
	uint8_t l2 = *(messageRx + 5);

	//Little Endian Format
	uint16_t payloadLength = l1 + (1 * 256) * l2;

	return payloadLength;

}

fpos == 0 -> Header 1
fpos == 1 -> Header 2
fpos == 2 -> Class
fpos == 3 -> ID
fpos == 4 -> Payload Length 1
fpos == 5 -> Payload Length 2
fpos == 6 + (Payload_Length - 1) -> Payload
fpos == 7 + (Payload_Length - 1) -> Checksum 1
fpos == 8 + (Payload_Length - 1) -> Checksum 2
*/
/*
bool processGPS() {
	uint16_t fpos = 0;
	uint8_t checksum[2]; // Checksum 1 7 2
	uint32_t payloadSize; // initially unknown
	uint8_t* payload;

	uint8_t messageRxSize = 4; // message class, ID, and payload length 1 & 2
	uint8_t* messageRx = (uint8_t*)malloc(sizeof(uint8_t) * messageRxSize);


	while (gpsPort.available()) {
		byte c = gpsPort.read();
		// Header 1 & 2
		if (fpos <= 1) {
			if (c == UBX_HEADER[fpos])
				fpos++;
			else
				fpos = 0;
		}
		else {
			// Message Class, ID, and payload length
			if (fpos >= 2 && fpos <= 5) {
				messageRx[fpos] == c;

				// Compute payload length
				if (fpos == 5) {
					payloadSize = computePayloadLength(messageRx);
					payload = (uint8_t*)malloc(sizeof(uint8_t) * payloadSize);
				}
			}
			// Payload
			else if (fpos < 5 + payloadSize) {
				payload[fpos] = c;
			}
			fpos++;

			// End of payload
			if (fpos == 5 + payloadSize) {
				calcChecksum(checksum);
			}
			// Checksum 1
			else if (fpos == 6 + payloadSize) {
				if (c != checksum[0])
					fpos = 0;
			}
			// Checksum 2
			else if (fpos == 7 + payloadSize) {
				fpos = 0;
				if (c == checksum[1]) {
					return true;
				}
			}
			// to cover all potential causes for fpos
			else if (fpos >= 8 + payloadSize) {
				fpos = 0;
			}
		}
	}
	return false;
}
*/

/*
bool readUBX() {
	uint8_t b;
	uint8_t header[2];
	uint8_t byte_count = 0;
	bool new_payload = false;

	header[0] = 0xB5;
	header[1] = 0x62;

	while (1) {

		if (byte_count == 2)
			return true;

		// Make sure data is available to read
		if (gpsPort.available()) {
			b = gpsPort.read();

			if (b = byte_count = header[new_payload]) {
				byte_count++;
			}
			else {
				byte_count = 0;
			}
		}
	}
}
*/