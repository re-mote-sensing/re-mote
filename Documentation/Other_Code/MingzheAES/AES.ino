// AES-128 implementation in CPP
// Author: Myron
/*
TO do next:
Think about the trade-off between speed (time) and code size (space).

```
The creators of AES designed the algorithm in such a way that implementations could make a trade-off between speed and code size. There are 4 possible levels, increasing in size and speed:

0kB - no lookup tables, all steps are calculated, including substitution.
256 bytes x 2 - s-box and inverse-s-box are stored as lookup tables. **(choosen for current implementation)**
4kB x 2 - the Galois field multiplication tables are stored. 
24kB x2 - The entire round (subBytes, shiftRows and mixColumns) are replaced by a lookup table. The only operation left is addRoundKey.

The last is the fastest possible software implementation. Such an implementation can be found in the Go standard library here. If the CPU's cache is large enough to accommodate the entire table, it will be really fast.
```
*/

/* ----------- constant ------------- */
#define AES_BLOCK_SIZE 16

/* ----------- global variables ------------- */
const uint8_t sbox[256] = 
{
  0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
  0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
  0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
  0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
  0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
  0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
  0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
  0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
  0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
  0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
  0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
  0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
  0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
  0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
  0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
  0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

const uint8_t sboxInv[256] = 
{
  0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
  0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
  0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
  0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
  0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
  0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
  0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
  0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
  0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
  0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
  0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
  0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
  0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
  0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
  0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
  0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

const uint8_t RC[11] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36};

/* ----------- set up ------------- */
void setup() {
  // begin Serial at frequency 9600 HZ
  Serial.begin(9600);

  // NOTE: in AES, the input is always a block of 16 bytes (i.e. a byte array with size 16).

  /*------- Substitute Bytes Test --------*/
  // uint8_t input[AES_BLOCK_SIZE] = {0x8e, 0x9f, 0xf1, 0xc6, 0x4d, 0xdc, 0xe1, 0xc7, 0xa1, 0x58, 0xd1, 0xc8, 0xbc, 0x9d, 0xc1, 0xc9};
  // uint8_t expected[AES_BLOCK_SIZE] = {0x19, 0xdb, 0xa1, 0xb4, 0xe3, 0x86, 0xf8, 0xc6, 0x32, 0x6a, 0x3e, 0xe8, 0x65, 0x5e, 0x78, 0xdd};

  // substituteBytes(input);

  // Serial.println("Substitute Bytes Test: [expected 1]");
  // Serial.println(compareTwoByteArr(input, expected, AES_BLOCK_SIZE));

  /*------- Substitute Bytes Inv Test --------*/
  // uint8_t input[AES_BLOCK_SIZE] = {0x19, 0xdb, 0xa1, 0xb4, 0xe3, 0x86, 0xf8, 0xc6, 0x32, 0x6a, 0x3e, 0xe8, 0x65, 0x5e, 0x78, 0xdd};
  // uint8_t expected[AES_BLOCK_SIZE] = {0x8e, 0x9f, 0xf1, 0xc6, 0x4d, 0xdc, 0xe1, 0xc7, 0xa1, 0x58, 0xd1, 0xc8, 0xbc, 0x9d, 0xc1, 0xc9};
  
  // substituteBytesInv(input);

  // Serial.println("Substitute Bytes Inv Test: [expected 1]");
  // Serial.println(compareTwoByteArr(input, expected, AES_BLOCK_SIZE));

  /*------- Shift Row Test --------*/
  // uint8_t input[AES_BLOCK_SIZE] = {0x54, 0x33, 0xab, 0xc1, 0x32, 0x15, 0x8d, 0xbb, 0x5a, 0x73, 0xd5, 0x52, 0x31, 0x91, 0xcc, 0x98};
  // uint8_t expected[AES_BLOCK_SIZE] = {0x54, 0x33, 0xab, 0xc1, 0x15, 0x8d, 0xbb, 0x32, 0xd5, 0x52, 0x5a, 0x73, 0x98, 0x31, 0x91, 0xcc};
  
  // shiftRow(input);

  // Serial.println(F("Shift Row Test: [expected 1]"));
  // Serial.println(compareTwoByteArr(input, expected, AES_BLOCK_SIZE));

  /*------- Shift Row Inv Test --------*/
  // uint8_t input[AES_BLOCK_SIZE] = {0x54, 0x33, 0xab, 0xc1, 0x15, 0x8d, 0xbb, 0x32, 0xd5, 0x52, 0x5a, 0x73, 0x98, 0x31, 0x91, 0xcc};
  // uint8_t expected[AES_BLOCK_SIZE] = {0x54, 0x33, 0xab, 0xc1, 0x32, 0x15, 0x8d, 0xbb, 0x5a, 0x73, 0xd5, 0x52, 0x31, 0x91, 0xcc, 0x98};
  
  // shiftRowInv(input);

  // Serial.println(F("Shift Row Inv Test: [expected 1]"));
  // Serial.println(compareTwoByteArr(input, expected, AES_BLOCK_SIZE));

  /*------- Mix Column Helper Test --------*/
  // uint8_t input[4] = {1, 2, 3, 4};
  // uint8_t expected[4] = {3, 4, 9, 10};
  
  // mixColumnHelper(input);

  // Serial.println(F("Mix Column Helper Test: [expected 1]"));
  // Serial.println(compareTwoByteArr(input, expected, 4));

  /*------- Mix Column Inv Helper Test --------*/
  // uint8_t input[4] = {3, 4, 9, 10};
  // uint8_t expected[4] = {1, 2, 3, 4};
  
  // mixColumnInvHelper(input);

  // Serial.println(F("Mix Column Inv Helper Test: [expected 1]"));
  // Serial.println(compareTwoByteArr(input, expected, 4));
  
  /*------- Mix Column Test --------*/
  // uint8_t input[AES_BLOCK_SIZE] = {1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4};
  // uint8_t expected[AES_BLOCK_SIZE] = {3, 3, 3, 3, 4, 4, 4, 4, 9, 9, 9, 9, 10, 10, 10, 10};

  
  // mixColumn(input);

  // Serial.println(F("Mix Column Test: [expected 1]"));
  // Serial.println(compareTwoByteArr(input, expected, AES_BLOCK_SIZE));

  /*------- Mix Column Inv Test --------*/
  // uint8_t input[AES_BLOCK_SIZE] = {3, 3, 3, 3, 4, 4, 4, 4, 9, 9, 9, 9, 10, 10, 10, 10};
  // uint8_t expected[AES_BLOCK_SIZE] = {1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4};

  // mixColumnInv(input);

  // Serial.println(F("Mix Column Inv Test: [expected 1]"));
  // Serial.println(compareTwoByteArr(input, expected, AES_BLOCK_SIZE));

  /*------- Add Round Key Test --------*/
  // uint8_t input[AES_BLOCK_SIZE] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
  // uint8_t roundKey[AES_BLOCK_SIZE] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 1};
  // uint8_t expected[AES_BLOCK_SIZE] = {3, 1, 7, 1, 3, 1, 15, 1, 3, 1, 7, 1, 3, 1, 31, 17};
  
  // addRoundKey(input, roundKey);

  // Serial.println(F("Add Round Key Test: [expected 1]"));
  // Serial.println(compareTwoByteArr(input, expected, AES_BLOCK_SIZE));

  /*------- Key Expansion Test --------*/
  // uint8_t input[16] = {0x0f, 0x15, 0x71, 0xc9, 0x47, 0xd9, 0xe8, 0x59, 0x0c, 
  //                     0xb7, 0xad, 0xd6, 0xaf, 0x7f, 0x67, 0x98};
  // uint8_t w[176];
  // keyExpansion(input, w);

  // Serial.println("Test result: "); // should have the same result as in: https://github.com/benjimr/AES-Key-Expansion
  // printByteArr(w, 176);

  /*------- arraySlice Test --------*/
  // uint8_t input[16] = {0x0f, 0x15, 0x71, 0xc9, 0x47, 0xd9, 0xe8, 0x59, 0x0c, 
  //                     0xb7, 0xad, 0xd6, 0xaf, 0x7f, 0x67, 0x98};
  // arraySlicetest(input);

  /*-------- transpose Test ---------*/
  // uint8_t input[16] = {0x01, 0x02, 0x03, 0x04, 
  //                      0x05, 0x06, 0x07, 0x08, 
  //                      0x09, 0x10, 0x11, 0x12,
  //                      0x13, 0x14, 0x15, 0x16};
  // uint8_t expected[16] = {0x01, 0x05, 0x09, 0x13, 
  //                         0x02, 0x06, 0x10, 0x14, 
  //                         0x03, 0x07, 0x11, 0x15,
  //                         0x04, 0x08, 0x12, 0x16};
  // transpose(input);
  // printByteArr(input, 16);

  /*------- AES-128 Test --------*/
  // // INPUT
  // uint8_t state[AES_BLOCK_SIZE] = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
  // uint8_t key[AES_BLOCK_SIZE] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};

  // // TEST
  // Serial.println("Key used [key]: ");
  // printByteArr(key, AES_BLOCK_SIZE);
  // Serial.println("----------------------------");
  // Serial.println("BEFORE encryption [state]: ");
  // printByteArr(state, AES_BLOCK_SIZE);
  // Serial.println("----------------------------");
  // uint8_t w[176]; //*******
  // keyExpansion(key, w); //*******
  // Serial.println("Key expension [expkey]: ");
  // printByteArr(w, 176);
  // Serial.println("----------------------------");

  // Serial.println("Encrypt begin [state]:");
  // encrypt(state, w, 10); //*******
  // Serial.println("----------------------------");
  // Serial.println("AFTER encryption [state]: ");
  // printByteArr(state, AES_BLOCK_SIZE);
  // Serial.println("----------------------------");

  // Serial.println("Decrypt begin [state]:");
  // decrypt(state, w, 10); //*******
  // Serial.println("----------------------------");
  // Serial.println("AFTER decryption [state]: ");
  // printByteArr(state, AES_BLOCK_SIZE);
  // Serial.println("----------------------------");

  /*------- API Test --------*/
  // input
  int dataLen = 32;
  uint8_t data[32] = {0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34, 
                      0x32, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
  uint8_t key[AES_BLOCK_SIZE] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
  Serial.println("Original [data]: ");
  printByteArr(data, dataLen);
  // encrypt
  AES_128_encrypt(data, dataLen, key);
  Serial.println("After encryption [data]: ");
  printByteArr(data, dataLen);
  // decrypt
  AES_128_decrypt(data, dataLen, key);
  Serial.println("After decryption [data]: ");
  printByteArr(data, dataLen);
}





/* ----------- loop ------------- */
void loop() {
  
}

/* ----------- API ------------- */
// Use AES-128 to encrypt the data  with the given key. Return true if encrypt successfully.
// Note:
// - This method will mutate the data.
// - This method does NOT deal with the padding automatically.
// - You should use PKCS#7 as the padding strategy before calling this function.
//  For more information about PKCS#7, 
//  see https://en.wikipedia.org/wiki/Padding_(cryptography)#PKCS#5_and_PKCS#7.utate the data.]
boolean AES_128_encrypt(uint8_t data[], int dataLen, uint8_t key[16]) {
  if (dataLen % AES_BLOCK_SIZE != 0) {
    return false;
  }
  int count = dataLen / AES_BLOCK_SIZE;
  Serial.println(count);
  // expense the key
  uint8_t w[176];
  keyExpansion(key, w);
  // encrypt
  for (int i = 0; i < count; i++) {
    encrypt(&data[AES_BLOCK_SIZE * i], w, 10);
  }
  return true;
}

// Use AES-128 to decrypt the data  with the given key. Return true if decrypt successfully.
// Note:
// - This method will mutate the data.
// - This method does NOT deal with the padding automatically.
// - You should use PKCS#7 as the padding strategy before calling this function.
//  For more information about PKCS#7, 
//  see https://en.wikipedia.org/wiki/Padding_(cryptography)#PKCS#5_and_PKCS#7.utate the data.]
boolean AES_128_decrypt(uint8_t data[], int dataLen, uint8_t key[16]) {
  if (dataLen % AES_BLOCK_SIZE != 0) {
    return false;
  }
  int count = dataLen / AES_BLOCK_SIZE;
  // expense the key
  uint8_t w[176];
  keyExpansion(key, w);
  // encrypt
  for (int i = 0; i < count; i++) {
    decrypt(&data[AES_BLOCK_SIZE * i], w, 10);
  }
  return true;
}

/* ----------- AES ------------- */
void encrypt(uint8_t state[AES_BLOCK_SIZE], uint8_t expKey[176], uint8_t rounds) {
  uint8_t keyInd = 0;
  // funciton: addRoundKey(uint8_t state[AES_BLOCK_SIZE], uint8_t roundKey[AES_BLOCK_SIZE])
  addRoundKey(state, &expKey[keyInd]);
  keyInd += 16;
  int i;
  for (i = 0; i < rounds - 1; i++) {
    // Serial.println("( Round " + String(i+1) + " BEGIN )");
    substituteBytes(state);
    shiftRow(state);
    mixColumn(state);
    addRoundKey(state, &expKey[keyInd]);
    keyInd += 16;
    // Serial.println("( Round " + String(i+1) + " END )");
  }
  // Serial.println("( Round " + String(i+1) + " BEGIN )");
  substituteBytes(state);
  shiftRow(state);
  addRoundKey(state, &expKey[keyInd]);
  // Serial.println("( Round " + String(i+1) + " END )");
}

void decrypt(uint8_t state[AES_BLOCK_SIZE], uint8_t expKey[176], uint8_t rounds) {
  uint8_t keyInd = 160;
  addRoundKey(state, &expKey[keyInd]);
  keyInd -= 16;
  shiftRowInv(state);
  substituteBytesInv(state);

  int i;
  for (i = rounds - 2; i >= 0; i--) {
    // Serial.println("( Round " + String(i+1) + " BEGIN )");
    addRoundKey(state, &expKey[keyInd]);
    keyInd -= 16;
    mixColumnInv(state);
    shiftRowInv(state);
    substituteBytesInv(state);
    // Serial.println("( Round " + String(i+1) + " END )");
  }

  // Serial.println("( Round " + String(i+1) + " BEGIN )");
  addRoundKey(state, &expKey[keyInd]);
  // Serial.println("( Round " + String(i+1) + " END )");
}

/* ----------- AES operations ------------- */
// Substitute Bytes
void substituteBytes(uint8_t state[AES_BLOCK_SIZE]) {
  for (uint8_t i = 0; i < AES_BLOCK_SIZE; i++) {
    state[i] = sbox[state[i]];
  }

  // Serial.println("After substituteBytes: ");
  // printByteArr(state, AES_BLOCK_SIZE);
}

// Substitute Bytes Inv
void substituteBytesInv(uint8_t state[AES_BLOCK_SIZE]) {
  for (uint8_t i = 0; i < AES_BLOCK_SIZE; i++) {
    state[i] = sboxInv[state[i]];
  }
}

// Shift Row
void shiftRow(uint8_t state[AES_BLOCK_SIZE]) {
  // transpose as shiftrow is actually shifting cols if you fill the matrix row by row
  transpose(state);
  // creat a temp array
  uint8_t temp[4] = {0, 0, 0, 0};
  // for each row
  for (uint8_t i = 1; i < 4; i++) {
    uint8_t baseInd = 4 * i;
    // store after shifted values in temp:
    // temp[0] = state[baseInd + ((i + 0) % 4)];
    // temp[1] = state[baseInd + ((i + 1) % 4)];
    // temp[2] = state[baseInd + ((i + 2) % 4)];
    // temp[3] = state[baseInd + ((i + 3) % 4)];
    for (uint8_t j = 0; j < 4; j++) {
      temp[j] = state[baseInd + ((i + j) % 4)];
    }
    // replace the old array with temp
    for (uint8_t j = 0; j < 4; j++) {
      state[baseInd + j] = temp[j];
    }
  }
  transpose(state);

  // Serial.println("After shiftRow: ");
  // printByteArr(state, AES_BLOCK_SIZE);
}

// Shift Row Inv
void shiftRowInv(uint8_t state[AES_BLOCK_SIZE]) {
  transpose(state);
  // creat a temp array
  uint8_t temp[4] = {0, 0, 0, 0};
  // for each row
  for (uint8_t i = 1; i < 4; i++) {
    uint8_t baseInd = 4 * i;
    // store after shifted values in temp:
    // temp[0] = state[baseInd + (((4 - i) + 0) % 4)];
    // temp[1] = state[baseInd + (((4 - i) + 1) % 4)];
    // temp[2] = state[baseInd + (((4 - i) + 2) % 4)];
    // temp[3] = state[baseInd + (((4 - i) + 3) % 4)];
    for (uint8_t j = 0; j < 4; j++) {
      temp[j] = state[baseInd + (((4 - i) + j) % 4)];
    }
    // replace the old array with temp
    for (uint8_t j = 0; j < 4; j++) {
      state[baseInd + j] = temp[j];
    }
  }
  transpose(state);
}

// Mix Column
void mixColumn(uint8_t state[AES_BLOCK_SIZE]) {
  transpose(state);
  uint8_t column[4];
  // for each column
  for (uint8_t j = 0; j < 4; j++) {
    // Read the column by doing the following:
    // for each row in a column
    for (uint8_t i = 0; i < 4; i++) {
      column[i] = state[4 * i + j];
    }

    // call the helper function on that column
    mixColumnHelper(column);

    // Write the column back by doing the following:
    // for each row in a column
    for (uint8_t i = 0; i < 4; i++) {
      state[4 * i + j] = column[i];
    }
  }
  transpose(state);

  // Serial.println("After mixColumn: ");
  // printByteArr(state, AES_BLOCK_SIZE);
}

void mixColumnHelper(uint8_t column[4]) {
  // create a temp by copying the column
  uint8_t temp[4];
  for (uint8_t i = 0; i < 4; i++) {
    temp[i] = column[i];
  }
  // update the column by temp
  column[0] = galoisMult(temp[0], 2) 
            ^ galoisMult(temp[1], 3)
            ^ galoisMult(temp[2], 1) 
            ^ galoisMult(temp[3], 1);
  column[1] = galoisMult(temp[0], 1) 
            ^ galoisMult(temp[1], 2)
            ^ galoisMult(temp[2], 3) 
            ^ galoisMult(temp[3], 1);
  column[2] = galoisMult(temp[0], 1) 
            ^ galoisMult(temp[1], 1)
            ^ galoisMult(temp[2], 2) 
            ^ galoisMult(temp[3], 3);
  column[3] = galoisMult(temp[0], 3) 
            ^ galoisMult(temp[1], 1)
            ^ galoisMult(temp[2], 1) 
            ^ galoisMult(temp[3], 2);

}

// Mix Column Inv
void mixColumnInv(uint8_t state[AES_BLOCK_SIZE]) {
  transpose(state);
  uint8_t column[4];
  // for each column
  for (uint8_t j = 0; j < 4; j++) {
    // Read the column by doing the following:
    // for each row in a column
    for (uint8_t i = 0; i < 4; i++) {
      column[i] = state[4 * i + j];
    }

    // call the helper function on that column
    mixColumnInvHelper(column);

    // Write the column back by doing the following:
    // for each row in a column
    for (uint8_t i = 0; i < 4; i++) {
      state[4 * i + j] = column[i];
    }
  }
  transpose(state);
}

void mixColumnInvHelper(uint8_t column[4]) {
  // create a temp by copying the column
  uint8_t temp[4];
  for (uint8_t i = 0; i < 4; i++) {
    temp[i] = column[i];
  }
  // update the column by temp
  column[0] = galoisMult(temp[0], 14) 
            ^ galoisMult(temp[1], 11)
            ^ galoisMult(temp[2], 13) 
            ^ galoisMult(temp[3], 9);
  column[1] = galoisMult(temp[0], 9) 
            ^ galoisMult(temp[1], 14)
            ^ galoisMult(temp[2], 11) 
            ^ galoisMult(temp[3], 13);
  column[2] = galoisMult(temp[0], 13) 
            ^ galoisMult(temp[1], 9)
            ^ galoisMult(temp[2], 14) 
            ^ galoisMult(temp[3], 11);
  column[3] = galoisMult(temp[0], 11) 
            ^ galoisMult(temp[1], 13)
            ^ galoisMult(temp[2], 9) 
            ^ galoisMult(temp[3], 14);
}

// Add round key
void addRoundKey(uint8_t state[AES_BLOCK_SIZE], uint8_t roundKey[AES_BLOCK_SIZE]) {
  for (uint8_t i = 0; i < AES_BLOCK_SIZE; i++) {
    state[i] = state[i] ^ roundKey[i];
  }

  // Serial.println("After addRoundKey: ");
  // printByteArr(state, AES_BLOCK_SIZE);
}

// takes as input a four-word (16-byte) key and produces a linear array of 44 words (176 bytes). 
void keyExpansion(uint8_t key[16], uint8_t w[176]) {
  // temp for holding a word
  uint8_t temp[4];
  // The key is copied into the first four words of the expanded key.
  for (uint8_t i = 0; i < 4; i++) {
    w[4 * i + 0] = key[4 * i + 0];
    w[4 * i + 1] = key[4 * i + 1];
    w[4 * i + 2] = key[4 * i + 2];
    w[4 * i + 3] = key[4 * i + 3];
  }
  // The remainder of the expanded key is filled in four words at a time.
  // see https://www.brainkart.com/article/AES-Key-Expansion_8410/ for detailed explanation.
  for (uint8_t i = 16; i < 176; i += 4) {
    for (uint8_t j = 0; j < 4; j++) {
      temp[j] = w[i - 4 + j];
    }

    if (i % 16 == 0) {
      rotWord(temp);
      subWord(temp);
      // Rcon[i / 16] = (RC[i / 16], 0, 0, 0)
      for (uint8_t k = 0; k < 4; k++) {
        if (k == 0) {
          temp[k] = temp[k] ^ RC[i / 16];
        } else {
          temp[k] = temp[k] ^ 0;
        }
      }
    }

    for (uint8_t m = 0; m < 4; m++) {
      w[i + m] = w[i - 16 + m] ^ temp[m];
    }
  }
}

/* ----------- AES helper functions ------------- */
// transpose a 16 * 16 matrix
void transpose(uint8_t state[AES_BLOCK_SIZE]) {
  uint8_t temp[AES_BLOCK_SIZE];
  // create a copy
  for (uint8_t i = 0; i < AES_BLOCK_SIZE; i++) {
    temp[i] = state[i];
  }
  for (uint8_t i = 0; i < AES_BLOCK_SIZE; i++) {
    // swap col and row
    state[4 * (i % 4) + (i / 4)] = temp[i];
  }
}

// Perform galois mutiply between a and b
uint8_t galoisMult(uint8_t a, uint8_t b) {
  int p = 0;
  int hiBitSet = 0;
  for (uint8_t i = 0; i < 8; i++) {
    if (b & 1 == 1) {
      p ^= a;
    }
    hiBitSet = a & 0x80;
    a <<= 1;
    if (hiBitSet == 0x80) {
      a ^= 0x1b;
    }
    b >>= 1;
  }
  return p % 256;
}

// RotWord performs a one-byte circular left shift on a word. This means that an input word [B0, B1, B2, B3] is transformed into [B1, B2, B3, B0].
void rotWord(uint8_t w[4]) {
  uint8_t temp = w[0];
  w[0] = w[1];
  w[1] = w[2];
  w[2] = w[3];
  w[3] = temp;
}

// SubWord performs a byte substitution on each byte of its input word, using the S-box.
void subWord(uint8_t w[4]) {
  for (uint8_t i = 0; i < 4; i++) {
    w[i] = sbox[w[i]];
  }
}

/* ----------- debug functions ------------- */
// Compare two byte array, return true for equality among all elements.
boolean compareTwoByteArr(uint8_t a[], uint8_t b[], size_t n) {
  for (int i = 0; i < n; i++) {
    if (a[i] != b[i]) {
      return false;
    }
  }
  return true;
}

// Print HEX representation for byte
void printByte(uint8_t b) {
    // Serial.print(F(" 0x"));
    if (b <= 0xF)
        Serial.print(F("0"));
    Serial.print(b, HEX);
}

// Print byte array for debug
void printByteArr(uint8_t a[], size_t n) {
  for (int i = 0; i < n; i++) {
    printByte(a[i]);
    Serial.print(" ");
  }
  Serial.println();
}
