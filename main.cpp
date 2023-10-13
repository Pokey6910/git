#include "BMP.h"
#include <iostream>

int main() {
    BMP bmp2("house_24.bmp");                     
    std::cout << bmp2.bmp_header.file_size << "\n"; 
    bmp2.write_file("out1.bmp");                 
    bmp2.toright();                             
    bmp2.write_file("out2.bmp");
    bmp2.toleft();                              
    bmp2.toleft();                              
    bmp2.write_file("out3.bmp");
    bmp2.gauss_filter(5);                       
    bmp2.write_file("out4.bmp");

}