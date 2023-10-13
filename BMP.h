#pragma once
#ifndef WORKBMP
#define WORKBMP
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>


#pragma pack(push, 1)
struct BMPHeader {
    uint16_t file_type{0x4D42};       
    uint32_t file_size{0};            
    uint16_t reserved1{0};            
    uint16_t reserved2{0};            
    uint32_t offset_data{0}; 

    uint32_t size{ 0 };             
    int32_t width{ 0 };                  
    int32_t height{ 0 };             
    uint16_t planes{ 1 };                  
    uint16_t bit_count{ 0 };
    uint32_t compression{ 0 }; 
    uint32_t size_image{ 0 };
    int32_t x_pixels_per_meter{ 0 };
    int32_t y_pixels_per_meter{ 0 };
    uint32_t colors_used{ 0 };             
    uint32_t colors_important{ 0 };            

    uint32_t red_mask{ 0x00ff0000 };      
    uint32_t green_mask{ 0x0000ff00 };      
    uint32_t blue_mask{ 0x000000ff };       
    uint32_t alpha_mask{ 0xff000000 };     
    uint32_t color_space_type{ 0x73524742 }; 
    uint32_t unused[16]{ 0 };
};
#pragma pack(pop)


struct BMP {
    BMPHeader bmp_header;
    std::vector<uint8_t> data_pix;
    std::vector<uint8_t> data_add;

    BMP(const char *fname) {
        read_file(fname);
    }

    void read_file(const char *fname) {
    	std:: ifstream image{fname, std::ios_base::binary};
        if (image) {
            image.read((char*)&bmp_header, sizeof(bmp_header));
            uint32_t pix_len = bmp_header.bit_count / 8;
            int padding_cur = pix_len * bmp_header.width % 4;
            if (padding_cur != 0) padding_cur = 4 - padding_cur;
            
            data_add.resize(bmp_header.offset_data - sizeof(bmp_header));
            image.read((char*)data_add.data(), data_add.size());

            data_pix.resize(bmp_header.height * (bmp_header.width * bmp_header.bit_count / 8 + padding_cur));
            image.read((char*)data_pix.data(), data_pix.size());
        }
        image.close();
    }

    void write_file(const char *fname) {
     	std:: ofstream of{fname, std:: ios_base::binary};
        if (of) {
            of.write((const char*)&bmp_header, sizeof(bmp_header));
            of.write((const char*)data_add.data(), data_add.size());
            of.write((const char*)data_pix.data(), data_pix.size());
        }
        of.close();
    }

    void toright(){
        std::vector <uint8_t> data_sup(data_pix);
        uint32_t pix_len = bmp_header.bit_count / 8;

        int padding_cur = pix_len * bmp_header.width % 4;
        if (padding_cur != 0) padding_cur = 4 - padding_cur;

        int padding_new = pix_len * bmp_header.height % 4;
        if (padding_new != 0) padding_new = 4 - padding_new;

        for (uint32_t y = 0; y < bmp_header.height; y++){
            for (uint32_t x = 0; x < bmp_header.width; x++){
                int32_t ind_cur = pix_len * (bmp_header.height * (bmp_header.width - x - 1) + y) + (bmp_header.width - x - 1) * padding_new;
                for (int pix = 0; pix < pix_len; pix++) {
                    data_pix[ind_cur + pix] = data_sup[pix_len * (x + y * bmp_header.width) + y * padding_cur + pix];
                }
            }
        }
        bmp_header.height += bmp_header.width;
        bmp_header.width = bmp_header.height - bmp_header.width;
        bmp_header.height -= bmp_header.width;
    }

    void toleft() {
        std::vector <uint8_t> data_sup(data_pix);
        uint32_t pix_len = bmp_header.bit_count / 8;

        int padding_cur = pix_len * bmp_header.width % 4;
        if (padding_cur != 0) padding_cur = 4 - padding_cur;

        int padding_new = pix_len * bmp_header.height % 4;
        if (padding_new != 0) padding_new = 4 - padding_new;

        for (uint32_t y = 0; y < bmp_header.height; y++){
            for (uint32_t x = 0; x < bmp_header.width; x++){
                int32_t ind_cur = pix_len * ((bmp_header.height - y - 1) + x * bmp_header.height) + x * padding_new;
                for (int pix = 0; pix < pix_len; pix++) {
                    data_pix[ind_cur+pix] = data_sup[pix_len * (x + y * bmp_header.width) + y * padding_cur + pix];
                }
            }
        }
        bmp_header.height += bmp_header.width;
        bmp_header.width = bmp_header.height - bmp_header.width;
        bmp_header.height -= bmp_header.width;
    }

    void gauss_filter(int r) {
        std::vector <uint8_t> data_sup((bmp_header.width + r *2 ) * (bmp_header.height + r * 2) * bmp_header.bit_count / 8);
        uint32_t pix_len = bmp_header.bit_count / 8;

        int padding_cur = pix_len * bmp_header.width % 4;
        if (padding_cur != 0) padding_cur = 4 - padding_cur;
        double sigm = 0;
        for (int y = 0; y < bmp_header.height; y++){
            for (int x = 0; x < bmp_header.width; x++){
                for (int pix = 0; pix < pix_len; pix++) {
                    sigm += data_pix[pix_len * (x + y * bmp_header.width) + pix + y * padding_cur];
                    data_sup[pix_len * (x+r + (y+r)*(bmp_header.width+r*2)) + pix + padding_cur * (y+r)] = data_pix[pix_len * (x + y * bmp_header.width) + pix + padding_cur*y];
                }
            }
        }
        sigm = sigm/(pix_len*bmp_header.height*bmp_header.width);
        double sigm_sup = 0;
        for (int y = 0; y < bmp_header.height; y++){
            for (int x = 0; x < bmp_header.width; x++){
                for (int pix = 0; pix < pix_len; pix++) {
                    sigm_sup += pow(data_pix[pix_len * (x + y * bmp_header.width) + pix + padding_cur*y]-sigm, 2);
                }
            }
        }
        sigm = sqrt(sigm_sup);
        const double PI = 3.141;
        std:: vector <double> d(4);
        for (int y = 0; y < bmp_header.height; y++){
            for (int x = 0; x < bmp_header.width; x++){
                d = {0, 0, 0, 0};
                for (int i = -r; i < r; i++){
                    for (int j = -r; j < r; j++){
                        for (int pix = 0; pix < pix_len; pix++) {
                            d[pix] += (1/ pow(exp(1), (i*i + j*j)/(2*sigm*sigm))) * data_sup[pix_len * ((x+r + i) + (bmp_header.width + r * 2) * (y + r + j)) + pix + (y + r + j)*padding_cur];
                        }
                    }
                }
                for (int pix = 0; pix < pix_len; pix++) {
                    data_pix[pix_len * (x + y * bmp_header.width)+pix+y*padding_cur] = d[pix] * (1/(2 * PI * r * r));

                }
            }
        }
    }

};
#endif