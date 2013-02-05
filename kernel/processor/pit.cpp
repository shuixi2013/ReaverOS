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

#include <processor/pit.h>

namespace
{
    processor::interrupts::handler _callbacks[16] = {};
    uint8_t _size = 0;
    
    uint8_t _interrupt = 0;
    
    void _pit_timer_interrupt(processor::idt::irq_context ctx)
    {
        for (uint8_t i = 0; i < _size; ++i)
        {
            _callbacks[i](ctx);
        }
    }
}

void processor::pit::initialize()
{
    _interrupt = interrupts::allocate(_pit_timer_interrupt);
    
    interrupts::set_isa_irq_int_vector(0, _interrupt);
}

uint8_t processor::pit::register_callback(processor::interrupts::handler h)
{
    if (_size == 16)
    {
        PANIC("Too much PIT timer handlers");
    }
    
    _callbacks[_size++] = h;
    return _size - 1;
}

void processor::pit::unregister_callback(uint8_t idx)
{
    if (idx > _size)
    {
        PANIC("Tried to unregister not registered PIT callback");
    }
    
    for (--_size; idx < _size; ++idx)
    {
        _callbacks[idx] = _callbacks[idx + 1];
    }
}

void processor::pit::interrupt(uint64_t microseconds)
{
    outb(0x43, 0x32);
    
    uint16_t divisor = 1193180 / (1000000 / microseconds);
    
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}