#include <Keypad.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#pragma region Globals
const char* ssid     = "TP-LINK_888"; // Your SSID (Name of your WiFi)
const char* password = "Zhang520521"; //Your Wifi password


WiFiMulti WiFiMulti;
WiFiClient client;

const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {25, 26, 27, 14}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {19, 18, 5, 17}; //connect to the column pinouts of the keypad

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

#define password_lenght 5 // Give enough room for six chars + NULL char

char data[password_lenght]; // 4 is the number of chars it can hold + the null char = 5
char master[password_lenght] = "1234";
byte data_count = 0, master_count = 0;
bool password_corrected = false, alarm_active = false, movement = false, user_informed = false;
char customKey;

int red_led = 13;
int sensor_pin = 39;
unsigned int counter = 0;




const char * DEVID = "v43859040BCC9A33";         // Scenario: "email"
const char * serverName = "api.pushingbox.com";   // Pushingbox API URL

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  delay(10);

  pinMode(sensor_pin, INPUT);
  pinMode(red_led, OUTPUT);


  Connect_to_Wifi();


}

void loop() {

  Check_Keypad();

  if (alarm_active == true) {
    Check_Movement();
  }

  if (alarm_active == true && movement == true && user_informed == false)
  {
    Serial.println("Send Mail");

    // Send_mail();
    sendToPushingBox(DEVID);


  }

}


bool Check_Movement()
{

  if (digitalRead(sensor_pin) == LOW)
  {
    counter++;
    delay(5);
    if (counter >= 200)
    {
      movement = true;
      Serial.println("Movement");
      counter = 0;
    }
  } else movement = false;
}


void Connect_to_Wifi()
{

  // We start by connecting to a WiFi network
  WiFiMulti.addAP(ssid, password);

  Serial.println();
  Serial.println();
  Serial.print("Wait for WiFi... ");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void sendToPushingBox(const char * devid)
{


  client.stop();
  if (client.connect(serverName, 80)) {
    client.print("GET /pushingbox?devid=");
    client.print(devid);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(serverName);
    client.println("User-Agent: test mail");
    client.println();
    client.flush();

    user_informed = true;

  }
  else {
    Serial.println("connection failed");
  }
}



void Check_Keypad()
{
  char customKey = customKeypad.getKey();

  if (customKey) // makes sure a key is actually pressed, equal to (customKey != NO_KEY)
  {

    delay(50);
    digitalWrite(red_led, HIGH);
    delay(50);
    digitalWrite(red_led, LOW);

    data[data_count] = customKey; // store char into data array
    data_count++; // increment data array by 1 to store new char, also keep track of the number of chars entered
  }

  if (data_count == password_lenght - 1) // if the array index is equal to the number of expected chars, compare data to master
  {

    if (!strcmp(data, master)) // equal to (strcmp(Data, Master) == 0)
    {
      //Right password
      Serial.println("Password Correct");

      for (int i = 0; i < 10; i++) {
        delay(50);
        digitalWrite(red_led, HIGH);
        delay(50);
        digitalWrite(red_led, LOW);
      }

      if (alarm_active == true ) {
        // reset alarm
        alarm_active = false;
        Serial.println("Alarm DESACTIVE");

      }
      else
      {
        alarm_active = true;
        user_informed = false;

        int counter = 0;
        while (counter < 5)
        {
          digitalWrite(red_led, HIGH);
          delay(1000);
          digitalWrite(red_led, LOW);
          delay(1000);
          counter++;
        }

        digitalWrite(red_led, LOW);
        Serial.println("Alarm ACTIVE");

      }


    } else {
      Serial.println("Password Wrong");
      digitalWrite(red_led, HIGH);
      delay(1000);
      digitalWrite(red_led, LOW);
    }

    //Clear data
    while (data_count != 0)
    { // This can be used for any array size,
      data[data_count--] = 0; //clear array for new data
    }

  }
}
