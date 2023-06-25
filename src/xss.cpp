// MIT License
//
// Copyright (c) 2023 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//
//

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "../include/stb_image_write.h"
#include "../include/xss.hpp"

#include <X11/Xutil.h>
#include <chrono>
#include <filesystem>
#include <thread>
#include <iostream>

void xss_main::screenshot_to_stream(XImage *&image_data,
                                    const std::string& file_name,
                                    const std::string& file_format,
                                    unsigned width,
                                    unsigned height) noexcept {
    if((width == 0) || (height == 0)) return;

    unsigned char* arr = new unsigned char[width * height * 3];
    unsigned long red_mask = image_data->red_mask;
    unsigned long green_mask = image_data->green_mask;
    unsigned long blue_mask = image_data->blue_mask;

    for(std::size_t width_index = 0; width_index < width; ++width_index) {
        for(std::size_t height_index = 0; height_index < height; ++height_index) {
            unsigned long pixel = XGetPixel(image_data, width_index, height_index);

            arr[(width_index + width * height_index) * 3] = ((pixel & red_mask) >> 16);
            arr[(width_index + width * height_index) * 3 + 1] = ((pixel & green_mask) >> 8);
            arr[(width_index + width * height_index) * 3 + 2] = (pixel & blue_mask);
        }
    }

    if(file_format == "jpg" || file_format == "jpeg" || file_format.empty())
        stbi_write_jpg(file_name.c_str(), width, height, 3, arr, width * 3);
    else if(file_format == "png")
        stbi_write_png(file_name.c_str(), width, height, 3, arr, width * 3);
    else if(file_format == "tga")
        stbi_write_tga(file_name.c_str(), width, height, 3, arr);
    else if(file_format == "bmp")
        stbi_write_bmp(file_name.c_str(), width, height, 3, arr);
    else {
        std::cout << "the given format is invalid, using jpeg\n";
        stbi_write_jpg(file_name.c_str(), width, height, 3, arr, width * 3);
    }

    delete[] arr;
}

void xss_main::write_to_file_name(std::string& file_name) noexcept {
    const auto now = std::chrono::system_clock::now();
    const auto today = std::chrono::time_point_cast<std::chrono::days>(now);

    const std::chrono::year_month_day ymd {today};
    unsigned indx = 0;
    const int _year { static_cast<int>(int(ymd.year())) };
    const unsigned _month { static_cast<unsigned>(ymd.month()) };
    const unsigned _day { static_cast<unsigned>(ymd.day()) };

    file_name = "screenshot_" + std::to_string(_year) + "_"
                + std::to_string(_month) + "_"
                + std::to_string(_day) + "_"
                + std::to_string(indx);

    while(std::filesystem::exists(file_name)) {
        ++indx;
        file_name = "screenshot_" + std::to_string(_year) + "_"
                    + std::to_string(_month) + "_"
                    + std::to_string(_day) + "_"
                    + std::to_string(indx);
    }
}

int main(int argc, char** argv) {
    std::string file_name{}, file_format{"jpg"};

    Display* dp = XOpenDisplay(NULL);
    Window wnd = DefaultRootWindow(dp);

    xss_main init;

    XWindowAttributes attrs;
    XGetWindowAttributes(dp, wnd, &attrs);

    int width {attrs.width},
        height {attrs.height},
        center_width {0},
        center_height {0},
        wait_time {0};

    bool append_extension {false};

    if(argc < 2) {
        std::cout << "you may want to use '-h' to list arguments.\n";
        init.write_to_file_name(file_name);
        append_extension = true;
    } else {
        for(std::size_t indx = 1; indx < argc; ++indx) {
            if(strlen(argv[indx]) >= 2 && argv[indx][0] == '-') {
                switch(argv[indx][1]) {
                    case 'f': {
                        // -f filename
                        if(indx + 1 < argc)
                            file_name = std::string(argv[++indx]);
                        break;
                    }

                    case 's': {
                        // -s width height
                        if(indx + 1 < argc) {
                            width = std::stoi(argv[++indx]);
                            height = std::stoi(argv[++indx]);
                        } break;
                    }

                    case 'i': {
                        // -i jpg | png | bmp | tga
                        if(indx + 1 < argc)
                            file_format = std::string(argv[++indx]);

                        if(!file_format.empty() && file_format.front() == '.')
                            file_format.erase(0);

                        break;
                    }

                    case 'c': {
                        // -c width height
                        if(indx + 1 < argc) {
                            center_width = std::stoi(argv[++indx]);
                            center_height = std::stoi(argv[++indx]);
                        } break;
                    }

                    case 't': {
                        // -t seconds
                        if(indx + 1 < argc)
                            wait_time = std::stoi(argv[++indx]);
                        break;
                    }

                    case 'a': {
                        // -a
                        append_extension = true;
                        break;
                    }

                    case 'h': {
                        // -h
                        std::cout << argv[0] << " [arguments]\n"
                                                "\n"
                                                "[arguments]:\n"
                                                " * -f filename : set filename of output image (with image format extension)\n"
                                                " * -s width height : set size of output image\n"
                                                " * -i format : set output image format (tga, jpg, png and bmp supported)\n"
                                                " * -c width height : set start position of crop layout\n"
                                                " * -t seconds : wait before capture\n"
                                                " * -a : append format suffix to output filename\n"
                                                " * -h : show help\n";

                        if(argc == 2) {
                            XCloseDisplay(dp);
                            std::exit(0);
                        }
                    }
                }
            }
        }
    }

    if(center_height + height > attrs.height) {
        center_height = 0;
    }

    if(center_width + width > attrs.width) {
        center_width = 0;
    }

    if(file_name.empty())
        init.write_to_file_name(file_name);

    std::this_thread::sleep_for(std::chrono::seconds(wait_time));

    XImage* img = XGetImage(dp, wnd, center_width, center_height, width, height, AllPlanes, ZPixmap);

    if(append_extension)
        file_name.append("." + file_format);

    init.screenshot_to_stream(img, file_name, file_format, width, height);

    std::cout << "image saved as " + file_name << "\n";

    XCloseDisplay(dp);
    XFree(img);
}