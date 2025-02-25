#ifndef ELOQUENT_ESP32CAM_VIZ_MJPEG
#define ELOQUENT_ESP32CAM_VIZ_MJPEG

#include "../camera/Camera.h"
#include "../extra/exception.h"
#include "../extra/esp32/wifi/sta.h"
#include "../extra/esp32/http/server.h"

using eloq::camera;
using eloq::wifi;
using Eloquent::Error::Exception;
using Eloquent::Extra::Esp32::Http::HttpServer;

namespace Eloquent {
    namespace Esp32cam {
        namespace Viz {
            /**
             * Stream MJPEG video to browser
             */
            class Mjpeg {
                public:
                    Exception exception;
                    HttpServer server;

                    /**
                     * Constructor
                     */
                    Mjpeg() :
                        exception("Mjpeg"),
                        server("Mjpeg", MJPEG_HTTP_PORT),
                        _paused(false),
                        _stopped(false) {

                        }

                    /**
                     * Debug self IP address
                     */
                    String address() const {
                        return String("MJPEG stream is available at http://") + wifi.ip() + ":" + String(MJPEG_HTTP_PORT);
                    }

                    /**
                     * Start server
                     */
                    Exception& begin() {
                        if (!wifi.isConnected())
                            return exception.set("WiFi not connected");

                        onJpeg();
                        onMjpeg();
                        onHtml();

                        // run in thread
                        server.thread.withStackSize(5000);

                        if (!server.begin().isOk())
                            return exception.propagate(server);

                        return exception.clear();
                    }

                    /**
                     * Pause stream
                     */
                    void pause() {
                        _paused = true;
                    }

                    /**
                     * Play stream
                     */
                    void play() {
                        _paused = false;
                        _stopped = false;
                    }

                    /**
                     * Completely stop the stream
                     */
                    void stop() {
                        _stopped = true;
                    }

                protected:
                    bool _paused;
                    bool _stopped;

                    /**
                     * Register / endpoint to get Mjpeg stream
                     */
                    void onMjpeg() {
                        server.onGET("/", [this](WebServer *web) {
                            if (_stopped) {
                                web->send(500, "text/plain", "Server is stopped");
                                return;
                            }

                            WiFiClient client = web->client();

                            client.println(F("HTTP/1.1 200 OK"));
                            client.println(F("Content-Type: multipart/x-mixed-replace;boundary=frame"));
                            client.println(F("Access-Control-Allow-Origin: *"));
                            client.println(F("\r\n--frame"));

                            while (true) {
                                delay(1);
                                yield();

                                if (_paused)
                                    continue;

                                if (_stopped)
                                    break;

                                if (!client.connected())
                                    break;

                                if (!camera.capture().isOk())
                                    continue;

                                client.print("Content-Type: image/jpeg\r\nContent-Length: ");
                                client.println(camera.frame->len);
                                client.println();
                                client.write((const char *) camera.frame->buf, camera.frame->len);
                                client.println(F("\r\n--frame"));
                                client.flush();
                            }
                        });
                    }

                    /**
                     * Register /jpeg endpoint to get a single Jpeg
                     */
                    void onJpeg() {
                        server.onGET("/jpeg", [this](WebServer *web) {
                            if (_stopped) {
                                web->send(500, "text/plain", "Server is stopped");
                                return;
                            }

                            if (!camera.capture().isOk())
                                return server.serverError(camera.exception.toString());

                            WiFiClient client = web->client();

                            client.println(F("HTTP/1.1 200 OK"));
                            client.println(F("Content-Type: image/jpeg"));
                            client.println(F("Access-Control-Allow-Origin: *"));
                            client.print(F("Content-Length: "));
                            client.println((unsigned int) camera.frame->len);
                            client.println();
                            client.write((const char *) camera.frame->buf, camera.frame->len);
                        });
                    }

                    /**
                     * Register /html endpoint to get a full HTML page
                     */
                    void onHtml() {
                        server.onGET("/html", [this](WebServer *web) {
                            if (_stopped) {
                                web->send(500, "text/plain", "Server is stopped");
                                return;
                            }
                            
                            String html = String("<img src=\"http://") + wifi.ip() + ":" + MJPEG_HTTP_PORT + "\" />";
                            web->send(200, "text/html", html);
                        });
                    }
            };
        }
    }
}

namespace eloq {
    namespace viz {
        static Eloquent::Esp32cam::Viz::Mjpeg mjpeg;
    }
}

#endif