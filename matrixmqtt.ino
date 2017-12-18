#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <PubSubClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

// WiFi and MQTT settings
const char* ssid = "...";
const char* password = "...";
const char* mqtt_server = "...";
const char* mqtt_user = "..."; 
const char* mqtt_pass = "...";

WiFiClient espClient;
PubSubClient client(espClient);

// D2 on WEMOS D1
#define PIN             4
#define NUMPIXELS      100

// Declare variables
String matrixMode;
String strTopic;
String strPayload;
int tetrisPieces[7][4] = {{0,1,2,3},{0,1,2,10},{0,1,2,12},{0,1,10,11},{0,1,11,12},{0,1,2,11},{1,2,10,11}};
int pieceColor[7][3] = {{0,75,75},{0,0,150},{100,50,0},{0,50,50},{0,150,0},{50,0,100},{150,0,0}};
int VUColors[10][3] = {{0,219,22},{3,250,11},{142,250,0},{193,250,5},{217,250,0},{250,192,0},{250,142,0},{250,83,0},{250,58,0},{250,0,0}};
int numberZero[12] = {0,1,2,10,20,30,40,41,42,12,22,32}; 
int numberOne[5] = {1,11,21,31,41};
int numberTwo[11] = {0,1,2,10,20,21,22,32,40,41,42};
int numberThree[11] = {0,1,2,12,20,21,22,32,42,41,40};
int numberFour[9] = {2,12,20,21,22,30,32,40,42};
int numberFive[11] = {0,1,2,12,22,21,20,30,40,41,42};
int numberSix[12] = {0,1,2,10,12,22,21,20,30,40,41,42};
int numberSeven[7] = {2,12,22,32,42,41,40};
int numberEight[13] = {0,1,2,10,12,20,21,22,30,32,40,41,42};
int numberNine[10] = {2,12,20,21,22,30,32,40,41,42};
int numberOffsets[4] = {51,55,1,5};
int clockTime[4] = {1,4,2,6};
int pw[22] = {30,40,50,60,70,51,52,62,71,72,44,54,64,74,35,46,56,37,48,58,68,78};
int randomPiece;
int randomPosX;
int lastPiece = 8;
bool canBePlaced = true;
int lightMode = 8;
int interruptPin = 2;
unsigned long previousMillis = 0;
const long interval = 500;
unsigned long clockMillis = 0;
const long clockUpdateInterval = 10000;
bool skipMode = false;
String htmlmessage = "<head>";
String drawHtml ="<html>";
String jsonResult = "";
int drawColor[3] = {255,255,255};
int connection = 0;

// Variables for ltc
int lastLTCprice = 0;
int LTCInSEK = 0;
int LTCInK = 0;
int LTCInH = 0;
bool priceRise = true;

// Using the neopixel library, create a pixel object with the settings specified
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient", mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.subscribe("ha/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) 
{
  payload[length] = '\0';
  strTopic = String((char*)topic);
  if(strTopic == "ha/matrix")
  {
    matrixMode = String((char*)payload);
    if(matrixMode == "Clock")
    {
      lightMode = 0;
      Serial.println(matrixMode);
      clearAll();
    }
    else if (matrixMode == "Night Clock")
    {
      lightMode = 7;
      Serial.println(matrixMode);
      clearAll();
    }
    else if (matrixMode == "Cube effect")
    {
      lightMode = 3;
      Serial.println(matrixMode);
      clearAll();
    }
    else if (matrixMode == "Random Pixel")
    {
      lightMode = 1;
      Serial.println(matrixMode);
      clearAll();
    }
    else if (matrixMode == "Equalizer")
    {
      lightMode = 4;
      Serial.println(matrixMode);
      clearAll();
    }
    else if (matrixMode == "Black")
    {
      lightMode = 5;
      Serial.println(matrixMode);
      clearAll();
    }
    else if (matrixMode == "LTC")
    {
      lightMode = 8;
      Serial.println(matrixMode);
      clearAll();
    }
  }
}

// <summary>
// Used in main loop and is what runs the specified light mode
// </summary>
void cycleLightMode()
{
  switch (lightMode) {
      case 0:
        clock();
        break;
      case 1:
        randomToBlack();
        break;
      case 2:
        tetris();
        break;
      case 3:
        crazy();
        break;
      case 4:
        VU();
        break;
      case 5:
        clearAll();
         break;
      case 6:
         //Draw
         break;
      case 7:
         nightClock();
         break;
      case 8:
         displayLTC();
         break;   
      default:
        clearAll();
    }
  delay(100);
}

// <summary>
// Goes through all the pixels and sets them to an off-state
// </summary>
void clearAll()
{
  for(int i = 0;i<100;i++){
    pixels.setPixelColor(i, pixels.Color(0,0,0));
  }
  pixels.show();
}

// <summary>
// Goes through a couple of different transitions with the specified colorss
// </summary>
void crazy()
{
  skipMode = false;
  if (!skipMode)
  {
    sidewaysCubeIn(200,200,0);
  }
  checkMQTTForUpdate(3);
  if (!skipMode)
  {
    cubeDownLeft(0,200,0);
  }
  checkMQTTForUpdate(3);
  if (!skipMode)
  {
    cubeDownRight(0,0,200);
  }
  checkMQTTForUpdate(3);
  if (!skipMode)
  {
    sidewaysCubeIn(0,200,0);
  }
  checkMQTTForUpdate(3);
  if (!skipMode)
  {
    cubeDownLeft(200,0,200);
  }
  checkMQTTForUpdate(3);
  if (!skipMode)
  {
    cubeDownRight(0,200,200);
  }
  checkMQTTForUpdate(3);
  if (!skipMode)
  {
    cubeUpLeft(200,0,0); 
  }
}

// <summary>
// Visualizes a random tetris game
// Works by loading in tetris pieces from an array with the corresponding colors of that piece
// It then spawn in a random position and moves on row down at the time.
// If there is a colored led under one of the pieces pixels, it's not allowed to go down further
// </summary>
void tetris()
{
  clearAll();
  for (int j = 0; j < 10; j++)
  {
    if (!skipMode)
    {
      randomPiece = random(7);
      randomPosX = random(7);
      if (randomPiece == lastPiece && randomPiece != 6 && lastPiece != 6)
      {
        randomPiece++;
      }
      lastPiece = randomPiece;
  
        for(int a = 90; a >= 0; a = a-10)
        {
          if (!skipMode)
          {
            canBePlaced = true;
            if (randomPiece == 0)
            {
              if (pixels.getPixelColor(tetrisPieces[randomPiece][0]+a+randomPosX) != 0 || pixels.getPixelColor(tetrisPieces[randomPiece][1]+a+randomPosX) != 0 || pixels.getPixelColor(tetrisPieces[randomPiece][2]+a+randomPosX) != 0 || pixels.getPixelColor(tetrisPieces[randomPiece][3]+a+randomPosX) != 0)
              {canBePlaced = false;}
            }
            else if (randomPiece == 1 || randomPiece == 2 || randomPiece == 5)
            {
              if (pixels.getPixelColor(tetrisPieces[randomPiece][0]+a+randomPosX) != 0 || pixels.getPixelColor(tetrisPieces[randomPiece][1]+a+randomPosX) != 0 || pixels.getPixelColor(tetrisPieces[randomPiece][2]+a+randomPosX) != 0)
              {canBePlaced = false;}
            }
            else if (randomPiece == 3 || randomPiece == 4 || randomPiece == 6)
            {
              if (pixels.getPixelColor(tetrisPieces[randomPiece][0]+a+randomPosX) != 0 || pixels.getPixelColor(tetrisPieces[randomPiece][1]+a+randomPosX) != 0)
              {canBePlaced = false;}
            }
            
            for (int i = 0; i < 4; i++)
            {
              if (canBePlaced)
              {
                pixels.setPixelColor(tetrisPieces[randomPiece][i]+a+10+randomPosX, pixels.Color(0,0,0));
                pixels.setPixelColor(tetrisPieces[randomPiece][i]+a+randomPosX, pixels.Color(pieceColor[randomPiece][0],pieceColor[randomPiece][1],pieceColor[randomPiece][2]));
              }
              else
              {
                i = 4;
              }
            }
          }
          else
          {
            a = 0;
          }
          pixels.show();
          delay(500);
      }
    }
    else
    {
      j = 10;
    }
  }
  clearAll();
}

// <summary>
// A transition that is visually like a cube rotated 45 degrees srinking in size
// Works by using a for loop and 4 lines that's tilted 45 degrees that then goes
// further and further towards the middle
// </summary>
// <param name="r" type="int" values="0-255">The amount of red in the transition</param>
// <param name="g" type="int" values="0-255">The amount of green in the transition</param>
// <param name="b" type="int" values="0-255">The amount of blue in the transition</param>
void sidewaysCubeIn(int r, int g, int b)
{
  for (int i = 0; i < 9; i++)
  { 
    if (!skipMode)
    {
      if (i == 0)
      {
        pixels.setPixelColor(0, pixels.Color(r,g,b));
        pixels.setPixelColor(9, pixels.Color(r,g,b));
        pixels.setPixelColor(90, pixels.Color(r,g,b));
        pixels.setPixelColor(99, pixels.Color(r,g,b));
        pixels.show();
        delay(20);
      }
      else
      {
        pixels.setPixelColor(i, pixels.Color(r,g,b));
        pixels.setPixelColor(10*i+9, pixels.Color(r,g,b));
        pixels.setPixelColor(90+i, pixels.Color(r,g,b));
        pixels.setPixelColor(99-10*i, pixels.Color(r,g,b));

       for (int j = 0; j < i; j++)
        {
          pixels.setPixelColor(i+(9*j)+9, pixels.Color(r,g,b));
          pixels.setPixelColor(9-i+(11*j),pixels.Color(r,g,b));
          pixels.setPixelColor(90-(10*i)+(11*j),pixels.Color(r,g,b));
          pixels.setPixelColor((99-i)-(9*j),pixels.Color(r,g,b));
        }
      }
      pixels.show();
      delay(20);
    }
    else
    {
      i = 9;
    }
    delay(50);
  }
}

// <summary>
// Transition the visually looks like the whole matrix is shrinking towards the bottom right
// </summary>
// <param name="r" type="int" values="0-255">The amount of red in the transition</param>
// <param name="g" type="int" values="0-255">The amount of green in the transition</param>
// <param name="b" type="int" values="0-255">The amount of blue in the transition</param>
void cubeDownRight(int r, int g, int b)
{
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 10; j++)
    {
      pixels.setPixelColor(i+j*10, pixels.Color(r,g,b));
      pixels.setPixelColor(90-10*i+j, pixels.Color(r,g,b));
    }

    pixels.show();
    delay(50);
  }
}

// <summary>
// Transition the visually looks like the whole matrix is shrinking towards the bottom left
// </summary>
// <param name="r" type="int" values="0-255">The amount of red in the transition</param>
// <param name="g" type="int" values="0-255">The amount of green in the transition</param>
// <param name="b" type="int" values="0-255">The amount of blue in the transition</param>
void cubeDownLeft(int r, int g, int b)
{
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 10; j++)
    {
      pixels.setPixelColor(9-i+j*10, pixels.Color(r,g,b));
      pixels.setPixelColor(90-10*i+j, pixels.Color(r,g,b));
    }

    pixels.show();
    delay(50);
  }
}

// <summary>
// Transition the visually looks like the whole matrix is shrinking towards the top right
// </summary>
// <param name="r" type="int" values="0-255">The amount of red in the transition</param>
// <param name="g" type="int" values="0-255">The amount of green in the transition</param>
// <param name="b" type="int" values="0-255">The amount of blue in the transition</param>
void cubeUpRight(int r, int g, int b)
{
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 10; j++)
    {
      pixels.setPixelColor(i+j*10, pixels.Color(r,g,b));
      pixels.setPixelColor(9+10*i-j, pixels.Color(r,g,b));
    }

    pixels.show();
    delay(50);
  }
}

// <summary>
// Transition the visually looks like the whole matrix is shrinking towards the top left
// </summary>
// <param name="r" type="int" values="0-255">The amount of red in the transition</param>
// <param name="g" type="int" values="0-255">The amount of green in the transition</param>
// <param name="b" type="int" values="0-255">The amount of blue in the transition</param>
void cubeUpLeft(int r, int g, int b)
{
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 10; j++)
    {
      pixels.setPixelColor(9-i+j*10, pixels.Color(r,g,b));
      pixels.setPixelColor(9+10*i-j, pixels.Color(r,g,b));
    }

    pixels.show();
    delay(50);
  }
}

// <summary>
// A light mode that uses the funtion getTimeJson to get the time and 
// then uses arrays to display the given numbers with for loops
// </summary>
void clock()
{
  unsigned long currentClockMillis = millis();
  if (currentClockMillis - clockMillis >= clockUpdateInterval) 
  {
      clockMillis = currentClockMillis;
      getTimeJson();
      for (int i = 0; i < 4; ++i)
      {
        printNumber(10,numberOffsets[i],0,0,0);
      }
        printNumber(clockTime[0],numberOffsets[0],255,255,255);
        printNumber(clockTime[1],numberOffsets[1],255,255,255);
        printNumber(clockTime[2],numberOffsets[2],0,255,255);
        printNumber(clockTime[3],numberOffsets[3],0,255,255);
        pixels.show();
  }
}

// <summary>
// Same as the clock() function but has a very low brightness
// </summary>
void nightClock()
{
  unsigned long currentClockMillis = millis();
  if (currentClockMillis - clockMillis >= clockUpdateInterval) 
  {
      clockMillis = currentClockMillis;
      getTimeJson();
      for (int i = 0; i < 4; ++i)
      {
        printNumber(10,numberOffsets[i],0,0,0);
      }
        printNumber(clockTime[0],numberOffsets[0],1,1,1);
        printNumber(clockTime[1],numberOffsets[1],1,1,1);
        printNumber(clockTime[2],numberOffsets[2],0,1,1);
        printNumber(clockTime[3],numberOffsets[3],0,1,1);
        pixels.show();
  }
}

// <summary>
// Used within the clock() funtion.
// Using the arrays of numbers, then displays the given number and in the given position 
// and with the given color
// </summary>
// <param name="number"  type="int" values="0-9">Number to display</param>
// <param name="startingPoint"  type="int" values="0-99">Where to start printing the number, starting from the most down-left pixel</param>
// <param name="r" type="int" values="0-255">The amount of red in the number</param>
// <param name="g" type="int" values="0-255">The amount of green in the number</param>
// <param name="b" type="int" values="0-255">The amount of blue in the number</param>
void printNumber(int number, int startingPoint, int r, int g, int b)
{
  switch (number) {
      case 0:
        for (int i = 0; i < 12; i++)
        {
          pixels.setPixelColor(startingPoint + numberZero[i], pixels.Color(r,g,b));
        }
        // pixels.show();
        break;
      case 1:
        for (int i = 0; i < 5; i++)
        {
          pixels.setPixelColor(startingPoint + numberOne[i], pixels.Color(r,g,b));
        }
        // pixels.show();
        break;
      case 2:
        for (int i = 0; i < 11; i++)
        {
          pixels.setPixelColor(startingPoint + numberTwo[i], pixels.Color(r,g,b));
        }
        // pixels.show();
        break;
      case 3:
        for (int i = 0; i < 11; i++)
        {
          pixels.setPixelColor(startingPoint + numberThree[i], pixels.Color(r,g,b));
        }
        // pixels.show();
        break;
      case 4:
        for (int i = 0; i < 9; i++)
        {
          pixels.setPixelColor(startingPoint + numberFour[i], pixels.Color(r,g,b));
        }
        // pixels.show();
        break;
      case 5:
        for (int i = 0; i < 11; i++)
        {
          pixels.setPixelColor(startingPoint + numberFive[i], pixels.Color(r,g,b));
        }
        // pixels.show();
        break;
      case 6:
        for (int i = 0; i < 12; i++)
        {
          pixels.setPixelColor(startingPoint + numberSix[i], pixels.Color(r,g,b));
        }
        // pixels.show();
        break;
      case 7:
        for (int i = 0; i < 7; i++)
        {
          pixels.setPixelColor(startingPoint + numberSeven[i], pixels.Color(r,g,b));
        }
        // pixels.show();
        break;
      case 8:
        for (int i = 0; i < 13; i++)
        {
          pixels.setPixelColor(startingPoint + numberEight[i], pixels.Color(r,g,b));
        }
        // pixels.show();
        break;
      case 9:
        for (int i = 0; i < 10; i++)
        {
          pixels.setPixelColor(startingPoint + numberNine[i], pixels.Color(r,g,b));
        }
        // pixels.show();
        break;
       case 10:
        for (int i = 0; i < 13; i++)
        {
          pixels.setPixelColor(startingPoint + numberEight[i], pixels.Color(r,g,b));
        }
        for (int i = 0; i < 5; i++)
        {
          pixels.setPixelColor(startingPoint + numberOne[i], pixels.Color(r,g,b));
        }
        break;
      default:
        clearAll();
    }
}

// <summary>
// Sets a random pixel to a random color
// After 100 pixels it clears with a random transition effect with all the colors set to 0
// </summary>
void randomToBlack(){
  skipMode = false;
  sidewaysCubeIn(0,0,0);
  int randomClearEffect = random(5);
  for (int i = 0; i <= 100; i++)
  {
    checkMQTTForUpdate(1);
    if (!skipMode)
    {
      pixels.setPixelColor(random(100), pixels.Color(random(255),random(255),random(255)));
      pixels.show();
      delay(300);
    }
    else
    {
      i = 100;
    }
  }
  switch (random(5)) {
      case 0:
        sidewaysCubeIn(0,0,0);
        break;
      case 1:
        cubeUpLeft(0,0,0);
        break;
      case 2:
        cubeDownRight(0,0,0);
        break;
      case 3:
        cubeUpRight(0,0,0);
        break;
      case 4:
        cubeDownLeft(0,0,0);
        break;
      default:
        clearAll();
    }
}

// <summary>
// Light mode that visually looks like a equalizer/ VU meter
// </summary>
void VU()
{
  for (int i = 0; i < 10; ++i)
  {
    VUOnePillar(i);
  }
  delay(100);
}

// <summary>
// Used within the VU() function
// Renders one pillar with the given height
// </summary>
// <param name="x"  type="intt" values="0-10">The height of the pillar</param>
// <returns></returns>
void VUOnePillar(int x)
{
  for (int i = 0; i <= 10; i++)
  {
    pixels.setPixelColor(i*10+x, pixels.Color(0,0,0));
  }
  for (int i = 0; i <= random(10); i++)
  {
    pixels.setPixelColor(i*10+x, pixels.Color(VUColors[i][0],VUColors[i][1],VUColors[i][2]));
  }
  pixels.show();
}

// <summary>
// Using HTTP GET it get the current time in the right time zone
// It then get a response back in JSON and converts the time from unix to regular time
// Using modul and divide it then gets the correct hour and minute and sets the clockTime array to the real time
// </summary>
void getTimeJson(){
  if (WiFi.status() == WL_CONNECTED) 
  {
    HTTPClient http;  
    http.begin("http://api.timezonedb.com/?zone=Europe/Stockholm&format=json&key=E36C82OUXKJS"); 
    int httpCode = http.GET();                                                          
 
    if (httpCode > 0) 
    { 
      String payload = http.getString();                  
     jsonResult = payload;
    }
    http.end();  
  }

  StaticJsonBuffer<800> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(jsonResult);
  long time = root["timestamp"];
  int hour1 = hour(time)/10;
  int hour2 = hour(time)%10;
  int minute1 = minute(time)/10;
  int minute2 = minute(time)%10;
  clockTime[0] = hour1;
  clockTime[1] = hour2;
  clockTime[2] = minute1;
  clockTime[3] = minute2;
  delay(10);
}

// <summary>
// A simple animation that runs when everything is dont in the setup() function
// </summary>
// <param name="r" type="int" values="0-255">The amount of red in the animation</param>
// <param name="g" type="int" values="0-255">The amount of green in the animation</param>
// <param name="b" type="int" values="0-255">The amount of blue in the animation</param>
void startupAnimation(int r, int g, int b)
{
  for (int i = 0; i < 100; i++)
  {
    pixels.setPixelColor(i, pixels.Color(r,g,b));
  }
  pixels.show();
  for (int i = 99; i >= 0; i--)
  {
    pixels.setPixelColor(i, pixels.Color(0,0,0));
    printPW(r,g,b);
    pixels.show();
    delay(50);
  }
  delay(1000);
  clearAll();
}

void printPW(int r, int g, int b)
{
  for (int i = 0; i < 22; ++i)
  {
    pixels.setPixelColor(pw[i], pixels.Color(r,g,b));
  }
}

void displayLTC(){
  unsigned long currentClockMillis = millis();
  if (currentClockMillis - clockMillis >= clockUpdateInterval) 
  {
      clockMillis = currentClockMillis;
      GetLTCInSek();
      delay(50);
      if (LTCInSEK == 0 || LTCInK == 0)
      {
        lightMode = 7;
      }
      delay(200);
      for (int i = 0; i < 4; ++i)
      {
        printNumber(10,numberOffsets[i],0,0,0);
      }

      if (LTCInSEK > lastLTCprice)
      {
        Serial.println("Green");
        printNumber(LTCInK,numberOffsets[0],0,255,0);
        printNumber(LTCInH,numberOffsets[1],0,255,0);
      }
      else
      {
        Serial.println("Red");
        printNumber(LTCInK,numberOffsets[0],255,0,0);
        printNumber(LTCInH,numberOffsets[1],255,0,0);
      }

      int myLtcK = (LTCInSEK * 3.0385)/1000;
      int myLtcH = ((LTCInSEK * 3.0385) - (myLtcK * 1000))/100;

      if (myLtcK >= 10)
      {
        int myLtcK1 = myLtcK/10;
        int myLtcK2 = myLtcK%10;
        printNumber(myLtcK,numberOffsets[2],100,100,100);
        printNumber(myLtcH,numberOffsets[3],100,100,100);
        pixels.setPixelColor(4, pixels.Color(0,0,0));
      }
      else
      {
        printNumber(myLtcK,numberOffsets[2],100,100,100);
        printNumber(myLtcH,numberOffsets[3],100,100,100);
        pixels.setPixelColor(4, pixels.Color(0,0,100));
      }
      pixels.setPixelColor(54, pixels.Color(0,0,100));
      pixels.show();
  }  
}

void GetLTCInSek(){
  if (WiFi.status() == WL_CONNECTED) 
  {
    HTTPClient http;  
    http.begin("http://192.168.2.247:55107/api/values"); 
    int httpCode = http.GET();   
    Serial.println(httpCode);                                                       

    if (httpCode > 0) 
    { 
      String payload = http.getString();                  
      LTCInK = payload.substring(2,3).toInt();
      LTCInH = payload.substring(3,4).toInt();
      if (payload.substring(2,6).toInt() != lastLTCprice)
      {
        lastLTCprice = LTCInSEK;
        LTCInSEK = payload.substring(2,6).toInt();      
      }  
      Serial.println(payload);
      Serial.println(LTCInSEK);
      Serial.println(LTCInK);
      Serial.println(LTCInH);
    }
    http.end();  
  }
}

void checkMQTTForUpdate(int inMode){
  client.loop();
  if (inMode != lightMode)
  {
    skipMode = true;
  }
}

void setColorMode(int mode)
{
  clearAll();
  lightMode = mode;
}

void setup(void){
  pixels.begin();
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    pixels.setPixelColor(connection, pixels.Color(100,100,100));
    pixels.show();
    connection++;
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  startupAnimation(100,100,100);
}

void loop(void){
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  cycleLightMode();
}
