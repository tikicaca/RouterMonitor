#include <EtherCard.h>

#define STATIC 0  // set to 1 to disable DHCP (adjust myip/gwip values below)

// mac address
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x30 };
// ethernet interface ip address
static byte myip[] = { 192,168,0,35 };
// gateway ip address
static byte gwip[] = { 192,168,0,1 };

static byte ftpip[] = { 255,255,255,255 };  //set the address to ping


static byte netmask[] = { 255,255,255,0 };

// Relay to control output
int relayPin7 = 7;

int noResponseCounter = 0;
static uint32_t timer = 5000;
static uint32_t timerReponse;
byte Ethernet::buffer[700];

bool responseReceived = true;

void setup () {
  pinMode(relayPin7, OUTPUT);
  digitalWrite(relayPin7, HIGH);
  delay(500);
  Serial.println("Starting router...");
  hitRelay();  //to start router
  
  delay(30000); //give some time to start router 30s

  Serial.begin(9600);
  Serial.println("Trying to get an IP...");

  Serial.print("MAC: ");
  for (byte i = 0; i < 6; ++i) {
    Serial.print(mymac[i], HEX);
    if (i < 5)
      Serial.print(':');
  }
  Serial.println();
  
  if (ether.begin(sizeof Ethernet::buffer, mymac,8) == 0) 
    Serial.println( "Failed to access Ethernet controller");

#if STATIC
  Serial.println( "Getting static IP.");
  if (!ether.staticSetup(myip, gwip)){
    Serial.println( "could not get a static IP");
  }
  else
  {
    ether.copyIp(ether.netmask, netmask); 
  }
#else

  Serial.println("Setting up DHCP");
  if (!ether.dhcpSetup()){
    Serial.println( "DHCP failed");
  }
#endif
  
  ether.printIp("My IP: ", ether.myip);
  ether.printIp("Netmask: ", ether.netmask);
  ether.printIp("GW IP: ", ether.gwip);
  ether.printIp("DNS IP: ", ether.dnsip);

  delay(5000); //give some time for dhcp setup to be finished
}

void loop () {
 

    word len = ether.packetReceive(); // go receive new packets
    word pos = ether.packetLoop(len); // respond to incoming pings
  
      // ping comes back
      if (len > 0 && ether.packetLoopIcmpCheckReply(ether.hisip)) {
          Serial.print("  ");
          Serial.print((millis() - timer), 3);
          Serial.println(" ms");
          responseReceived = true;
          noResponseCounter = 0;
        }
  
     if (millis() - timerReponse >= 3000)  //timeout 3 seconds
     {
        
            if(!responseReceived)
            {
                if(noResponseCounter <5 )
                {
                    noResponseCounter++;
                    responseReceived = true;
                    Serial.print("No response counter: ");
                    Serial.print(noResponseCounter);
            
                 }
                else
                {
                    Serial.print(" Restarting router");
                    hitRelay();
                    delay(30000); //wait time to restart router 30s
                    responseReceived = true;
                    noResponseCounter = 0; 
                }        
             }
             else
             {
                  //timerReponse = millis(); 
              }   
        }
     
     if (millis() - timer >= 60000)  //60000 every minute
     {

        ether.copyIp(ether.hisip, ftpip); //ftpip  gwip
             
        ether.printIp("Pinging: ", ether.hisip);
        timer = millis();
        timerReponse = millis();
        responseReceived = false;
        ether.clientIcmpRequest(ether.hisip);
    }
}

void hitRelay(){
  
    digitalWrite(relayPin7, LOW);
    delay(500);
    digitalWrite(relayPin7, HIGH);
    //delay(500);
  
}
