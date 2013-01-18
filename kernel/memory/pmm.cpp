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

#include <memory/pmm.h>
#include <memory/vm.h>
#include <memory/map.h>
#include <memory/x64paging.h>
#include <processor/processor.h>
#include <screen/screen.h>

namespace
{
    memory::pmm::frame_stack boot_stack;
    
    uint8_t boot_helper_frames[3 * 4096] __attribute__((aligned(4096)));
    uint8_t boot_helpers_available = 3;
    
    memory::map_entry * memory_map;
    uint64_t map_size;
}

void memory::pmm::initialize(memory::map_entry * map, uint64_t map_size)
{
    new ((void *)&boot_stack) frame_stack(map, map_size);
    
    memory_map = map;
    ::map_size = map_size;
}

memory::pmm::frame_stack::frame_stack() : _stack(nullptr), _size(0), _capacity(0)
{
}

memory::pmm::frame_stack::frame_stack(memory::map_entry * map, uint64_t map_size) : _stack((uint64_t *)vm::boot_page_stack), 
    _size(0), _capacity(0)
{
    for (uint64_t i = 0; i < map_size; ++i)
    {
        if (map[i].type != 1)
        {
            continue;
        }
        
        if (map[i].base < 1024 * 1024 && map[i].base + map[i].length <= 1024 * 1024)
        {
            continue;
        }
        
        for (uint64_t frame = (map[i].base < 1024 * 1024) ? (1024 * 1024) : ((map[i].base + 4095) & ~(uint64_t)4095); frame < map[i].base
            + map[i].length; frame += 4096)
        {            
            push(frame);
        }
    }
}

void memory::pmm::frame_stack::_expand()
{
    vm::map(vm::boot_page_stack + _capacity * 8);
    
    _capacity += 512;
}

void memory::pmm::frame_stack::_shrink()
{
}

uint64_t memory::pmm::frame_stack::pop()
{
    if (_size == 0)
    {
        if (_capacity == 0)
        {
            if (boot_helpers_available)
            {
                --boot_helpers_available;
                return vm::get_physical_address((uint64_t)boot_helper_frames + boot_helpers_available * 4096);
            }
            
            // if (scheduler::ready())
            // {
            //     scheduler::request_caches();
            // }
            //
            // memory::vm::gc_paging_structures();

            if (_size == 0)
            {
                PANIC("System has ran out of memory");
            }
        }
        
        // uint64_t ret_frame = memory::vm::get_physical_address(_stack);
        // memory::vm::unmap(_stack);
        // _stack = nullptr;
        // return ret_frame;
    }
    
    // if (_size < 128)
    // {
    //     if (scheduler::ready())
    //     {
    //         scheduler::request_caches(false);
    //     }
    //
    //     memory::vm::gc_paging_structures(false);
    // }    
    
    // if (_capacity - _size > 512)
    // {
    //     _shrink();
    // }
    
    return _stack[--_size];
}

void memory::pmm::frame_stack::push(uint64_t frame)
{
    if (_size == _capacity)
    {
        _expand();
    }
    
    _stack[_size++] = frame;
}

uint64_t memory::pmm::pop()
{
    // if (processor::ready())
    // {
    //     return processor::core.frame_stack.pop();
    // }
    
    return boot_stack.pop();
}

void memory::pmm::push(uint64_t frame)
{
    // if (processor::ready())
    // {
    //     return processor::core.frame_stack.push(frame);
    // }
    
    return boot_stack.push(frame);
}

void memory::pmm::boot_report()
{
    screen::print("Free memory: ", (boot_stack.size() * 4096) / (1024 * 1024 * 1024), " GiB ", ((boot_stack.size() * 4096 ) % 
        (1024 * 1024 * 1024)) / (1024 * 1024), " MiB ", ((boot_stack.size() * 4096 ) % (1024 * 1024)) / 1024, " KiB", '\n');
    screen::print("Total usable memory detected at boot: ");

    uint64_t total = 0;
    
    for (uint64_t i = 0; i < map_size; ++i)
    {
        if (memory_map[i].type != 6 && memory_map[i].type != 8 && memory_map[i].type != 9)
        {
            total += memory_map[i].length;
        }
    }
        
    screen::print(total / (1024 * 1024 * 1024), " GiB ", (total % 1024 * 1024 * 1024) / (1024 * 1024), " MiB ", (total % 1024 * 1024) / 1024,
        " KiB", '\n', '\n');
}