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

#include <acpi/acpi.h>
#include <memory/memory.h>
#include <memory/x64paging.h>

namespace acpi
{
    rsdt * root = nullptr;
    xsdt * new_root = nullptr;
}

template<>
void screen::print_impl(const acpi::rsdp & rsdp)
{
    screen::printl("Revision: ", rsdp.revision);
    screen::printl("OEMID: ", rsdp.oemid);
}

acpi::rsdp * acpi::find_rsdp()
{
    uint32_t ebda = *(uint16_t *)0x40E << 4;
    
    rsdp * ptr = (rsdp *)ebda;
    
    while (ptr < (rsdp *)(ebda + 1024))
    {
        if (ptr->validate())
        {
            if (ptr->revision)
            {
                new_root = (xsdt *)0xFFFFA000;
                memory::vas->map(0xFFFFA000, 0xFFFFFFFF, ptr->xsdt_ptr);
            }
            
            else
            {
                root = (rsdt *)0xFFFFA000;
                memory::vas->map(0xFFFFA000, 0xFFFFFFFF, ptr->rsdt_ptr);
            }
        }
        
        else
        {
            ptr = (rsdp *)((uint32_t)ptr + 16);
        }
    }
    
    ptr = (rsdp *)0xe0000;
    
    while (ptr < (rsdp *)0x100000)
    {
        if (ptr->validate())
        {
            if (ptr->revision)
            {
                new_root = (xsdt *)0xFFFFA000;
                memory::vas->map(0xFFFFA000, 0xFFFFFFFF, ptr->xsdt_ptr);
            }
            
            else
            {
                root = (rsdt *)0xFFFFA000;
                memory::vas->map(0xFFFFA000, 0xFFFFFFFF, ptr->rsdt_ptr);
            }
            
            return ptr;
        }
        
        else
        {
            ptr = (rsdp *)((uint32_t)ptr + 16);
        }
    }
    
    PANIC("RSDP not found!");
    
    return nullptr;
}

processor::numa_env * acpi::find_numa_domains()
{
    srat * resources = nullptr;
    
    if (new_root)
    {
        
    }
    
    else
    {
        
    }
}
