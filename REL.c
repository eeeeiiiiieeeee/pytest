#include <stdio.h>
#include <stdlib.h>

// Function to perform threshold binarization
void threshold_binarization(int** image, int** binary_image, int height, int width, int threshold) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            binary_image[i][j] = (image[i][j] > threshold) ? 1 : 0;
        }
    }
}

// Function to merge binary data
void merge_binary_data(int** binary_image, int** merged_data, int height, int width, int block_size) {
    int merged_width = width / block_size;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < merged_width; j++) {
            int merged_value = 0;
            for (int k = 0; k < block_size; k++) {
                merged_value = (merged_value << 1) | binary_image[i][j * block_size + k];
            }
            merged_data[i][j] = merged_value;
        }
    }
}

// Function to unmerge binary data
void unmerge_binary_data(int** merged_data, int** binary_image, int height, int original_width, int block_size) {
    int merged_width = original_width / block_size;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < merged_width; j++) {
            int merged_value = merged_data[i][j];
            for (int k = 0; k < block_size; k++) {
                binary_image[i][j * block_size + k] = (merged_value >> (block_size - k - 1)) & 1;
            }
        }
    }
}

int main() {
    int height = 320;
    int width = 320;
    int threshold = 90;
    int block_size = 32;

    // Allocate memory for image and binary_image
    int** image = (int**)malloc(height * sizeof(int*));
    int** binary_image = (int**)malloc(height * sizeof(int*));

    for (int i = 0; i < height; i++) {
        image[i] = (int*)malloc(width * sizeof(int));
        binary_image[i] = (int*)malloc(width * sizeof(int));
    }

    // Fill image with data (you'll need to load your image data here)
    
    

    // Call threshold_binarization
    threshold_binarization(image, binary_image, height, width, threshold);

    // Call merge_binary_data
    int merged_width = width / block_size;
    int** merged_data = (int**)malloc(height * sizeof(int*));

    for (int i = 0; i < height; i++) {
        merged_data[i] = (int*)malloc(merged_width * sizeof(int));
    }

    merge_binary_data(binary_image, merged_data, height, width, block_size);

    // Call unmerge_binary_data to recover the binary_image

    // Free allocated memory

    return 0;
}
