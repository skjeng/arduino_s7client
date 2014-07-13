/*----------------------------------------------------------------------
Data Read Demo

 Created 19 Nov 2013
 by Davide Nardella
 
------------------------------------------------------------------------
This demo shows how to read data from the PLC.
A DB1 with at least 1024 byte into the PLC is needed.

- Both small and large data transfer are performed (see DO_IT_SMALL)
- During the loop, try to disconnect the ethernet cable.
  The system will report the error and will reconnect automatically
  when you re-plug the cable.
- For safety, this demo *doesn't write* data into the PLC, try
  yourself to change ReadArea with WriteArea.
- This demo uses ConnectTo() with Rack=0 and Slot=2 (S7300)
  - If you want to connect to S71200/S71500 change them to Rack=0, Slot=0.
  - If you want to connect to S7400 see your hardware configuration.
  - If you want to work with a LOGO 0BA7 or S7200 please refer to the
    documentation and change 
    Client.ConnectTo(<IP>, <Rack>, <Slot>);
    with the couple
    Client.SetConnectionParams(<IP>, <LocalTSAP>, <Remote TSAP>);
    Client.Connect();
----------------------------------------------------------------------*/
#include <SPI.h>
#include <Ethernet.h>
#include "Settimino.h"

// Uncomment next line to perform small and fast data access
#define DO_IT_SMALL

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0x90, 0xA2, 0xDA, 0x0F, 0x08, 0xE11 };
  
IPAddress Local(192,168,0,70); // Local Address
IPAddress PLC(192,168,0,5);   // PLC Address

byte Buffer[1024];

S7Client Client;
unsigned long Elapsed; // To calc the execution time
//----------------------------------------------------------------------
// Setup : Init Ethernet and Serial port
//----------------------------------------------------------------------
void setup() {
    // Open serial communications and wait for port to open:
    Serial.begin(9600);
     while (!Serial) {
      ; // wait for serial port to connect. Needed for Leonardo only
    }
    // Start the Ethernet Library
    Ethernet.begin(mac, Local);
    // Setup Time, someone said me to leave 2000 because some 
    // rubbish compatible boards are a bit deaf.
    delay(2000); 
    Serial.println(Ethernet.localIP());
}
//----------------------------------------------------------------------
// Connects to the PLC
//----------------------------------------------------------------------
bool Connect()
{
    int Result=Client.ConnectTo(PLC, 
                                  0,  // Rack (see the doc.)
                                  2); // Slot (see the doc.)
    Serial.print("Connecting to ");Serial.println(PLC);  
    if (Result==0) 
    {
      Serial.print("Connected ! PDU Length = ");Serial.println(Client.GetPDULength());
    }
    else
      Serial.println("Connection error");
    return Result==0;
}
//----------------------------------------------------------------------
// Dumps a buffer (a very rough routine)
//----------------------------------------------------------------------
void Dump(void *Buffer, int Length)
{
  int i, cnt=0;
  pbyte buf;
  
  if (Buffer!=NULL)
    buf = pbyte(Buffer);
  else  
    buf = pbyte(&PDU.DATA[0]);

  Serial.print("[ Dumping ");Serial.print(Length);
  Serial.println(" bytes ]==========================");
  for (i=0; i<Length; i++)
  {
    cnt++;
    if (buf[i]<0x10)
      Serial.print("0");
    Serial.print(buf[i], HEX);
    Serial.print(" ");
    if (cnt==16)
    {
      cnt=0;
      Serial.println();
    }
  }  
  Serial.println("===============================================");
}
//----------------------------------------------------------------------
// Prints the Error number
//----------------------------------------------------------------------
void CheckError(int ErrNo)
{
  Serial.print("Error No. 0x");
  Serial.println(ErrNo, HEX);
  
  // Checks if it's a Severe Error => we need to disconnect
  if (ErrNo & 0x00FF)
  {
    Serial.println("SEVERE ERROR, disconnecting.");
    Client.Disconnect(); 
  }
}
//----------------------------------------------------------------------
// Profiling routines
//----------------------------------------------------------------------
void MarkTime()
{
  Elapsed=millis();
}
//----------------------------------------------------------------------
void ShowTime()
{
  // Calcs the time
  Elapsed=millis()-Elapsed;
  Serial.print("Job time (ms) : ");
  Serial.println(Elapsed);   
}
//----------------------------------------------------------------------
// Main Loop
//----------------------------------------------------------------------
void loop() 
{
  
  int Size, Result;
  void *Target;
   
#ifdef DO_IT_SMALL
  Size=64;
  Target = NULL; // Uses the internal Buffer (PDU.DATA[])
#else
  Size=1024;
  Target = &Buffer; // Uses a larger buffer
#endif
  
  // Connection
  while (!Client.Connected)
  {
    if (!Connect())
      delay(500);
  }
  
  Serial.print("Reading ");Serial.print(Size);Serial.println(" bytes from DB1"); 
  // Get the current tick
  MarkTime();
  Result=Client.WriteArea(S7AreaDB, // We are requesting DB access
                         1,        // DB Number = 1
                         0,        // Start from byte N.0
                         Size,     // We need "Size" bytes
                         Target);  // Put them into our target (Buffer or PDU)
                        
  if (Result==0)
  {
    ShowTime();
    Dump(Target, Size);
  }
  else
    CheckError(Result);
    
  delay(500);  
}



