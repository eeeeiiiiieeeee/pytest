#include <stdio.h>
#include <jpeglib.h>

void compress_image(const char* input_image, const char* output_image, int quality) {
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE* infile, *outfile;
    JSAMPROW row_pointer[1];
    int row_stride;

    if ((infile = fopen(input_image, "rb")) == NULL) {
        fprintf(stderr, "Can't open %s\n", input_image);
        return;
    }

    if ((outfile = fopen(output_image, "wb")) == NULL) {
        fprintf(stderr, "Can't create %s\n", output_image);
        fclose(infile);
        return;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);

    // 设置压缩参数
    cinfo.image_width = 320; // 设置图像宽度
    cinfo.image_height = 240; // 设置图像高度
    cinfo.input_components = 3; // 设置输入图像的颜色通道数
    cinfo.in_color_space = JCS_RGB; // 设置输入图像的颜色空间

    // 设置压缩质量
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);

    jpeg_start_compress(&cinfo, TRUE);

    row_stride = cinfo.image_width * cinfo.input_components;
    unsigned char* buffer = (unsigned char*)malloc(row_stride);

    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &buffer[cinfo.next_scanline * row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    free(buffer);
    jpeg_finish_compress(&cinfo);
    fclose(outfile);
    jpeg_destroy_compress(&cinfo);
    fclose(infile);
}

int main() {
    const char* input_image = "/C_Project/bmptest.bmp";
    const char* output_image = "/C_Project/test.jpg";
    int quality = 75;  // 设置压缩质量（1-100）

    compress_image(input_image, output_image, quality);

    return 0;
}


// const char* input_image = "/C_Project/bmptest.bmp";
// const char* output_image = "/C_Project/test.jpg";