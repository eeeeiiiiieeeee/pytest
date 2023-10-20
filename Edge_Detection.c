#include "Edge_Detection.h"
#include "motion_multiple.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
// #include "nbsdk.h"

#include <stdint.h>


struct BrightnessFrameInformation information;
struct BoundingRectManger Brightness_Frame;

#define ARRAY_SIZE 32

/*
*十字膨胀
*inputArray iterations：膨胀次数
*/
void cross_dilation(int input_array[32][32], int output_array[32][32], int iterations) {
    int dilation_kernel[3][3] = {{0, 1, 0},
                                  {1, 1, 1},
                                  {0, 1, 0}};
    
    for (int iter = 0; iter < iterations; iter++) {
        int temp_array[32][32];
        for (int i = 1; i < ARRAY_SIZE - 1; i++) {
            for (int j = 1; j < ARRAY_SIZE - 1; j++) {
                if (input_array[i][j] == 1) {
                    for (int dx = -1; dx <= 1; dx++) {
                        for (int dy = -1; dy <= 1; dy++) {
                            temp_array[i + dx][j + dy] = 1;
                        }
                    }
                }
            }
        }
        for (int i = 0; i < ARRAY_SIZE; i++) {
            for (int j = 0; j < ARRAY_SIZE; j++) {
                output_array[i][j] = temp_array[i][j];
            }
        }
    }
}

/*
*十字腐蚀
*inputArray iterations：腐蚀次数
*/
void cross_erosion(int input_array[32][32], int eroded_array[32][32], int iterations) {
    int erosion_kernel[3][3] = {{0, 1, 0},
                                 {1, 1, 1},
                                 {0, 1, 0}};
    
    for (int iter = 0; iter < iterations; iter++) {
        int temp_array[32][32] = {0};  // Initialize the array to zeros
        
        for (int i = 1; i < ARRAY_SIZE - 1; i++) {
            for (int j = 1; j < ARRAY_SIZE - 1; j++) {
                if (input_array[i][j] == 1) {
                    int is_erosion = 1;
                    for (int dx = -1; dx <= 1; dx++) {
                        for (int dy = -1; dy <= 1; dy++) {
                            if (erosion_kernel[dx + 1][dy + 1] == 1 && input_array[i + dx][j + dy] != 1) {
                                is_erosion = 0;
                                break;
                            }
                        }
                        if (!is_erosion) {
                            break;
                        }
                    }
                    temp_array[i][j] = is_erosion ? 1 : 0;
                } else {
                    temp_array[i][j] = input_array[i][j];
                }
            }
        }
        
        for (int i = 0; i < ARRAY_SIZE; i++) {
            for (int j = 0; j < ARRAY_SIZE; j++) {
                eroded_array[i][j] = temp_array[i][j];
            }
        }
    }
}

// 计算九宫格均值的函数
unsigned char apply_neighborhood_average(unsigned char arr[ARRAY_SIZE][ARRAY_SIZE], int x, int y) {
    int neighborhood[9][2] = {
        {x, y}, {x, y - 1}, {x - 1, y}, {x + 1, y}, {x, y + 1},
        {x - 1, y - 1}, {x + 1, y + 1}, {x + 1, y - 1}, {x - 1, y + 1}
    };
    int total = 0;
    int count = 0;
    unsigned char ret=0;

    for (int i = 0; i < 9; i++) {
        int nx = neighborhood[i][0];
        int ny = neighborhood[i][1];
        if (nx >= 0 && nx < ARRAY_SIZE && ny >= 0 && ny < ARRAY_SIZE) {
            total += arr[nx][ny];
            count++;
        }
    }
    
    ret=(unsigned char)(total / count);
    return ret;
}

/*先将算均值*/

/*
*
*亮度值计算边缘的逻辑
*中心点对比均值，大于预设值，对应位置赋值1否则0
*
*/
void process_brightness_array(unsigned char brightness_array[ARRAY_SIZE][ARRAY_SIZE], unsigned char change_array[ARRAY_SIZE][ARRAY_SIZE], int *count_of_ones) {
    for (int x = 0; x < ARRAY_SIZE; x++) {
        for (int y = 0; y < ARRAY_SIZE; y++) {
            unsigned char neighborhood_avg = apply_neighborhood_average(brightness_array, x, y);
            unsigned char change = abs(brightness_array[x][y] - neighborhood_avg);
            if (change > 10) {
                change_array[x][y] = 1;
                (*count_of_ones)++;
            }
            else {
                change_array[x][y] = 0;
            }
        }
    }
}

/*
*亮度阀值处理，将亮度大于230的边界去掉
*
*/
void apply_brightness_threshold(const int brightness_array[ARRAY_SIZE][ARRAY_SIZE], int thresholded_array[ARRAY_SIZE][ARRAY_SIZE]) {
    for (int x = 0; x < ARRAY_SIZE; x++) {
        for (int y = 0; y < ARRAY_SIZE; y++) {
            if (brightness_array[x][y] > 230) {
                int neighborhood[9][2] = {
                    {x, y}, {x, y - 1}, {x - 1, y}, {x + 1, y}, {x, y + 1},
                    {x - 1, y - 1}, {x + 1, y + 1}, {x + 1, y - 1}, {x - 1, y + 1}
                };
                for (int i = 0; i < 9; i++) {
                    int nx = neighborhood[i][0];
                    int ny = neighborhood[i][1];
                    if (nx >= 0 && nx < ARRAY_SIZE && ny >= 0 && ny < ARRAY_SIZE) {
                        thresholded_array[nx][ny] = 1;
                    }
                }
            }
        }
    }
}

/*
*去掉亮度大于230的边界
*
*/
void process_arrays(const int average_array[ARRAY_SIZE][ARRAY_SIZE], const int thresholded_array[ARRAY_SIZE][ARRAY_SIZE], int new_array[ARRAY_SIZE][ARRAY_SIZE]) {
    for (int x = 0; x < ARRAY_SIZE; x++) {
        for (int y = 0; y < ARRAY_SIZE; y++) {
            if (thresholded_array[x][y] == 1 && average_array[x][y] == 1) {
                new_array[x][y] = 0;
            } else {
                new_array[x][y] = thresholded_array[x][y];
            }
        }
    }
}

/*
*消除连续的行和列，消除边框对画框的影响
*/
void remove_continuous_rows_cols(int array[ARRAY_SIZE][ARRAY_SIZE]) {
    int new_array[ARRAY_SIZE][ARRAY_SIZE];
    for (int x = 0; x < ARRAY_SIZE; x++) {
        for (int y = 0; y < ARRAY_SIZE; y++) {
            new_array[x][y] = array[x][y];
            if (array[x][y] == 1) {
                // Check right direction
                int continuous_right = 0;
                for (int i = 1; i < ARRAY_SIZE - y; i++) {
                    if (array[x][y + i] == 1) {
                        continuous_right++;
                    } else {
                        break;
                    }
                }
                // Check down direction
                int continuous_down = 0;
                for (int i = 1; i < ARRAY_SIZE - x; i++) {
                    if (array[x + i][y] == 1) {
                        continuous_down++;
                    } else {
                        break;
                    }
                }
                if (continuous_right > 15) {
                    for (int i = 0; i <= continuous_right; i++) {
                        new_array[x][y + i] = 0;
                    }
                }
                if (continuous_down > 15) {
                    for (int i = 0; i <= continuous_down; i++) {
                        new_array[x + i][y] = 0;
                    }
                }
            }
        }
    }

    // Copy new_array back to array
    for (int x = 0; x < ARRAY_SIZE; x++) {
        for (int y = 0; y < ARRAY_SIZE; y++) {
            array[x][y] = new_array[x][y];
        }
    }
}

/*
*将32x32的二维数组转化为32位的一维数组
*/
void merge_rows_to_1d(int array[ARRAY_SIZE][ARRAY_SIZE], uint32_t merged_array[ARRAY_SIZE]) {
    for (int x = 0; x < ARRAY_SIZE; x++) {
        uint32_t merged_value = 0;
        for (int y = 0; y < ARRAY_SIZE; y++) {
            merged_value = (merged_value << 1) | array[x][y];
        }
        merged_array[x] = merged_value;
    }
}
/*
*oneDimensionalArray:一维数组
*twoDimensionalArray：二维数组
*/
void convertTo2DArray(const unsigned char *oneDimensionalArray, unsigned char twoDimensionalArray[32][32]) {
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            int index = i * 32 + j;
            twoDimensionalArray[i][j] = oneDimensionalArray[index];
        }
    }
}

/*
*打印函数
*/
void print_2d_array(unsigned char arr[32][32], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d ", arr[i][j]);
        }
        printf("\n");
    }
}

/*
*将一个亮度一维数组传进来，得到一组记录图像边缘轮廓的一维数组
*
*/
void process_array(unsigned char input_2d_array[ARRAY_SIZE][ARRAY_SIZE]) {

    unsigned char change_array[ARRAY_SIZE][ARRAY_SIZE];
    int count_of_ones = 0;

    static unsigned int now_time=0;

	static unsigned int  last_time=0;
 
    //convertTo2DArray(input_1d_array, input_2d_array);
    
    process_brightness_array(input_2d_array, change_array, &count_of_ones);
    if (now_time-last_time>=3000)
    {
        print_2d_array(change_array,32,32);
        printf("\n");
        // for (int i = 0; i < 1024; i++)
        // {
        //     if ((i % 32)==0) 
        //     {
        //         printf("\n");
        //     }
        //     printf("%d ",sensor_conf->charBins[i]);
        // }
        last_time=now_time;
    }
    // print_2d_array(change_array,32,32);
    // printf("\n");


    // int thresholded_array[ARRAY_SIZE][ARRAY_SIZE] = {0};  // 初始值设为0的新数组
    // apply_brightness_threshold(input_2d_array,thresholded_array);
    // // print_2d_array(thresholded_array,32,32);
    // // printf("\n");

    // int remove_brightness_boundaries[ARRAY_SIZE][ARRAY_SIZE] = {0};  // 初始值设为0的新数组
    // process_arrays(thresholded_array,change_array,remove_brightness_boundaries);

    // // print_2d_array(remove_brightness_boundaries,32,32);
    // // printf("\n");

    // remove_continuous_rows_cols(remove_brightness_boundaries);

    // int dilated_array[ARRAY_SIZE][ARRAY_SIZE];
    // cross_dilation(remove_brightness_boundaries, dilated_array, 1);

    // remove_continuous_rows_cols(dilated_array);

    // int eroded_array[ARRAY_SIZE][ARRAY_SIZE];
    // cross_erosion(dilated_array, eroded_array, 2);
    // //print_2d_array(eroded_array,32,32);

    // // memset(information.result_1d_array, 0, sizeof(information.result_1d_array));
    // merge_rows_to_1d(eroded_array, information.result_1d_array);
    // now_time = xTaskGetTickCount();
	// 			if (now_time-last_time>=3000)
	// 			{
	// 				for (int i = 0; i < 1024; i++)
	// 				{
	// 					if ((i % 32)==0) 
	// 					{
	// 						printf("\n");
	// 					}
	// 					printf("%d ",input_1d_array[i]);
	// 				}
	// 				last_time=now_time;
	// 			}

}

/*
*二维数组升降排序
*/
void descendingColumnSort(int arr[][4], int size, int column) {
    for (int i = 0; i < size - 1; i++) {
        int maxIndex = i;
        for (int j = i + 1; j < size; j++) {
            if (arr[j][column] > arr[maxIndex][column]) {
                maxIndex = j;
            }
        }
        // Swap arr[i] and arr[maxIndex]
        int temp1 = arr[i][0];
        int temp2 = arr[i][1];
        int temp3 = arr[i][2];
        int temp4 = arr[i][3];
        arr[i][0] = arr[maxIndex][0];
        arr[i][1] = arr[maxIndex][1];
        arr[i][2] = arr[maxIndex][2];
        arr[i][3] = arr[maxIndex][3];
        arr[maxIndex][0] = temp1;
        arr[maxIndex][1] = temp2;
        arr[maxIndex][2] = temp3;
        arr[maxIndex][3] = temp4;
    }
}
/*
*获取画框数量以及画框的坐标点、大小
*/
int GetBrightnessFrame(struct BoundingRectManger *manger,struct BrightnessFrameInformation *IF) {

    struct EegeGroup *edgeGroupCurrent;

    int ret=0;

    for (edgeGroupCurrent = manger->edgeGroup; edgeGroupCurrent != NULL; edgeGroupCurrent = edgeGroupCurrent->last) {
        if (edgeGroupCurrent->belong == NULL) {
            
            if(ret<30){
                IF->bright_f[ret][0]=(uint32_t)(31 - edgeGroupCurrent->left) * 10;//x
                IF->bright_f[ret][1]=(uint32_t)(edgeGroupCurrent->up)*10;//y
                IF->bright_f[ret][2]=(uint32_t)(31 - edgeGroupCurrent->right) * 10 + 10-(uint32_t)(31 - edgeGroupCurrent->left) * 10;//w
                IF->bright_f[ret][3]=(uint32_t)(edgeGroupCurrent->down)*10 + 10-(uint32_t)(edgeGroupCurrent->up)*10;//h
                IF->num++;
                ret++;
            }
            
            /* 在这里得到 */
            //左边界:(uint32_t)(31 - edgeGroupCurrent->left) * 10
            //右边界:(uint32_t)(31 - edgeGroupCurrent->right) * 10 + 10
            //上边界:(uint32_t)(edgeGroupCurrent->up)
            //下边界:(uint32_t)(edgeGroupCurrent->down) + 10
        }
    }
    descendingColumnSort(IF->bright_f, ret, 2);

    printf("%d\n",ret);

    return ret;
}

#if 0
void Brightness_Edge_Computing(void)
{
    int ret=0;

    unsigned char input_2d_array[32][32];

    unsigned char change_array[ARRAY_SIZE][ARRAY_SIZE];

    int count_of_ones = 0;
    static unsigned int now_time=0;

	static unsigned int  last_time=0;
    if (get_global_area()==0) {

        convertTo2DArray(sensor_conf->charBins, input_2d_array);

        process_brightness_array(input_2d_array, change_array, &count_of_ones);

       //process_array(input_2d_array) ;

        now_time = xTaskGetTickCount();
            if (now_time-last_time>=3000)
            {
                print_2d_array(change_array,32,32);
                printf("\n");
                // for (int i = 0; i < 1024; i++)
                // {
                //     if ((i % 32)==0) 
                //     {
                //         printf("\n");
                //     }
                //     printf("%d ",sensor_conf->charBins[i]);
                // }
                last_time=now_time;
            }

        // ret=getBitsMapBoundingRect( &Brightness_Frame, information.result_1d_array, ARRAY_SIZE);
        // if (ret==0) {
        //     GetBrightnessFrame(&Brightness_Frame,&information);
        // }
    }
}

#else
int main (void)
{
    // int arr_1d[1024]={28,0,21,60,48,3,29,66,77,83,84,86,87,89,90,90,92,94,95,102,124,236,254,252,151,105,95,93,93,92,90,87,
    //                     65,12,3,41,60,27,6,58,77,83,86,88,88,89,90,91,93,95,97,101,117,185,224,182,120,102,95,91,89,88,87,86,
    //                     73,58,3,9,59,56,1,34,76,85,88,88,91,91,92,93,94,95,96,100,106,119,132,118,102,96,92,91,88,87,85,84,  
    //                     76,75,43,1,28,63,2,11,68,87,90,91,91,92,92,91,93,95,94,96,98,100,104,101,95,91,89,87,85,84,83,81,    
    //                     79,79,76,26,3,48,2,6,41,86,91,93,93,92,92,92,94,93,93,94,95,96,97,95,91,89,88,86,83,83,80,80,        
    //                     81,80,80,73,12,11,2,29,15,74,93,92,92,93,92,94,92,91,92,92,92,92,93,91,90,87,86,83,82,80,79,78,      
    //                     95,85,82,81,61,4,6,67,20,44,88,92,92,92,92,91,90,90,92,91,91,89,89,88,86,85,83,81,80,78,76,76,       
    //                     116,106,75,69,67,34,3,77,63,16,74,86,86,87,86,85,82,80,77,76,73,70,65,60,55,49,45,40,35,31,27,24,    
    //                     137,134,117,54,49,45,2,81,97,22,32,37,35,33,31,29,27,27,27,28,28,33,38,44,49,53,58,62,67,70,73,76,   
    //                     154,152,153,141,90,75,5,86,115,46,12,42,64,73,82,84,83,82,81,87,102,102,100,101,100,100,98,96,96,95,95,101,
    //                     170,171,172,178,173,127,11,105,123,58,10,50,91,124,173,135,100,91,89,100,110,108,107,107,106,105,103,103,101,102,106,137,
    //                     177,176,179,187,197,190,22,121,117,61,37,39,98,189,254,216,123,100,101,141,164,114,107,101,95,89,84,80,76,75,78,134,
    //                     180,179,189,208,200,201,30,100,95,47,28,23,51,90,133,101,62,52,54,79,94,54,53,53,58,67,81,97,116,136,155,177,
    //                     176,176,177,182,189,190,24,117,116,53,35,82,136,193,235,241,214,184,138,154,162,153,147,144,148,159,173,190,209,224,222,219,
    //                     175,173,174,177,181,177,23,169,176,86,51,120,165,203,234,250,222,196,142,189,171,158,158,159,162,173,188,203,216,223,213,199,
    //                     170,166,167,169,172,162,19,173,186,100,58,133,170,197,217,227,216,198,139,170,167,161,159,165,171,183,199,208,211,210,203,191,
    //                     164,158,158,160,162,147,18,166,184,112,72,152,174,192,204,206,203,192,132,168,169,164,164,168,176,188,201,206,206,203,195,183,
    //                     149,144,144,147,148,131,15,156,176,108,69,150,169,183,194,196,195,186,131,164,169,167,167,170,177,186,193,197,195,191,184,176,
    //                     138,135,141,142,138,116,15,145,167,102,65,141,159,173,182,186,185,179,128,159,169,169,170,171,178,182,185,186,184,179,174,170,
    //                     126,123,129,138,144,113,10,136,153,95,57,131,148,163,173,177,177,173,126,154,169,171,171,172,177,179,180,178,176,173,169,165,
    //                     104,111,115,123,127,100,4,118,141,88,51,120,137,151,164,168,169,165,120,147,167,164,108,124,174,175,175,174,172,169,164,160,
    //                     108,103,106,112,104,88,3,107,130,76,30,87,114,132,147,157,157,151,109,136,161,71,25,56,136,170,170,168,165,161,158,154,
    //                     89,91,95,94,92,82,2,95,118,83,21,70,101,120,135,144,146,137,93,89,118,27,58,61,91,162,158,153,147,147,142,133,
    //                     51,51,54,59,70,70,1,86,108,63,14,49,54,60,75,75,67,73,60,57,96,8,47,43,67,135,114,109,111,109,107,103,
    //                     79,74,71,77,79,63,1,79,101,59,15,57,71,88,105,117,119,117,88,99,107,4,33,21,17,115,133,127,120,115,101,84,
    //                     105,105,86,82,82,64,1,76,96,61,21,68,81,94,104,112,114,112,85,48,130,139,141,133,55,58,113,104,96,88,63,38,
    //                     95,105,77,76,76,63,3,70,91,61,26,70,78,87,95,101,104,104,80,31,155,151,159,164,135,104,107,79,70,63,44,23,
    //                     72,72,73,73,73,61,4,66,86,61,24,66,74,81,88,94,98,98,77,33,114,124,141,153,151,129,95,62,56,48,34,18,
    //                     75,74,75,75,72,59,5,63,83,61,23,63,67,75,81,89,93,94,75,20,33,96,124,129,126,85,57,55,46,35,26,13,
    //                     84,84,86,86,82,63,4,59,81,59,21,61,66,71,79,86,90,93,76,22,25,73,97,141,128,89,46,51,43,29,23,14,
    //                     96,95,95,91,79,52,3,55,76,57,19,56,62,67,76,84,91,95,76,16,22,46,79,36,27,77,27,36,27,36,33,27,
    //                     102,96,80,68,63,48,3,51,72,54,16,48,49,58,73,88,97,102,91,23,16,6,33,55,40,43,3,45,36,34,51,50};

    unsigned char arr_1d[1024]={83,83,85,87,89,94,105,139,236,254,236,141,112,105,102,100,100,99,98,99,99,98,96,97,96,97,97,99,107,128,193,219,
80,82,84,87,90,95,104,134,235,254,247,135,108,103,102,100,100,101,100,100,99,99,98,97,98,98,99,102,110,140,206,217,
80,82,83,86,90,94,100,113,153,198,168,118,106,102,102,102,102,101,101,100,100,100,100,99,100,101,103,113,131,184,202,157,     
80,82,83,85,87,91,95,101,113,125,117,108,103,102,101,101,101,101,101,101,101,101,101,101,101,103,111,156,230,245,153,114,     
78,80,82,85,87,90,92,96,101,105,104,102,101,100,100,101,101,101,101,98,89,95,99,100,101,103,113,202,254,235,129,102,
78,79,81,84,85,88,89,93,96,98,98,98,98,98,99,100,99,100,98,64,36,43,59,86,100,102,110,137,166,135,109,96,
75,78,79,81,85,86,89,92,94,95,95,95,97,98,97,97,97,99,91,73,87,82,82,98,100,100,102,106,109,104,84,67,
41,48,55,61,68,73,81,86,89,91,91,93,93,95,95,95,96,97,98,98,97,96,97,97,97,97,97,98,93,71,75,90,
41,37,31,28,25,24,20,22,30,39,46,53,62,71,78,85,90,94,96,96,97,96,96,94,94,93,93,77,67,85,85,81,
98,85,76,75,75,77,91,91,85,77,72,64,56,49,39,34,34,34,38,43,50,58,65,73,80,82,66,70,81,78,72,71,
231,178,100,84,82,85,104,106,106,105,108,108,109,109,109,110,112,115,114,102,86,75,63,52,33,34,31,29,28,39,68,77,
240,215,123,94,93,105,145,119,114,113,115,115,116,116,116,117,124,150,198,159,124,115,103,87,96,98,91,82,61,22,76,87,
104,89,64,49,48,60,142,81,67,69,74,82,90,99,108,119,139,210,250,192,141,108,100,111,104,97,95,94,82,22,85,99,
241,245,215,184,135,137,146,130,117,109,101,101,101,102,104,105,107,148,126,89,61,55,59,59,62,70,80,87,87,25,93,110,
222,239,217,193,141,161,171,158,152,156,157,167,180,197,216,235,239,244,240,212,176,143,121,111,102,94,84,73,57,19,90,112,   
208,215,209,194,136,166,173,160,158,165,169,179,195,210,227,237,223,210,197,180,166,154,148,146,147,148,145,139,123,33,97,120,
193,198,195,186,126,163,167,164,162,168,176,188,205,215,221,220,210,200,186,174,167,163,160,160,160,160,156,150,137,46,97,129,
182,186,185,179,122,160,167,166,166,170,179,191,204,211,212,209,203,193,183,175,171,170,169,169,168,167,163,157,146,56,92,133,
172,176,177,172,117,158,167,168,170,173,180,189,197,201,202,198,194,187,180,177,173,174,174,173,172,170,166,161,152,69,85,134,
162,168,169,164,113,153,168,170,173,174,181,186,190,192,191,188,185,181,178,178,175,176,176,174,172,171,168,162,154,81,76,132,
150,157,159,154,107,147,167,171,170,173,179,182,186,185,184,181,179,178,178,177,175,176,176,172,166,171,167,162,154,92,67,132,
136,144,147,142,100,142,164,85,16,76,171,179,180,179,179,177,176,175,175,175,174,174,166,53,21,123,165,161,153,98,54,129,
123,131,134,124,86,116,133,7,29,72,160,173,174,174,173,172,171,169,160,171,170,171,154,60,54,62,160,148,138,108,45,126,
85,96,82,87,62,43,77,2,46,67,154,144,134,124,135,136,126,137,139,165,164,167,136,51,48,32,127,137,136,114,37,120,
73,81,81,81,61,101,73,0,37,25,104,147,145,145,144,141,131,130,133,140,153,145,90,8,28,14,132,152,148,120,31,115,
98,104,107,102,70,85,93,73,105,76,18,131,131,124,118,99,63,55,57,84,131,66,105,4,33,20,115,146,144,123,26,109,
87,92,95,95,48,77,160,172,172,172,79,144,105,93,85,67,37,31,38,73,123,48,132,49,95,65,120,147,141,126,25,103,
81,86,90,88,45,87,143,132,153,157,150,150,81,71,63,48,25,23,32,72,116,44,100,133,140,141,135,120,121,125,35,96,
75,81,85,84,45,35,87,113,133,151,113,83,67,61,49,38,21,18,29,81,128,89,114,112,111,103,112,116,123,129,111,141,
71,78,82,81,42,10,57,89,139,126,103,64,64,55,41,32,18,19,28,72,88,98,109,118,128,126,131,137,136,131,117,145,
69,76,81,80,37,9,51,79,86,77,86,38,48,36,47,42,36,41,53,90,84,74,90,91,87,90,104,123,135,133,118,144,
68,77,83,83,34,10,19,36,60,7,70,6,45,47,42,62,64,68,79,98,75,30,51,42,53,78,94,87,79,82,90,110};

    uint32_t arr_2d[32]={0};

    int ret=0;

    unsigned char input_2d_array[32][32];

    unsigned char change_array[ARRAY_SIZE][ARRAY_SIZE];

    int count_of_ones = 0;

    memset(&information, 0, sizeof(information));

    BoundingRectManger_init(&Brightness_Frame);

    convertTo2DArray(arr_1d, input_2d_array);

    process_brightness_array(input_2d_array, change_array, &count_of_ones);

    print_2d_array(change_array,32,32);
    printf("\n");

     //process_array(arr_1d);

    // ret=getBitsMapBoundingRect( &Brightness_Frame, information.result_1d_array, ARRAY_SIZE);
    // if (ret==0) {
    //     GetBrightnessFrame(&Brightness_Frame,&information);
    // }

    // for (int i; i<4; i++) {
    //    printf("w: %d  h: %d \n", information.bright_f[i][2] , information.bright_f[i][3]);     
    // }
    
}

#endif