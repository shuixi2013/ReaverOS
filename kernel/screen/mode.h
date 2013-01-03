/**
 * Reaver Project OS, Rose License
 * 
 * Copyright (C) 2011-2012 Reaver Project Team:
 * 1. Michał "Griwes" Dominiak
 * 
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 * 
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 * 
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation is required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 * 
 * Michał "Griwes" Dominiak
 * 
 **/

#pragma once

namespace screen
{
    struct mode
    {
        uint32_t addr;
        uint16_t resolution_x;
        uint16_t resolution_y;
        uint16_t bytes_per_line;
        
        uint8_t bpp;
        uint8_t red_size, red_pos;
        uint8_t green_size, green_pos;
        uint8_t blue_size, blue_pos;
        uint8_t rsvd_size, rsvd_pos;
    } __attribute__((packed));
}