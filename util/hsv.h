#ifndef HSV_H
#define HSV_H

#include <windows.h>

#define HUE_DEGREE    512

typedef struct hsv_tag hsv_t;
struct hsv_tag {
    int hue;               /* 0 ... (360*HUE_DEGREE - 1) */
    BYTE saturation;       /* 0 ... 255 */
    BYTE value;            /* 0 ... 255 */
};

void rgb2hsv(COLORREF rgb, hsv_t* hsv);

COLORREF hsv2rgb(hsv_t* hsv);

#endif  /* HSV_H */
