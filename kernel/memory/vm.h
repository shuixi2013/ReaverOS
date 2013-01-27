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

#pragma once

#include <processor/processor.h>
#include <memory/pmm.h>

namespace memory
{
    namespace vm
    {
        enum addresses
        {
            boot_page_stack =                   0xFFFFFFFF40000000,
            boot_video_memory =                 0xFFFFFFFE80000000,
            boot_backbuffer =                   0xFFFFFFFF00000000,
            acpi_temporal_table_mapping_start = 0xFFFFFFFFFFFE8000,
            acpi_temporal_table_mapping_end =   0xFFFFFFFFFFFEFFFF,
            acpi_temporal_rsdt_mapping_start =  0xFFFFFFFFFFFF0000,
            acpi_temporal_rsdt_mapping_end =    0xFFFFFFFFFFFF7FFF,
            apic_area =                         0xFFFFFFFE40000000
        };
        
        inline void map(uint64_t virtual_address)
        {
            x64::map(virtual_address, virtual_address + 4096, memory::pmm::pop());
        }
        
        inline void map(uint64_t virtual_address, uint64_t physical_address)
        {
            x64::map(virtual_address, virtual_address + 4096, physical_address);
        }
        
        void map_multiple(uint64_t, uint64_t);
        
        inline void map_multiple(uint64_t virtual_start, uint64_t virtual_end, uint64_t physical_start)
        {
            x64::map(virtual_start, virtual_end, physical_start);
        }
        
        inline uint64_t get_physical_address(uint64_t virtual_address)
        {
            return x64::get_physical_address(virtual_address);
        }
        
        inline void unmap(uint64_t address, bool push = true)
        {
            x64::unmap(address, (address + 4095) & ~(uint64_t)4096, push);
        }
        
        inline void unmap(uint64_t start, uint64_t end, bool push = true)
        {
            x64::unmap(start, end, push);
        }
    }
}
