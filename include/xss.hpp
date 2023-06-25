// MIT License
//
// Copyright (c) 2023 Ferhat Geçdoğan All Rights Reserved.
// Distributed under the terms of the MIT License.
//
//
#pragma once

#include <vector>
#include <utility>
#include <string>

#include <X11/Xlib.h>
#include <X11/X.h>

class xss_main {
public:
    std::string file_format{};

    void screenshot_to_stream(XImage*& image_data,
                              const std::string& file_name,
                              const std::string& file_format,
                              unsigned width,
                              unsigned height) noexcept;
    void write_to_file_name(std::string& file_name) noexcept;
};