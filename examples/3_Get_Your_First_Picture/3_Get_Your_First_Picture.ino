/**
 * Example 3: Get your First picture
 * This sketch shows the basic code you need to take a picture
 * from your Esp32 camera
 *
 * BE SURE TO SET "TOOLS > CORE DEBUG LEVEL = DEBUG"
 * to turn on debug messages
 */
#include "esp32camera.h"


/**
 *
 */
void setup() {
    Serial.begin(115200);
    delay(3000);
    Serial.println("Init");

    /**
     * Configure camera model
     * Allowed values are:
     *
     *  - aithinker
     *  - m5
     *  - m5wide
     *  - eye
     *  - wrover
     *  - ttgoLCD
     *  - simcam
     *
     *  Default is aithinker
     */
    camera.model.aithinker();

    /**
     * Configure camera resolution
     * Allowed values are:
     *
     * - _96x96
     * - qqvga
     * - qcif
     * - hqvga
     * - _240x240
     * - qvga
     * - cif
     * - hvga
     * - vga
     * - svga
     * - xga
     * - hd
     * - sxga
     * - uxga
     *
     * Default is vga
     */
    camera.resolution.vga();

    /**
     * Configure JPEG quality
     * Allowed values are:
     *
     *  - low (30)
     *  - high (20)
     *  - best (10)
     *  - set(quality), ranging from 10 (best) to 64 (lowest)
     *
     *  Default is high
     */
    camera.quality.high();

    /**
     * Initialize the camera
     * If something goes wrong, print the error message
     */
    while (!camera.begin())
        Serial.println(camera.getErrorMessage());

    Serial.println("Camera OK");
}

/**
 *
 */
void loop() {
    /**
     * Try to capture a frame
     * If something goes wrong, print the error message
     */
    if (!camera.capture()) {
        Serial.println(camera.getErrorMessage());
        return;
    }

    Serial.print("Jpeg size: ");
    Serial.println(camera.getSizeInBytes());

    /**
     * Do whatever you want with the captured frame.
     * See next examples.
     */
}