/**
 * Reaver Project OS, Rose License
 * 
 * Copyright (C) 2011-2013 Reaver Project Team:
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

#include <screen/console.h>
#include <screen/terminal.h>

namespace screen
{
    kernel_console console;
}

screen::kernel_console::kernel_console(terminal * term) : _terminal(term)
{
}

void screen::kernel_console::clear()
{
    _terminal->clear();
}

void screen::kernel_console::print(char c)
{
    _terminal->put_char(c);
    
    if (c != '\0')
    {
        outb(0x378, (unsigned char)c);
        outb(0x37a, 0x0c);
        outb(0x37a, 0x0d);
    }
}

void screen::kernel_console::print(const char * str)
{
    while (*str)
    {
        outb(0x378, (unsigned char)*str);
        outb(0x37a, 0x0c);
        outb(0x37a, 0x0d);
        
        _terminal->put_char(*str++);
    }
}

void screen::kernel_console::set_color(color::colors c)
{
    _terminal->set_color(c);
}
