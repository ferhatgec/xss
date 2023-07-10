// MIT License
//
// Copyright (c) 2023 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//
//

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "../include/stb_image_write.h"
#include <X11/Xutil.h>

/*
this class thing isn't required
 
1) it could've been `struct`, since it's just public anyway

2) it could've been just a namespace since you just use it once

3) it could just be a global system since this is a single file thing

tl;dr : oop bad, use data oriented programming

#include "../include/xss.hpp"
*/

/* 
    the c++ standard library is super bloated
    It's better to use the least amount of it as you can

    also some of modern C++'s syntax is just so bad I hate to look at it

    why do you enfoce the bad parts for no reason :,( "std::string str{"str"}"
    you could've at least done std::string str("str"), but no :(

    I'd replace all your c++ standard code with sili.h , but I'm not going to
    fundementally change your code, just reorganize some of the data in it

    [and of course, write comments like these :(]
*/

/*
    I replaced the iostream with c standard I/O, again because it's less bloated
    but also less ugly
*/


#include <unistd.h> /* replaced c++ standard time sleep with the unix standard library's sleep function, (again, less bloated and it looks cleaner) */
#include <sys/stat.h> /* replaced the c++ standard file system with the unix file system (to see if a path exists already) */
#include <string>

#include <time.h> /* chrono is kinda bloated and ugly, replaced it with c standard time (chrono is also probably a [bad] wrapper around this) */

/* 
    there's a file format string thing

    not sure why it was there though
*/

/* to replace stoi because it's slow/bloated (just as everything else in the c++ standard library as a rule) */
int cstr_to_int(char* str) {
    if (str == NULL) {
        exit(0);
    }

	int result = 0;
	char cur;
	bool negative = false;

	if ((*str) == '-') {
		negative = true;
		str++;
	}

	while ((cur = *str++)) {
		if (cur >= '0' && cur <= '9') {
			result = (result * 10) + (cur - '0');
		}
        else {
            printf("Attempted to use `cstr_to_int` with a string that contains non numbers.");
            exit(0);
        }
	}
	
	if (negative) {
		result = -result;
	}

    return result;
}

/* replacement for std::to_string because it's probably bloated too */
std::string u32_to_string(unsigned int num) {
	std::string res;

	do {
		res.insert(res.begin(), (num % 10) +'0');
		num /= 10;
	} while (num > 0);

	return res;
}

void screenshot_to_stream(XImage *&image_data,
                                    const char* file_name, /* chnage to char*, [char* is better, plus now you don't have to convert it in function]*/
                                    const std::string& file_format, /* it'd probably be better if the format wasn't a string for performance */
                                    unsigned width,
                                    unsigned height) {
    if((width == 0) || (height == 0)) return;

    /*
        this makes it so the data isn't allocated [and now doesn't need to be free]
        it's just memory from the stack [this is also faster]
    */
    unsigned char arr[width * height * 3];
    
    unsigned long red_mask = image_data->red_mask;
    unsigned long green_mask = image_data->green_mask;
    unsigned long blue_mask = image_data->blue_mask;

    /* 
        why use std::size_t !?
        just use size_t 
    */
    
    for(size_t width_index = 0; width_index < width; ++width_index) {
        for(size_t height_index = 0; height_index < height; ++height_index) {
            unsigned long pixel = XGetPixel(image_data, width_index, height_index);

            arr[(width_index + width * height_index) * 3] = ((pixel & red_mask) >> 16);
            arr[(width_index + width * height_index) * 3 + 1] = ((pixel & green_mask) >> 8);
            arr[(width_index + width * height_index) * 3 + 2] = (pixel & blue_mask);
        }
    }

    /* 
        not only would it be better performance
        but it would also be better syntax because then you could use a switch here instead 
        of a bunch of if statements [if you didn't use strings]

        and the switch would also incrase performance and make this part faster too
    */

    if(file_format == "jpg" || file_format == "jpeg" || file_format.empty())
        stbi_write_jpg(file_name, width, height, 3, arr, width * 3);
    else if(file_format == "png")
        stbi_write_png(file_name, width, height, 3, arr, width * 3);
    else if(file_format == "tga")
        stbi_write_tga(file_name, width, height, 3, arr);
    else if(file_format == "bmp")
        stbi_write_bmp(file_name, width, height, 3, arr);
    else {
        printf("the given format is invalid, using jpeg\n");
        stbi_write_jpg(file_name, width, height, 3, arr, width * 3);
    }
}


/* 
    this function name is really bad

    maybe it should be something like
    `create_file_name` or something, not sure

    write_to_file_name makes it seem like it's going to write a file
    when all it does is finds a new file name based on the date

    (maybe date_to_file_name !?)
*/

void write_to_file_name(std::string& file_name) {
    time_t t = time(NULL);
    struct tm now = *localtime(&t);
    
    unsigned indx = 0;
    /* 
        modern C++ syntax makes me want to die

        notice how this C library doesn't look like garbage 
        [and probably runs way better]

        except for the .tm_ part, that's a big silly
    */

    /* 
        not sure why you say

        _year, it's kinda ugly
    */
    const int year = now.tm_year + 1900;
    const unsigned month = now.tm_mon + 1;
    const unsigned day = now.tm_mday;

    file_name = "screenshot_" + u32_to_string(year) + "_"
                + u32_to_string(month) + "_"
                + u32_to_string(day) + "_"
                + u32_to_string(indx);

    /* (while path with filename exists) */
    struct stat tmp;

    while(!stat(file_name.c_str(), &tmp)) {
        ++indx; /* ++ before indx !? kinda funky looking :( */
        file_name = "screenshot_" + u32_to_string(year) + "_"
                    + u32_to_string(month) + "_"
                    + u32_to_string(day) + "_"
                    + u32_to_string(indx);
    }
}

int main(int argc, char** argv) {
    /* 
        why use {}, it's a bit forgiveable here because this is an constructor 
        but still :(
    */
    std::string file_name{}, file_format{"jpg"};

    Display* dp = XOpenDisplay(NULL);
    Window wnd = DefaultRootWindow(dp);

    XWindowAttributes attrs;
    XGetWindowAttributes(dp, wnd, &attrs);

    /* 
        not sure why you're using {}
        
        just be normal and use =
    */
    
    int width {attrs.width},
        height {attrs.height},
        center_width {0},
        center_height {0},
        wait_time {0};

    bool append_extension {false};

    if(argc < 2) {
        printf("you may want to use '-h' to list arguments.\n");
        write_to_file_name(file_name);
        append_extension = true;
    } else {
        for(size_t indx = 1; indx < argc; ++indx) {
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
                            width = cstr_to_int(argv[++indx]);
                            height = cstr_to_int(argv[++indx]);
                        } break;
                    }

                    case 'i': {
                        // -i jpg | png | bmp | tga
                        if(indx + 1 < argc)
                            file_format = std::string(argv[++indx]);

                        if(!file_format.empty() && file_format.front() == '.') /* could've used [0] but */
                            file_format.erase(0);

                        break;
                    }

                    case 'c': {
                        // -c width height
                        if(indx + 1 < argc) {
                            center_width = cstr_to_int(argv[++indx]);
                            center_height = cstr_to_int(argv[++indx]);
                        } break;
                    }

                    case 't': {
                        // -t seconds
                        if(indx + 1 < argc)
                            wait_time = cstr_to_int(argv[++indx]);
                        break;
                    }

                    case 'a': {
                        // -a
                        append_extension = true;
                        break;
                    }

                    case 'h': {
                        // -h
                        printf("%i [arguments]\n"
                                            "\n"
                                            "[arguments]:\n"
                                            " * -f filename : set filename of output image (with image format extension)\n"
                                            " * -s width height : set size of output image\n"
                                            " * -i format : set output image format (tga, jpg, png and bmp supported)\n"
                                            " * -c width height : set start position of crop layout\n"
                                            " * -t seconds : wait before capture\n"
                                            " * -a : append format suffix to output filename\n"
                                            " * -h : show help\n", argv[0]);

                        if(argc == 2) {
                            XCloseDisplay(dp);

                            /* just use exit(), why even use std::exit() !? */
                            exit(0);
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
        write_to_file_name(file_name);

    sleep(wait_time);

    XImage* img = XGetImage(dp, wnd, center_width, center_height, width, height, AllPlanes, ZPixmap);

    if(append_extension)
        file_name.append("." + file_format);

    screenshot_to_stream(img, file_name.c_str(), file_format, width, height);

    printf("image saved as %s\n", file_name.c_str());

    XCloseDisplay(dp);
    XFree(img);
}
