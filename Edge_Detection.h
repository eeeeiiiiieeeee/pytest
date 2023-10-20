#ifndef __EDGE_DETACTION__
#define __EDGE_DETACTION__

#include <stdint.h>


extern struct BoundingRectManger Brightness_Frame;

struct BrightnessFrameInformation {

    uint32_t result_1d_array[32];
    int num;
    int bright_f[30][4];
};

#define ARRAY_SIZE 32

void process_array(unsigned char input_2d_array[ARRAY_SIZE][ARRAY_SIZE]);
void Brightness_Edge_Computing(void);


#endif