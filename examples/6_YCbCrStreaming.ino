#include "esp32cam.h"
#include "esp32cam/YCbCrStreaming.h"

// Replace with your WiFi credentials
#define WIFI_SSID "Novanetworks_Spagro2017"
#define WIFI_PASS "AGRIMAG01"

// 80 is the port to listen to
// You can change it to whatever you want, 80 is the default for HTTP
Eloquent::Esp32cam::Cam cam;
Eloquent::Esp32cam::YCbCrStreaming streaming(cam, 80);


void setup() {
    Serial.begin(115200);
    delay(3000);
    Serial.println("Init");

    // see 3_Get_Your_First_Picture for more details
    cam.m5wide();
    cam.highQuality();
    cam.qvga();

    while (!cam.begin())
        Serial.println(cam.getErrorMessage());

    cam.vflip();

    // Connect to WiFi
    // If something goes wrong, print the error message
    while (!cam.connect(WIFI_SSID, WIFI_PASS))
        Serial.println("Cannot connect to WiFi");

    //Initialize stream web server
    // If something goes wrong, print the error message
    while (!streaming.begin())
        Serial.println(streaming.getErrorMessage());

    // make the camera accessible at http://esp32cam.local
    //cam.viewAt("esp32cam");

    // as a fallback, display the IP address of the camera
    Serial.println(streaming.getWelcomeMessage());
}


void loop() {

}
