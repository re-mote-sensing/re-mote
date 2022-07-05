#ifndef _TurtleTracker_UBX_h_2022
#define _TurtleTracker_UBX_h_2022
#include <TurtleTracker_UBX_Config.h>

/*-----------------------------------UBLOX CONFIGURATION----------------------------------------*/

class TurtleTracker_UBX_2022 {
	public:
		/*
		*	Default Constructor
		* 
		*	debugPort:	Serial Port used for providing helpful output on the state of the program
		*	gpsPort:	Software Serial Port used for communicating with the GPS module
		*/
		TurtleTracker_UBX_2022(HardwareSerial &debugPort, NeoSWSerial &gpsPort);

		/*
		*	Used to initiate the process of configuring the GPS module
		*/
		void configGPS();

	private:
		/*
		*	Used to send a UBX Command to the GPS
		*
		*	MSG: an array containing the hexadecimal command
		*	len: Length of the message
		*/
		void sendUBX(uint8_t* MSG, uint8_t len);

		/*
		*	Checks the messages received from the GPS for an acknowledgment to the commands sent. 
		*
		*	MSG: an array containing the hexadecimal command sent
		*
		*	returns: whether an acknowledgment was received or not
		*/
		bool getUBX_ACK(uint8_t* MSG);

		//	pointer to the Software Serial object defiend in the program and passed via the constructor. 
		//	Used to communicate with the GPS
		NeoSWSerial *_gpsPort;

		// pointer to the Serial object defined in the main program and passed via the constructor
		// Used for providing helpful output on the state of the program
		HardwareSerial* _debugPort;

		// Array containing the command names used to configure the GPS
		char *_config_names[CONFIG_SIZE] = { 
			"AID_INI", 
			"MON_HW", 
			"MON_TXBUF", 
			"NAV_AOPSTATUS", 
			"NAV_DGPS", 
			"NAV_DOP", 
			"NAV_SOL", 
			"NAV_STATUS", 
			"CFG_NAV5", 
			"CFG_NAVX5", 
			"CFG_SBAS", 
			"CFG_CFG" 
		};		

		// Array containing the size (length) of the commands
		const uint8_t _config_size[CONFIG_SIZE] = {
			sizeof(AID_INI) / sizeof(uint8_t),
			sizeof(MON_HW) / sizeof(uint8_t),
			sizeof(MON_TXBUF) / sizeof(uint8_t),
			sizeof(NAV_AOPSTATUS) / sizeof(uint8_t),
			sizeof(NAV_DGPS) / sizeof(uint8_t),
			sizeof(NAV_DOP) / sizeof(uint8_t),
			sizeof(NAV_SOL) / sizeof(uint8_t),
			sizeof(NAV_STATUS) / sizeof(uint8_t),
			sizeof(CFG_NAV5) / sizeof(uint8_t),
			sizeof(CFG_NAVX5) / sizeof(uint8_t),
			sizeof(CFG_SBAS) / sizeof(uint8_t),
			sizeof(CFG_CFG) / sizeof(uint8_t)
		};

		const uint8_t* _config[CONFIG_SIZE] =
		{
			&AID_INI[0],
			&MON_HW[0],
			&MON_TXBUF[0],
			&NAV_AOPSTATUS[0],
			&NAV_DGPS[0],
			&NAV_DOP[0],
			&NAV_SOL[0],
			&NAV_STATUS[0],
			&CFG_NAV5[0],
			&CFG_NAVX5[0],
			&CFG_SBAS[0],
			&CFG_CFG[0]
		};
};
#endif

