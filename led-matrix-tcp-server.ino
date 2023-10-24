#include <WiFi.h>
#include <FastLED.h>

#define LED_PIN 12
// 13 columns * 13 rows
#define NUM_LEDS 169
#define BRIGHTNESS 64
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];
uint8_t *leds_int = (uint8_t *)leds;

// Set tcp server port number to 8888
WiFiServer server(8888);

// network credentials
const char *ssid = "xxxx";
const char *password = "xxxx";
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;
unsigned long currentTime = millis();
unsigned long previousTime = 0;

int connectingDotsIdx = 0;
String helper_string = "";

void display_ip_digit(uint8_t x, uint8_t col)
{
    uint8_t ones = x % 10;
    uint8_t tens = (x / 10) % 10;
    uint8_t huns = (x / 100) % 10;
    // Serial.println(helper_string + "address single huns:" + huns + " tens:" + tens + " ones:" + ones);

    for (int row = 0; row < huns; row++)
    {
        uint8_t _col = row % 2 == 0 ? col : (12 - col);
        uint8_t idx = row * 13 + _col;
        leds[idx] = CRGB::Red;
    }
    for (int row = 0; row < tens; row++)
    {
        uint8_t _col = row % 2 == 0 ? (col - 1) : (12 - col + 1);
        uint8_t idx = row * 13 + _col;
        leds[idx] = CRGB::Green;
    }
    for (int row = 0; row < ones; row++)
    {
        uint8_t _col = row % 2 == 0 ? (col - 2) : (12 - col + 2);
        uint8_t idx = row * 13 + _col;
        leds[idx] = CRGB::Blue;
    }
}

void display_ip(IPAddress ip)
{
    // Serial.println(helper_string + "address a:" + ip[0]);
    // Serial.println(helper_string + "address b:" + ip[1]);
    // Serial.println(helper_string + "address c:" + ip[2]);
    // Serial.println(helper_string + "address d:" + ip[3]);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    display_ip_digit(ip[0], 12);
    display_ip_digit(ip[1], 9);
    display_ip_digit(ip[2], 6);
    display_ip_digit(ip[3], 3);
    FastLED.show();
}

void setup()
{
    Serial.begin(115200);

    Serial.print("Setting up FastLed");

    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);

    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        leds[connectingDotsIdx] = CRGB::Red;
        connectingDotsIdx++;
        if (connectingDotsIdx >= NUM_LEDS)
        {
            connectingDotsIdx = 0;
            fill_solid(leds, NUM_LEDS, CRGB::Black);
        }
        FastLED.show();
    }

    Serial.println("\nWiFi connected. IP address is:");
    IPAddress ip = WiFi.localIP();
    display_ip(ip);
    Serial.println(ip);

    server.begin();
}

void loop()
{
    WiFiClient client = server.available(); // Listen for incoming clients
    if (!client)
    {
        return;
    }
    // a new client connected
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client."); // print a message out in the serial port
    // String currentLine = "";       // make a String to hold incoming data from the client
    String currentLine = "";
    bool new_frame = false;
    while (client.connected() && currentTime - previousTime <= timeoutTime)
    { // loop while the client's connected
        currentTime = millis();
        if (!client.available())
        {
            continue;
        }
        previousTime = currentTime;
        if (!new_frame)
        {
            // there are bytes to read from the client
            char c = client.read(); // read a byte, then
            // Serial.write(c);        // print it out the serial monitor
            // Serial.println(helper_string + "got another char: " + c + " and the int " + (uint8_t)c);
            if (c != '\n')
            {
                if (c != 0 && c != '\r')
                {
                    // if you got anything else but a carriage return character,
                    currentLine += c; // add it to the end of the currentLine
                }
                continue;
            }
            // Serial.println("checking for a new frame");
            // the byte is a newline character
            // if (currentLine != "frame1234")
            if (!currentLine.endsWith("frame1234"))
            {
                Serial.println(helper_string + "expected: 'frame1234' actual: " + currentLine);
                currentLine = "";
                continue;
            }
            // Serial.println("start of a new frame");
            new_frame = true;
            continue;
        }
        // Serial.println("reading the new frame");
        int n_bytes = client.read(leds_int, NUM_LEDS * 3);
        if (n_bytes != NUM_LEDS * 3)
        {
            Serial.println(helper_string + "expected a new frame. n_bytes: " + n_bytes);
            currentLine = "";
            new_frame = false;
            continue;
        }
        new_frame = false;
        // Serial.println("display the new frame");
        FastLED.show();
    }
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.\n");
}
