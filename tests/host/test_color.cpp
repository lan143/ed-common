// Host tests for the FastLED-free color module.
//
// Reference values were produced with the real FastLED 3.10.3 library on the
// same host (hsv2rgb.cpp compiled with FASTLED_SCALE8_FIXED=1).

#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "color/color.h"

static int failures = 0;

#define CHECK(cond)                                                            \
    do {                                                                       \
        if (cond) {                                                            \
            std::printf("PASS %s\n", #cond);                                   \
        } else {                                                               \
            std::printf("FAIL %s (line %d)\n", #cond, __LINE__);               \
            ++failures;                                                        \
        }                                                                      \
    } while (0)

static bool rgbIs(const CRGB& c, int r, int g, int b)
{
    return c.r == r && c.g == g && c.b == b;
}

static bool hsvIs(const CHSV& c, int h, int s, int v)
{
    return c.h == h && c.s == s && c.v == v;
}

static void testLayout()
{
    static_assert(sizeof(CRGB) == 3, "CRGB must stay 3 bytes for memcpy persistence");
    static_assert(offsetof(CRGB, r) == 0, "CRGB::r must be at offset 0");
    static_assert(offsetof(CRGB, g) == 1, "CRGB::g must be at offset 1");
    static_assert(offsetof(CRGB, b) == 2, "CRGB::b must be at offset 2");
    static_assert(sizeof(CHSV) == 3, "CHSV must stay 3 bytes");

    CHECK(sizeof(CRGB) == 3);
    CHECK(offsetof(CRGB, r) == 0);
    CHECK(offsetof(CRGB, g) == 1);
    CHECK(offsetof(CRGB, b) == 2);

    CRGB c(1, 2, 3);
    CHECK(c.raw[0] == 1);
    CHECK(c.raw[1] == 2);
    CHECK(c.raw[2] == 3);
}

static void testConstructorsAndPacking()
{
    CRGB zero;
    CHECK(zero == CRGB(0, 0, 0));

    CRGB rgb(0x11, 0x22, 0x33);
    CHECK(rgb.r == 0x11);
    CHECK(rgb.g == 0x22);
    CHECK(rgb.b == 0x33);

    // FastLED layout is 0xRRGGBB.
    CRGB fromCode(0x00112233u);
    CHECK(fromCode.r == 0x11);
    CHECK(fromCode.g == 0x22);
    CHECK(fromCode.b == 0x33);

    CRGB assigned;
    assigned = 0x00AABBCCu;
    CHECK(assigned.r == 0xAA);
    CHECK(assigned.g == 0xBB);
    CHECK(assigned.b == 0xCC);

    CHECK(rgb.as_uint32_t() == 0xff112233u);
    CHECK((fromCode.as_uint32_t() & 0x00FFFFFFu) == 0x00112233u);
    CHECK(CRGB(fromCode.as_uint32_t()) == fromCode);

    CHECK(CRGB::White == CRGB(255, 255, 255));
    CHECK(CRGB::Black == CRGB(0, 0, 0));
    CHECK(CRGB::White != CRGB::Black);
    CHECK(CRGB(1, 2, 3) == CRGB(1, 2, 3));
    CHECK(CRGB(1, 2, 3) != CRGB(3, 2, 1));
}

static void testNscale8Video()
{
    CRGB black(0, 0, 0);
    black.nscale8_video(0);
    CHECK(black == CRGB(0, 0, 0));

    black.nscale8_video(128);
    CHECK(black == CRGB(0, 0, 0));

    CRGB one(1, 1, 1);
    one.nscale8_video(1);
    CHECK(one == CRGB(1, 1, 1));

    CRGB mid(255, 255, 255);
    mid.nscale8_video(128);
    CHECK(mid == CRGB(128, 128, 128));

    CRGB full(255, 255, 255);
    full.nscale8_video(255);
    CHECK(full == CRGB(255, 255, 255));

    CRGB zeroScale(255, 255, 255);
    zeroScale.nscale8_video(0);
    CHECK(zeroScale == CRGB(0, 0, 0));

    CRGB channels(10, 20, 30);
    channels.nscale8_video(128);
    CHECK(channels == CRGB(6, 11, 16));
}

static void testHsvToRgb()
{
    CRGB rgb;

    hsv2rgb_rainbow(CHSV(0, 255, 255), rgb);
    CHECK(rgbIs(rgb, 255, 0, 0));

    // FastLED 3.10.3 rainbow wheel: the pure primaries live at hue 96/160,
    // not at the "standard" HSV-wheel 85/170.
    hsv2rgb_rainbow(CHSV(96, 255, 255), rgb);
    CHECK(rgbIs(rgb, 0, 255, 0));

    hsv2rgb_rainbow(CHSV(160, 255, 255), rgb);
    CHECK(rgbIs(rgb, 0, 0, 255));

    hsv2rgb_rainbow(CHSV(85, 255, 255), rgb);
    CHECK(rgbIs(rgb, 59, 226, 0));

    hsv2rgb_rainbow(CHSV(170, 255, 255), rgb);
    CHECK(rgbIs(rgb, 26, 0, 229));

    hsv2rgb_rainbow(CHSV(0, 0, 255), rgb);
    CHECK(rgbIs(rgb, 255, 255, 255));

    hsv2rgb_rainbow(CHSV(0, 255, 0), rgb);
    CHECK(rgbIs(rgb, 0, 0, 0));

    hsv2rgb_rainbow(CHSV(0, 200, 180), rgb);
    CHECK(rgbIs(rgb, 127, 6, 6));

    hsv2rgb_rainbow(CHSV(85, 200, 180), rgb);
    CHECK(rgbIs(rgb, 34, 113, 6));

    hsv2rgb_rainbow(CHSV(224, 200, 180), rgb);
    CHECK(rgbIs(rgb, 87, 6, 46));

    CRGB returned = hsv2rgb_rainbow(CHSV(0, 255, 255));
    CHECK(rgbIs(returned, 255, 0, 0));
}

static void testRgbToHsv()
{
    CHSV hsv;

    hsv = rgb2hsv_approximate(CRGB(255, 0, 0));
    CHECK(hsvIs(hsv, 1, 255, 255));

    hsv = rgb2hsv_approximate(CRGB(0, 255, 0));
    CHECK(hsvIs(hsv, 96, 255, 255));

    hsv = rgb2hsv_approximate(CRGB(0, 0, 255));
    CHECK(hsvIs(hsv, 161, 255, 255));

    hsv = rgb2hsv_approximate(CRGB(255, 255, 255));
    CHECK(hsvIs(hsv, 0, 0, 255));

    hsv = rgb2hsv_approximate(CRGB(0, 0, 0));
    CHECK(hsvIs(hsv, 0, 0, 0));

    hsv = rgb2hsv_approximate(CRGB(128, 128, 128));
    CHECK(hsvIs(hsv, 0, 0, 181));

    hsv = rgb2hsv_approximate(CRGB(10, 20, 30));
    CHECK(hsvIs(hsv, 145, 205, 108));

    hsv = rgb2hsv_approximate(CRGB(255, 128, 0));
    CHECK(hsvIs(hsv, 112, 255, 255));
}

int main()
{
    testLayout();
    testConstructorsAndPacking();
    testNscale8Video();
    testHsvToRgb();
    testRgbToHsv();

    if (failures != 0) {
        std::printf("FAILED: %d check(s)\n", failures);
        return 1;
    }

    std::printf("ALL COLOR TESTS PASSED\n");
    return 0;
}
