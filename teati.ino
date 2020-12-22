#include <Crypto.h>
#include <AES.h>
#include <string.h>

uint8_t key[] =   {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};      //test key
//uint8_t test_vector[] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};

AES128 AES;
//AES256 AES;

byte buffer[16];
byte text[16];
byte date[4] = {30,8,19,22};      //put date here(gps/rtc) to create responses unique to each day.

uint64_t encoded;
byte encoded_arr[8];
char inputString[21];         // a String to hold incoming data
uint8_t inputIndex = 0;
bool stringComplete = false;  // whether the string is complete


uint64_t encode_base36(const char* str) 
{
  uint64_t _encoded = 0;
  for (const char *p = (str + strlen(str) - 1); p >= str; p-- ) 
  { 
    _encoded *= 36;
    if (*p >= 'A' && *p <= 'Z') // 1-26
      _encoded += *p - 'A';
    else if (*p >= 'a' && *p <= 'z') // 1-26
      _encoded += *p - 'a';
    else if (*p >= '0' && *p <= '9') // 27-36
      _encoded += *p - '0' + 26;
  }
  return _encoded;
}

void b64ToByte(uint64_t in, byte* out)
{
  for(int i=0;i<8;i++)
    {out[i] = in & 0xFF;  
     in = in>>8;
    }
}

uint64_t byteTo64(byte* in)
{
  uint64_t out = 0;
  for(int i=7; i>0; i--)
    {out += in[i];  
     out = out << 8;
    }
    out += in[0]; 
    return out;
}

String decode_base36(uint64_t base36) 
{ 
  String str = "";
 
  while ( base36 > 0) {
    str += "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"[base36 % 36];
    base36 /= 36;
  }
  
  return str;
}

void print_array(byte* arr)
{
  Serial.print("arr:");
  for(int i=0;i<16;i++)
  {//Serial.print("{0x");
   if(arr[i]<0x10) Serial.print("0");
   Serial.print(arr[i],HEX);
   Serial.print(",");
  }
   Serial.println("");
}

void print_64(uint64_t num)
{ String str = "64: 0x";
  byte n;
  for(int i=0; i<8;i++)
   { n = (num >> (7-i)*8) &0xFF;
     if(n < 0x10)                 //pad 0
        str+= "0";
    str+= String(n,HEX); 
    str+=",";
   }
  Serial.println(str);
}

void setup() 
{
  Serial.begin(9600);  
  AES.setKey(key,AES.keySize());        // Set encryption/decryption key for cipher
  Serial.println("--- Parola teatisi için hazırız ---");
}

void loop() 
{
  if (stringComplete) 
  {
    Serial.println(inputString);
    encoded = encode_base36(inputString);   // Convert alphanumeric input to bytes
    b64ToByte(encoded, encoded_arr);
    
    memcpy(text,date,4);     
    text[4]=44;text[5]=55;text[6]=66;text[7]=77;        //some random bytes
    memcpy(text+8,encoded_arr,8);            
    //print_array(text);
    //print_64(encoded);
    
    AES.encryptBlock(buffer, text);       //encryp
    //memcpy(buffer,text,16);            // encryption subst.
    
    memcpy(encoded_arr,buffer+8,8);          //get last 8 bytes
    encoded = byteTo64(encoded_arr);
    //print_array(buffer);
    //print_64(encoded);
    Serial.print("  -->  ");
    Serial.println(decode_base36(encoded));      //convert into human readable form and print
    
    text[4]=15;text[5]=26;text[6]=37;text[7]=48;
    
    Serial.print("  <--  ");
    AES.encryptBlock(buffer, text);       //encryp
    //memcpy(buffer,text,16);            // encryption subst.
    
    memcpy(encoded_arr,buffer+8,8);          //get last 8 bytes
    encoded = byteTo64(encoded_arr);
    //print_array(buffer);
    //print_64(encoded);
    Serial.println(decode_base36(encoded));      //convert into human readable form and print
    Serial.println("------------");
    
    // clear the string:
    inputIndex = 0;
    stringComplete = false;
  }

}



void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    
    if (inChar == '\n') 
    {
      inputString[inputIndex] = 0;    //terminate string
      stringComplete = true;
    }
    else if(inChar != '\r')
      inputString[inputIndex++] = inChar;
  }
}
