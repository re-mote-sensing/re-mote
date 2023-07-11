- dif between lora and lorawan
	- Lora: https://forum.arduino.cc/t/lora-encryption/587754  
      - Data consist simply of a collection of bytes. The radio does not know or care whether the collection being sent is encrypted.
      - There are many different encryption libraries for Arduino. Pick one, learn how to use it, and encrypt the data buffer before sending it by radio. Decrypt at the other end.  
      - encryption library document: https://rweather.github.io/arduinolibs/crypto.html  
      - encryption library src: https://github.com/rweather/arduinolibs  
	- LoraWan: 
      - Architecture: https://lora-alliance.org/about-lorawan/ 
      - Security (AES 128 bits): https://lora-alliance.org/resource_hub/lorawan-is-secure-but-implementation-matters/

- make sure:
  1. noone can receive our data (if received, cannot decrypt)
      - Solution: any encryption (either symmetric or asymmetric) solve this problem
  2. if someone physically get the tracker, they will get the key stored in the tracker, therefore we should not store key (plaintext) in tracker  
      - Candidate solution: 
  3. (provenance) noone can send us faked data 
      - (I think this issue only exists when assymmetric algorithm is selected)
      - Candiate solution: use serials number to distinguish unique hardware
      - Note: currently, id coded in the software (different source codes) 

- some thoughts
  - AES is symmetric encryption. LoranWAN officially use AES 128 bits to secure LoRa physical layer.
  - ![](Symmetric-Encryption.png)
  - ![](Asymmetric-Encryption.png)
  - **I think the main constraint of selecting an encryption should be the energy cost in Encryption stage (where turtle tacker battery is limited), so we need an algorithms that makes sure the above 3 things and has the least Encryption time.**
  - **question: how does the turtle tracker works without the arduino?**


- TODO: 
  - check if moteino (LoRa) has an unique MAC address/encrption module.
  - GPS has a unique id?
  - currently, id coded in the software (different source codes) 
  - think about what happends when we loose trackers and routers?
  - check if there's some implementation Diffie–Hellman in Arduino.
  - moteino: 
  https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf 
  27.3 Signature Bytes

- Dr. Jelle's suggestion: 
  - turtle tracker: we don't care if someone sees the history of a tracker, so sysmetric.
  - water sensor: we want long-term privacy, so asysmetric.
  - Diffie–Hellman in Arduino (search in google)
  - SSL in C: https://bearssl.org/

- task:
  - Diffie–Hellman for bi-direction communication between tracker and gateway.
    - tracker -> gateway: to send the data 
    - gateway -> tracker: to see if the tracker is still alive, remote configure the setting
  - https://blog.arduino.cc/2020/07/02/arduino-security-primer/

  turtle-trackers: automatically generate the key (Diffie–Hellman), and then AES 128.
    - ![Diffie-Hellman](Diffie-Hellman.png)
    - ![AES 128](AES.png)
  water sensor: don't want gateway storage to be hacked, so assymetric. (SSL)
