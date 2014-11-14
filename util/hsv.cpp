#include "hsv.h"

#define MIN(a,b)      ((a) < (b) ? (a) : (b))
#define MAX(a,b)      ((a) > (b) ? (a) : (b))

#define MIN3(a,b,c)   MIN((a), MIN((b), (c)))
#define MAX3(a,b,c)   MAX((a), MAX((b), (c)))

void rgb2hsv(COLORREF rgb, hsv_t* hsv)
{
    int r = GetRValue(rgb);
    int g = GetGValue(rgb);
    int b = GetBValue(rgb);
    int m = MIN3(r, g, b);
    int M = MAX3(r, g, b);
    int delta = M - m;

    if(delta == 0) {
        /* Achromatic case (i.e. grayscale) */
        hsv->hue = -1;          /* undefined */
        hsv->saturation = 0;
    } else {
        int h;

        if(r == M)
            h = ((g-b)*60*HUE_DEGREE) / delta;
        else if(g == M)
            h = ((b-r)*60*HUE_DEGREE) / delta + 120*HUE_DEGREE;
        else /*if(b == M)*/
            h = ((r-g)*60*HUE_DEGREE) / delta + 240*HUE_DEGREE;

        if(h < 0)
            h += 360*HUE_DEGREE;

        hsv->hue = h;

        /* The constatnt 8 is tuned to statistically cause as little
         * tolerated mismatches as possible in RGB -> HSV -> RGB conversion.
         * (See the unit test at the bottom of this file.)
         */
        hsv->saturation = (256*delta-8) / M;
    }
    hsv->value = M;
}

COLORREF hsv2rgb(hsv_t* hsv)
{
    BYTE r, g, b;

    if(hsv->saturation == 0) {
        r = g = b = hsv->value;
    } else {
        int h = hsv->hue;
        int s = hsv->saturation;
        int v = hsv->value;
        int i = h / (60*HUE_DEGREE);
        int p = (256*v - s*v) / 256;

        if(i & 1) {
            int q = (256*60*HUE_DEGREE*v - h*s*v + 60*HUE_DEGREE*s*v*i) / (256*60*HUE_DEGREE);
            switch(i) {
                case 1:   r = q; g = v; b = p; break;
                case 3:   r = p; g = q; b = v; break;
                case 5:   r = v; g = p; b = q; break;
            }
        } else {
            int t = (256*60*HUE_DEGREE*v + h*s*v - 60*HUE_DEGREE*s*v*(i+1)) / (256*60*HUE_DEGREE);
            switch(i) {
                case 0:   r = v; g = t; b = p; break;
                case 2:   r = p; g = v; b = t; break;
                case 4:   r = t; g = p; b = v; break;
            }
        }
    }

    return RGB(r, g, b);
}
