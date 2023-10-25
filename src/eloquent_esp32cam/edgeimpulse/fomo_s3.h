#ifndef ELOQUENT_ESP32CAM_EDGEIMPULSE_FOMO_S3_H
#define ELOQUENT_ESP32CAM_EDGEIMPULSE_FOMO_S3_H

#if defined(EI_CLASSIFIER_OBJECT_DETECTION)
    #include <edge-impulse-sdk/dsp/image/image.hpp>
    #include "../extra/exception.h"
    #include "../extra/time/benchmark.h"
    #include "../transform/crop.h"
    #include "./bbox.h"

    using Eloquent::Extra::Exception;
    using Eloquent::Extra::Time::Benchmark;
    using namespace eloq;

    namespace Eloquent {
        namespace Esp32cam {
            namespace EdgeImpulse {
                /**
                 * Run Edge Impulse FOMO model
                 */
                class FOMOS3 {
                public:
                    ei::signal_t signal;
                    ei_impulse_result_t result;
                    EI_IMPULSE_ERROR error;
                    Exception exception;
                    Benchmark benchmark;
                    uint8_t input[EI_CLASSIFIER_NN_INPUT_FRAME_SIZE];
                    bool isRGB;
                    bbox_t first;

                    /**
                     *
                     */
                    FOMOS3() :
                        exception("FOMO"),
                        first(0, 0, 0, 0, 0),
                        _isDebugEnabled(false) {
                            isRGB = (EI_CLASSIFIER_NN_INPUT_FRAME_SIZE / EI_CLASSIFIER_RAW_SAMPLE_COUNT) > 1;
                            signal.total_length = EI_CLASSIFIER_RAW_SAMPLE_COUNT;
                            signal.get_data = [this](size_t offset, size_t length, float *out) {
                                if (isRGB) {
                                    for (size_t i = 0, off = offset * 3; i < length; i++, off += 3) {
                                        const uint32_t r = input[off + 0];
                                        const uint32_t g = input[off + 1];
                                        const uint32_t b = input[off + 2];

                                        out[i] = (r << 16) | (g << 8) | b;
                                    }
                                }
                                else {
                                    for (size_t i = 0; i < length; i++) {
                                        const uint32_t gray = input[offset + i];
                                        out[i] = (gray << 16) | (gray << 8) | gray;
                                    }
                                }

                                return 0;
                            };
                    }


                    /**
                     *
                     * @param enabled
                     */
                    void debug(bool enabled = true) {
                        _isDebugEnabled = enabled;
                    }

                    /**
                     *
                     * @return
                     */
                    size_t getDspTime() {
                        return result.timing.dsp;
                    }

                    /**
                     *
                     * @return
                     */
                    size_t getClassificationTime() {
                        return result.timing.classification;
                    }

                    /**
                     *
                     */
                    size_t getTotalTime() {
                        return getDspTime() + getClassificationTime();
                    }

                    /**
                     *
                     * @return
                     */
                    template<typename Frame>
                    Exception& detectObjects(Frame& frame) {
                        if (!frame.length)
                            return exception.set("Cannot run FOMO on empty image");

                        benchmark.start();
                        // crop squash
                        crop
                            .from(frame)
                            .to(EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT)
                            .squash()
                            .rgb(isRGB)
                            .nearest()
                            .apply(input);

                        error = run_classifier(&signal, &result, _isDebugEnabled);
                        benchmark.stop();

                        if (error != EI_IMPULSE_OK)
                            return exception.set(String("Failed to run classifier with error code ") + error);

                        return exception.clear();
                    }

                    /**
                     * Check if objects were found
                     */
                    bool found() {
                        return result.bounding_boxes[0].value != 0;
                    }

                    /**
                     * Check if objects were found
                     */
                    bool foundAnyObject() {
                        return found();
                    }

                    /**
                     * Run function on each bounding box found
                     */
                    template<typename Callback>
                    void forEach(Callback callback) {
                        for (size_t ix = 0, i = 0; ix < result.bounding_boxes_count; ix++) {
                            auto bb = result.bounding_boxes[ix];
                            bbox_t bbox(
                                bb.value,
                                bb.x,
                                bb.y,
                                bb.width,
                                bb.height
                            );

                            if (bbox.value != 0)
                                callback(i++, bbox);
                        }
                    }

                    /**
                     *
                     */
                    ei_impulse_result_bounding_box_t at(size_t ix) {
                        return result.bounding_boxes[ix];
                    }

                    /**
                     * Get count of (non background) bounding boxes
                     */
                    size_t count() {
                        size_t count = 0;

                        for (size_t ix = 0, i = 0; ix < result.bounding_boxes_count; ix++) {
                            auto bb = result.bounding_boxes[ix];

                            if (bb.value != 0)
                                count++;
                        }

                        return count;
                    }

                protected:
                    bool _isDebugEnabled;

                    /**
                     * 
                     */
                    void _crop() {
                        /*const float src_width = camera.rgb565.width;
                        const float src_height = camera.rgb565.height;
                        const size_t limit = (src_width - 1) * (src_height * 1);
                        const float dx = src_width / EI_CLASSIFIER_INPUT_WIDTH;
                        const float dy = src_height / EI_CLASSIFIER_INPUT_HEIGHT;

                        for (size_t y = 0; y < EI_CLASSIFIER_INPUT_HEIGHT; y++) {
                            const size_t dest_y = y * EI_CLASSIFIER_INPUT_WIDTH;
                            const size_t src_y = constrain(dy * y, 0, src_height - 1);
                            const size_t offset_y = src_y * src_width;
                            
                            for (size_t x = 0; x < EI_CLASSIFIER_INPUT_WIDTH; x++) {
                                const size_t pixel_offset = offset_y + constrain(x * dy, 0, src_width - 1);
                                //const uint32_t pixel = camera.rgb565.as888(pixel_offset);
                                const uint32_t pixel = 0; // todo

                                data[dest_y + x] = pixel;
                            }
                        }*/
                    }
                };
            }
        }
    }

    namespace eloq {
        static Eloquent::Esp32cam::EdgeImpulse::FOMOS3 fomo;
    }

#else
#error "EdgeImpulse FOMO library not found"
#endif

#endif //ELOQUENT_ESP32CAM_EDGEIMPULSE_FOMO
