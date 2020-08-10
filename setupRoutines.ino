/********************************************************************************
  This file contains functions used to configure hardware peripherals and various libraries.
********************************************************************************/

/********************************************************************************
  Configure peripherals and system
********************************************************************************/
void setup()
{
  //--------------------------------------------
  Serial.begin(115200);         // Initialize Serial Connection for debug
  while ( !Serial && millis() < 20 );

  pinMode(LED_PIN, OUTPUT);      // initialize on board LED control pin as an output.
  digitalWrite(LED_PIN, HIGH);    // Turn LED on

  // assign RGB LED pins to channels
  ledcAttachPin(LED_R_PIN, LED_R_CHANNEL);
  ledcAttachPin(LED_G_PIN, LED_G_CHANNEL);
  ledcAttachPin(LED_B_PIN, LED_B_CHANNEL);

  // Initialize PWM channels
  // channels 0-15, resolution 1-16 bits, freq limits depend on resolution
  // ledcSetup(uint8_t channel, uint32_t freq, uint8_t resolution_bits);
  ledcSetup(LED_R_CHANNEL, 12000, 8); // 12 kHz PWM, 8-bit resolution
  ledcSetup(LED_G_CHANNEL, 12000, 8);
  ledcSetup(LED_B_CHANNEL, 12000, 8);


  //turn on LEDs
  ledcWrite(LED_R_CHANNEL, 255);
  ledcWrite(LED_G_CHANNEL, 255);
  ledcWrite(LED_B_CHANNEL, 255);

  delay(1000);
  //turn off LEDs
  ledcWrite(LED_R_CHANNEL, 0);
  ledcWrite(LED_G_CHANNEL, 0);
  ledcWrite(LED_B_CHANNEL, 0);

  delay(1000);

  ledcWrite(LED_R_CHANNEL, 255);
  delay(500);
  ledcWrite(LED_R_CHANNEL, 0);
  delay(500);

  ledcWrite(LED_G_CHANNEL, 255);
  delay(500);
  ledcWrite(LED_G_CHANNEL, 0);
  delay(500);

  ledcWrite(LED_B_CHANNEL, 255);
  delay(500);
  ledcWrite(LED_B_CHANNEL, 0);


  //--------------------------------------------
  // Optional Features of EspMQTTClient
  //WiFiMQTTclient.enableDebuggingMessages(); // Enable MQTT debugging messages sent to serial output
  WiFiMQTTclient.enableHTTPWebUpdater();    // Enable the web firmware updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overwritten with enableHTTPWebUpdater("user", "password").
  //  WiFiMQTTclient.enableLastWillMessage("IoT/lastwill", "Goodbye");  // You can activate the retain flag by setting the third parameter to true
  WiFiMQTTclient.enableMQTTConnect();       //use this to enable MQTT connections.  If you comment this line out than you can use this with a WiFi only connection.

  WiFiMQTTclient.enableMACaddress_for_ClientName(); //This will use the WiFi hardware MAC address for the client name instead of the value
}


// Handler for receiving MQTT message
void set_Handler (const String & payload) {
  Serial.print("received MQTT topic IOT/set2: ");
  Serial.println(payload);
}

void rgb2_Handler (const String & payload) {
  Serial.print("received MQTT topic PJsensors/rgb2: ");
  Serial.println(payload);

  StaticJsonDocument<100> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, payload);

  int b = doc["b"];
  int r = doc["r"];
  int g = doc["g"];

  ledcWrite(LED_R_CHANNEL, r);
  ledcWrite(LED_G_CHANNEL, g);
  ledcWrite(LED_B_CHANNEL, b);

}


// Handler for receiving MQTT message
void allMSG_Handler (const String & topic, const String & payload) {
  Serial.print("received MQTT via wildcard: ");
  Serial.println(topic + ": " + payload);
}

/********************************************************************************
  This function is called once WiFi and MQTT connections are complete
********************************************************************************/
void onConnectionEstablished() {

  if (!initialConnectionEstablished_Flag) {     //execute this the first time we have established a WiFi and MQTT connection after powerup
    initialConnectionEstablished_Flag = true;


#ifdef _enableNTP
    //--------------------------------------------
    //  sync local time to NTP server
    configTime(TIME_ZONE * 3600, DST, "pool.ntp.org", "time.nist.gov");
#endif

    //--------------------------------------------
    //  display IP address.
    Serial.println("\nIP address: ");
    Serial.println(WiFi.localIP());

    // Subscribe to "jake/set"
    WiFiMQTTclient.subscribe("jake/set", set_Handler);

    // Subscribe to "jake/#"
    WiFiMQTTclient.subscribe("jake/#", allMSG_Handler);

    // Subscribe to "PJsensors/rgb2"
    WiFiMQTTclient.subscribe("PJsensors/rgb2", rgb2_Handler);



    Serial.println("");
    Serial.println(WiFiMQTTclient.getMqttClientName());
    Serial.println(WiFiMQTTclient.getMqttServerIp());
    Serial.println(WiFiMQTTclient.getMqttServerPort());
    Serial.println(WiFiMQTTclient.getConnectionEstablishedCount());


#ifdef _enableNTP
    //wait for time to sync from NTP server
    while (time(nullptr) <= 100000) {
      delay(20);
    }

    time_t now = time(nullptr);   //get current time
    struct tm * timeinfo;
    time(&now);
    timeinfo = localtime(&now);

    Serial.print("Unix epoch time is: ");
    Serial.println(now);
    char formattedTime [30];
    strftime (formattedTime, 30, "%r", timeinfo); // http://www.cplusplus.com/reference/ctime/strftime/
    Serial.print("Regular time is: ");
    Serial.println(formattedTime);
#endif

  }


  else {
    // we have a new connection but it is not the first connection after power up.

  }

}
