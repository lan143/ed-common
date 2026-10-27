#pragma once

#ifndef EDCOMMON_COLOR_COLOR_H
#define EDCOMMON_COLOR_COLOR_H

/*
 * FastLED-free color module.
 *
 * When FastLED is available it is included unchanged and this header defines
 * nothing else, so projects that still use the real FastLED keep its exact
 * CRGB/CHSV semantics. Otherwise a small, dependency-free port is provided.
 *
 * Ported from FastLED 3.10.3:
 *   src/hsv2rgb.cpp        (hsv2rgb_rainbow, rgb2hsv_approximate)
 *   src/lib8tion/scale8.h  (scale8, scale8_video, *_LEAVING_R1_DIRTY)
 *   src/lib8tion/math8.h   (qadd8, qsub8, sqrt16)
 *   src/lib8tion/trig8.h   (sin8 and its lookup table)
 *   src/fl/hsv.h           (HUE_* constants)
 *
 * The numeric helpers are reproduced with the default FASTLED_SCALE8_FIXED=1
 * behaviour so the two conversion routines are bit-for-bit compatible with
 * FastLED 3.10.3 on a non-AVR build.
 */

#if defined(__has_include)
#  if __has_include(<FastLED.h>)
#    include <FastLED.h>
#    define EDCOMMON_COLOR_HAS_FASTLED 1
#  endif
#endif

#ifndef EDCOMMON_COLOR_HAS_FASTLED

#include <stdint.h>

namespace EDColorDetail
{
    typedef uint8_t fract8;

    /// Scale one byte by a second one, which is treated as the numerator of a
    /// fraction whose denominator is 256. FASTLED_SCALE8_FIXED=1 variant.
    inline uint8_t scale8(uint8_t i, fract8 scale)
    {
        return (((uint16_t)i) * (1 + (uint16_t)(scale))) >> 8;
    }

    /// The "video" version of scale8(): non-zero inputs never become zero.
    inline uint8_t scale8_video(uint8_t i, fract8 scale)
    {
        uint8_t j = (((int)i * (int)scale) >> 8) + ((i && scale) ? 1 : 0);
        return j;
    }

    /// FASTLED_SCALE8_FIXED=1 variant of scale8_LEAVING_R1_DIRTY.
    inline uint8_t scale8_LEAVING_R1_DIRTY(uint8_t i, fract8 scale)
    {
        return (((uint16_t)i) * ((uint16_t)(scale) + 1)) >> 8;
    }

    /// FASTLED_SCALE8_FIXED=1 variant of scale8_video_LEAVING_R1_DIRTY.
    inline uint8_t scale8_video_LEAVING_R1_DIRTY(uint8_t i, fract8 scale)
    {
        uint8_t j = (((int)i * (int)scale) >> 8) + ((i && scale) ? 1 : 0);
        return j;
    }

    /// No-op on non-AVR targets; kept for structural parity with FastLED.
    inline void cleanup_R1() {}

    /// Add one byte to another, saturating at 0xFF.
    inline uint8_t qadd8(uint8_t i, uint8_t j)
    {
        unsigned int t = i + j;
        if (t > 255) {
            t = 255;
        }
        return static_cast<uint8_t>(t);
    }

    /// Subtract one byte from another, saturating at 0x00.
    inline uint8_t qsub8(uint8_t i, uint8_t j)
    {
        int t = i - j;
        if (t < 0) {
            t = 0;
        }
        return static_cast<uint8_t>(t);
    }

    /// Square root for 16-bit integers (FastLED's small integer approximation).
    inline uint8_t sqrt16(uint16_t x)
    {
        if (x <= 1) {
            return x;
        }

        uint8_t low = 1; // lower bound
        uint8_t hi, mid;

        if (x > 7904) {
            hi = 255;
        } else {
            hi = (x >> 5) + 8; // initial estimate for upper bound
        }

        do {
            mid = (low + hi) >> 1;
            if ((uint16_t)(mid * mid) > x) {
                hi = mid - 1;
            } else {
                if (mid == 255) {
                    return 255;
                }
                low = mid + 1;
            }
        } while (hi >= low);

        return low - 1;
    }

    /// Fast 8-bit approximation of sin(x); the exact FastLED lookup table.
    inline uint8_t sin8(uint8_t theta)
    {
        static const uint8_t b_m16_interleave[] = {0, 49, 49, 41, 90, 27, 117, 10};

        uint8_t offset = theta;
        if (theta & 0x40) {
            offset = (uint8_t)255 - offset;
        }
        offset &= 0x3F; // 0..63

        uint8_t secoffset = offset & 0x0F; // 0..15
        if (theta & 0x40) {
            ++secoffset;
        }

        uint8_t section = offset >> 4; // 0..3
        uint8_t s2 = section * 2;
        const uint8_t* p = b_m16_interleave;
        p += s2;
        uint8_t b = *p;
        ++p;
        uint8_t m16 = *p;

        uint8_t mx = (m16 * secoffset) >> 4;

        int8_t y = mx + b;
        if (theta & 0x80) {
            y = -y;
        }

        y += 128;

        return y;
    }

    enum
    {
        HUE_RED = 0,
        HUE_ORANGE = 32,
        HUE_YELLOW = 64,
        HUE_GREEN = 96,
        HUE_AQUA = 128,
        HUE_BLUE = 160,
        HUE_PURPLE = 192,
        HUE_PINK = 224
    };
}

struct CRGB;

namespace EDColorDetail
{
    /*
     * C++11 has no inline variables and an out-of-class definition of a static
     * data member in a header would produce duplicate symbols. Placing the
     * constants in a class template keeps them header-only while preserving
     * the CRGB::White / CRGB::Black API.
     */
    template <typename Tag>
    struct CRGBStatics
    {
        static const CRGB White;
        static const CRGB Black;
    };
}

struct CRGB : EDColorDetail::CRGBStatics<void>
{
    union
    {
        struct
        {
            uint8_t r, g, b;
        };
        uint8_t raw[3];
    };

    CRGB() : r(0), g(0), b(0) {}

    CRGB(uint8_t ir, uint8_t ig, uint8_t ib) : r(ir), g(ig), b(ib) {}

    /// FastLED layout: 0xRRGGBB (the top byte is ignored).
    CRGB(uint32_t colorcode)
        : r((colorcode >> 16) & 0xFF),
          g((colorcode >> 8) & 0xFF),
          b((colorcode >> 0) & 0xFF)
    {
    }

    CRGB& operator=(uint32_t colorcode)
    {
        r = (colorcode >> 16) & 0xFF;
        g = (colorcode >> 8) & 0xFF;
        b = (colorcode >> 0) & 0xFF;
        return *this;
    }

    bool operator==(const CRGB& rhs) const
    {
        return r == rhs.r && g == rhs.g && b == rhs.b;
    }

    bool operator!=(const CRGB& rhs) const { return !(*this == rhs); }

    /// FastLED layout: 0xFFRRGGBB.
    uint32_t as_uint32_t() const
    {
        return ((uint32_t)0xff000000)
            | ((uint32_t)r << 16)
            | ((uint32_t)g << 8)
            | ((uint32_t)b);
    }

    CRGB& nscale8_video(uint8_t scaledown)
    {
        r = EDColorDetail::scale8_video_LEAVING_R1_DIRTY(r, scaledown);
        g = EDColorDetail::scale8_video_LEAVING_R1_DIRTY(g, scaledown);
        b = EDColorDetail::scale8_video_LEAVING_R1_DIRTY(b, scaledown);
        EDColorDetail::cleanup_R1();
        return *this;
    }
};

namespace EDColorDetail
{
    template <typename Tag>
    const CRGB CRGBStatics<Tag>::White(255, 255, 255);

    template <typename Tag>
    const CRGB CRGBStatics<Tag>::Black(0, 0, 0);
}

struct CHSV
{
    uint8_t h, s, v;

    CHSV() : h(0), s(0), v(0) {}

    CHSV(uint8_t ih, uint8_t is, uint8_t iv) : h(ih), s(is), v(iv) {}
};

inline void hsv2rgb_rainbow(const CHSV& hsv, CRGB& rgb)
{
    using EDColorDetail::cleanup_R1;
    using EDColorDetail::scale8;
    using EDColorDetail::scale8_LEAVING_R1_DIRTY;
    using EDColorDetail::scale8_video;
    using EDColorDetail::scale8_video_LEAVING_R1_DIRTY;

    // Yellow has a higher inherent brightness than any other color; 'pure'
    // yellow is perceived to be 93% as bright as white. In order to make
    // yellow appear the correct relative brightness, it has to be rendered
    // brighter than all other colors.
    const uint8_t Y1 = 1;
    const uint8_t Y2 = 0;

    // G2: Whether to divide all greens by two.
    const uint8_t G2 = 0;

    // Gscale: what to scale green down by.
    const uint8_t Gscale = 0;

    uint8_t hue = hsv.h;
    uint8_t sat = hsv.s;
    uint8_t val = hsv.v;

    uint8_t offset = hue & 0x1F; // 0..31

    // offset8 = offset * 8
    uint8_t offset8 = offset;
    offset8 <<= 3;

    uint8_t third = scale8(offset8, (256 / 3)); // max = 85

    uint8_t r, g, b;

    if (!(hue & 0x80)) {
        // 0XX
        if (!(hue & 0x40)) {
            // 00X
            // section 0-1
            if (!(hue & 0x20)) {
                // 000
                // case 0: // R -> O
                r = 255 - third;
                g = third;
                b = 0;
            } else {
                // 001
                // case 1: // O -> Y
                if (Y1) {
                    r = 171;
                    g = 85 + third;
                    b = 0;
                }
                if (Y2) {
                    r = 170 + third;
                    uint8_t twothirds = scale8(offset8, ((256 * 2) / 3)); // max=170
                    g = 85 + twothirds;
                    b = 0;
                }
            }
        } else {
            // 01X
            // section 2-3
            if (!(hue & 0x20)) {
                // 010
                // case 2: // Y -> G
                if (Y1) {
                    uint8_t twothirds = scale8(offset8, ((256 * 2) / 3)); // max=170
                    r = 171 - twothirds;
                    g = 170 + third;
                    b = 0;
                }
                if (Y2) {
                    r = 255 - offset8;
                    g = 255;
                    b = 0;
                }
            } else {
                // 011
                // case 3: // G -> A
                r = 0;
                g = 255 - third;
                b = third;
            }
        }
    } else {
        // section 4-7
        // 1XX
        if (!(hue & 0x40)) {
            // 10X
            if (!(hue & 0x20)) {
                // 100
                // case 4: // A -> B
                r = 0;
                uint8_t twothirds = scale8(offset8, ((256 * 2) / 3)); // max=170
                g = 171 - twothirds; // K170?
                b = 85 + twothirds;
            } else {
                // 101
                // case 5: // B -> P
                r = third;
                g = 0;
                b = 255 - third;
            }
        } else {
            if (!(hue & 0x20)) {
                // 110
                // case 6: // P -- K
                r = 85 + third;
                g = 0;
                b = 171 - third;
            } else {
                // 111
                // case 7: // K -> R
                r = 170 + third;
                g = 0;
                b = 85 - third;
            }
        }
    }

    // This is one of the good places to scale the green down,
    // although the client can scale green down as well.
    if (G2) {
        g = g >> 1;
    }
    if (Gscale) {
        g = scale8_video_LEAVING_R1_DIRTY(g, Gscale);
    }

    // Scale down colors if we're desaturated at all
    // and add the brightness_floor to r, g, and b.
    if (sat != 255) {
        if (sat == 0) {
            r = 255;
            b = 255;
            g = 255;
        } else {
            uint8_t desat = 255 - sat;
            desat = scale8_video(desat, desat);

            uint8_t satscale = 255 - desat;
            // satscale = sat; // uncomment to revert to pre-2021 saturation behavior

            r = scale8_LEAVING_R1_DIRTY(r, satscale);
            g = scale8_LEAVING_R1_DIRTY(g, satscale);
            b = scale8_LEAVING_R1_DIRTY(b, satscale);
            cleanup_R1();

            uint8_t brightness_floor = desat;
            r += brightness_floor;
            g += brightness_floor;
            b += brightness_floor;
        }
    }

    // Now scale everything down if we're at value < 255.
    if (val != 255) {
        val = scale8_video_LEAVING_R1_DIRTY(val, val);
        if (val == 0) {
            r = 0;
            g = 0;
            b = 0;
        } else {
            r = scale8_LEAVING_R1_DIRTY(r, val);
            g = scale8_LEAVING_R1_DIRTY(g, val);
            b = scale8_LEAVING_R1_DIRTY(b, val);
            cleanup_R1();
        }
    }

    rgb.r = r;
    rgb.g = g;
    rgb.b = b;
}

inline CRGB hsv2rgb_rainbow(const CHSV& hsv)
{
    CRGB rgb;
    hsv2rgb_rainbow(hsv, rgb);
    return rgb;
}

inline CHSV rgb2hsv_approximate(const CRGB& rgb)
{
    using EDColorDetail::qadd8;
    using EDColorDetail::qsub8;
    using EDColorDetail::scale8;
    using EDColorDetail::sqrt16;

#define EDCOLOR_FIXFRAC8(N, D) (((N) * 256) / (D))

    uint8_t r = rgb.r;
    uint8_t g = rgb.g;
    uint8_t b = rgb.b;
    uint8_t h, s, v;

    // find desaturation
    uint8_t desat = 255;
    if (r < desat) desat = r;
    if (g < desat) desat = g;
    if (b < desat) desat = b;

    // remove saturation from all channels
    r -= desat;
    g -= desat;
    b -= desat;

    // saturation is opposite of desaturation
    s = 255 - desat;

    if (s != 255) {
        // undo 'dimming' of saturation
        s = 255 - sqrt16((255 - s) * 256);
    }

    // at least one channel is now zero
    // if all three channels are zero, we had a shade of gray.
    if ((r + g + b) == 0) {
        // we pick hue zero for no special reason
        return CHSV(0, 0, 255 - s);
    }

    // scale all channels up to compensate for desaturation
    if (s < 255) {
        if (s == 0) s = 1;
        uint32_t scaleup = 65535 / (s);
        r = ((uint32_t)(r) * scaleup) / 256;
        g = ((uint32_t)(g) * scaleup) / 256;
        b = ((uint32_t)(b) * scaleup) / 256;
    }

    uint16_t total = r + g + b;

    // scale all channels up to compensate for low values
    if (total < 255) {
        if (total == 0) total = 1;
        uint32_t scaleup = 65535 / (total);
        r = ((uint32_t)(r) * scaleup) / 256;
        g = ((uint32_t)(g) * scaleup) / 256;
        b = ((uint32_t)(b) * scaleup) / 256;
    }

    if (total > 255) {
        v = 255;
    } else {
        v = qadd8(desat, total);
        // undo 'dimming' of brightness
        if (v != 255) v = sqrt16(v * 256);
    }

    // since this wasn't a pure shade of gray,
    // the interesting question is what hue is it

    // start with which channel is highest
    // (ties don't matter)
    uint8_t highest = r;
    if (g > highest) highest = g;
    if (b > highest) highest = b;

    if (highest == r) {
        // Red is highest.
        // Hue could be Purple/Pink-Red, Red-Orange, Orange-Yellow
        if (g == 0) {
            // if green is zero, we're in Purple/Pink-Red
            h = (EDColorDetail::HUE_PURPLE + EDColorDetail::HUE_PINK) / 2;
            h += scale8(qsub8(r, 128), EDCOLOR_FIXFRAC8(48, 128));
        } else if ((r - g) > g) {
            // if R-G > G then we're in Red-Orange
            h = EDColorDetail::HUE_RED;
            h += scale8(g, EDCOLOR_FIXFRAC8(32, 85));
        } else {
            // R-G < G, we're in Orange-Yellow
            h = EDColorDetail::HUE_ORANGE;
            h += scale8(qsub8((g - 85) + (171 - r), 4), EDCOLOR_FIXFRAC8(32, 85)); // 221
        }
    } else if (highest == g) {
        // Green is highest
        // Hue could be Yellow-Green, Green-Aqua
        if (b == 0) {
            // if Blue is zero, we're in Yellow-Green
            //   G = 171..255
            //   R = 171..  0
            h = EDColorDetail::HUE_YELLOW;
            uint8_t radj = scale8(qsub8(171, r), 47); // 171..0 -> 0..171 -> 0..31
            uint8_t gadj = scale8(qsub8(g, 171), 96); // 171..255 -> 0..84 -> 0..31
            uint8_t rgadj = radj + gadj;
            uint8_t hueadv = rgadj / 2;
            h += hueadv;
        } else {
            // if Blue is nonzero we're in Green-Aqua
            if ((g - b) > b) {
                h = EDColorDetail::HUE_GREEN;
                h += scale8(b, EDCOLOR_FIXFRAC8(32, 85));
            } else {
                h = EDColorDetail::HUE_AQUA;
                h += scale8(qsub8(b, 85), EDCOLOR_FIXFRAC8(8, 42));
            }
        }
    } else /* highest == b */ {
        // Blue is highest
        // Hue could be Aqua/Blue-Blue, Blue-Purple, Purple-Pink
        if (r == 0) {
            // if red is zero, we're in Aqua/Blue-Blue
            h = EDColorDetail::HUE_AQUA + ((EDColorDetail::HUE_BLUE - EDColorDetail::HUE_AQUA) / 4);
            h += scale8(qsub8(b, 128), EDCOLOR_FIXFRAC8(24, 128));
        } else if ((b - r) > r) {
            // B-R > R, we're in Blue-Purple
            h = EDColorDetail::HUE_BLUE;
            h += scale8(r, EDCOLOR_FIXFRAC8(32, 85));
        } else {
            // B-R < R, we're in Purple-Pink
            h = EDColorDetail::HUE_PURPLE;
            h += scale8(qsub8(r, 85), EDCOLOR_FIXFRAC8(32, 85));
        }
    }

    h += 1;
    return CHSV(h, s, v);

#undef EDCOLOR_FIXFRAC8
}

#endif // EDCOMMON_COLOR_HAS_FASTLED

#ifdef EDCOMMON_COLOR_HAS_FASTLED
#  undef EDCOMMON_COLOR_HAS_FASTLED
#endif

#endif // EDCOMMON_COLOR_COLOR_H
