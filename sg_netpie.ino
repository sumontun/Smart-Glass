#include <ESP8266WiFi.h>
#include <MicroGear.h>
#include <CapacitiveSensor.h>
const char* ssid     = "Vivo20";       // WiFi ssid
const char* password = "12345687";

#define APPID   "SmartGlass"              // use group AppID 
#define KEY     "lNDk4cwwx7vUkhi"                // use group Key
#define SECRET  "BLFqrcGSkHFSNAd17vRx98ny1"             // use group Secret
#define ALIAS   "smartglass"
#define CVALUE "/cap/" ALIAS
#define LEDPIN 5
 
/*
 * CapitiveSense Library Demo Sketch
 * Paul Badger 2008
 * Uses a high value resistor e.g. 10M between send pin and receive pin
 * Resistor effects sensitivity, experiment with values, 50K - 50M. Larger resistor values yield larger sensor values.
 * Receive pin is the sensor pin - try different amounts of foil/metal on this pin
 */

CapacitiveSensor   cs_4_2 = CapacitiveSensor(4,2);        // 10M resistor between pins 4 & 2, pin 2 is sensor pin, add a wire and or foil if desired
WiFiClient client;
int timer = 0;
int currentLEDState = 1;
int  lastCRead = 0;
MicroGear microgear(client);

void onMsghandler(char *topic, uint8_t* msg, unsigned int msglen) {
    Serial.print("Incoming message --> ");
    msg[msglen] = '\0';
    Serial.println((char *)msg); 
}

void onFoundgear(char *attribute, uint8_t* msg, unsigned int msglen) {
    Serial.print("Found new member --> ");
    for (int i=0; i<msglen; i++)
        Serial.print((char)msg[i]);
    Serial.println();  
}

void onLostgear(char *attribute, uint8_t* msg, unsigned int msglen) {
    Serial.print("Lost member --> ");
    for (int i=0; i<msglen; i++)
        Serial.print((char)msg[i]);
    Serial.println();
}

void onConnected(char *attribute, uint8_t* msg, unsigned int msglen) {
    Serial.println("Connected to NETPIE...");
    /* Set the alias of this microgear ALIAS */
    microgear.setAlias(ALIAS);
}

void setup()                    
{
    /* Call onMsghandler() when new message arraives */
    microgear.on(MESSAGE,onMsghandler);

    /* Call onFoundgear() when new gear appear */
    microgear.on(PRESENT,onFoundgear);

    /* Call onLostgear() when some gear goes offline */
    microgear.on(ABSENT,onLostgear);

    /* Call onConnected() when NETPIE connection is established */
    microgear.on(CONNECTED,onConnected);

    Serial.begin(115200);
    Serial.println("Starting...");
    /* Initial WIFI, this is just a basic method to configure WIFI on ESP8266.                       */
    /* You may want to use other method that is more complicated, but provide better user experience */
    if (WiFi.begin(ssid, password)) {
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
    }

    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    /* Initial with KEY, SECRET and also set the ALIAS here */
    microgear.init(KEY,SECRET,ALIAS);

    /* connect to NETPIE to a specific APPID */
    microgear.connect(APPID);
}

void loop()                    
{
  if (microgear.connected())
  {
     Serial.println("connected");

        /* Call this method regularly otherwise the connection may be lost */
        microgear.loop();
        ReadCap();
    }
    else {
        Serial.println("connection lost, reconnect...");
        if (timer >= 5000) 
        {
            microgear.connect(APPID);
            timer = 0;
        }
        else timer += 100;
    }
    delay(100);
}
void ReadCap ()
{
  if(millis() - lastCRead>2000)
  {
  cs_4_2.set_CS_AutocaL_Millis(0xFFFFFFFF);     // turn off autocalibrate on channel 1 - just as an example
   pinMode(LEDPIN, OUTPUT);
   digitalWrite(LEDPIN, HIGH);  //LED OFF
    /* Add Event listeners */
    //long start = millis();
    int total1 =  cs_4_2.capacitiveSensor(30);
    lastCRead = millis();

    //Serial.print(millis() - start);        // check on performance in milliseconds
    //Serial.print("\t");                    // tab character for debug windown spacing
    Serial.println(total1); 

    if (isnan(total1))
    {
      Serial.println("Failed to read data!");
      }
      else{
    Serial.print("Sending -->");
    microgear.publish(CVALUE,total1);
      }  
}
}
