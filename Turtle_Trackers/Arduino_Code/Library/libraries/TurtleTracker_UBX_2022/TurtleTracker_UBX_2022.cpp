#include "TurtleTracker_UBX_2022.h"

TurtleTracker_UBX_2022::TurtleTracker_UBX_2022(HardwareSerial &debugPort, NeoSWSerial &gpsPort) {
	_debugPort = &debugPort;
	_gpsPort = &gpsPort;
}

void TurtleTracker_UBX_2022::configGPS()
{
	int gps_set_sucess;
	_debugPort->println(F("Configuring GPS..."));

	for (int i = 0; i < CONFIG_SIZE; i++) {
		gps_set_sucess = 0;
		uint8_t* buf = (uint8_t*)malloc(_config_size[i] * sizeof(uint8_t));

		for (uint8_t j = 0; j < _config_size[i]; j++) {
			uint8_t b = pgm_read_byte(&_config[i][j]); // Special operator must be used to retreive the commands from FLASH memory
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

void TurtleTracker_UBX_2022::sendUBX(uint8_t* MSG, uint8_t len) {
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
bool TurtleTracker_UBX_2022::getUBX_ACK(uint8_t* MSG) {
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