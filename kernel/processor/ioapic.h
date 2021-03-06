/**
 * Reaver Project OS, Rose License
 *
 * Copyright © 2011-2013 Michał "Griwes" Dominiak
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
 **/

#pragma once

#include <memory/vm.h>
#include <processor/interrupt_entry.h>
#include <processor/processor.h>
#include <screen/screen.h>
#include <processor/lapic.h>

namespace processor
{
    class ioapic
    {
    public:
        ioapic() : _is_valid{}, _is_nmi_valid{} {}

        ioapic(uint32_t apic_id, uint32_t base_vector, phys_addr_t base_address) : _apic_id{ apic_id }, _base_vector_number{ base_vector },
            _is_valid{ true }, _is_nmi_valid{ false }
        {
            auto address = memory::vm::allocate_address_range(4096);

            memory::vm::map(address, base_address);
            _base_address = address;

            _size = ((_read_register(1) >> 16) & (~(1 << 8) & 0xFF)) + 1;
        }

        bool set_global_nmi(uint32_t vector, uint32_t flags)
        {
            if (vector >= _base_vector_number && vector < _base_vector_number + _size)
            {
                _global_nmi_vector = vector;
                _global_nmi_flags = flags;

                _is_nmi_valid = true;
            }

            return _is_nmi_valid;
        }

        void initialize()
        {
            for (uint32_t i = 0; i < _size; ++i)
            {
                _write_register(0x10 + 2 * i, _read_register(0x10 + 2 * i) | (1 << 16));
            }
        }

        uint32_t begin()
        {
            return _base_vector_number;
        }

        uint32_t end()
        {
            return _base_vector_number + _size;
        }

        void route_interrupt(uint8_t input, uint8_t local_input)
        {
            bool low = false;
            bool level = false;

            auto sources = processor::sources();

            if (sources[input])
            {
                if (!sources[input].standard_polarity())
                {
                    low = sources[input].low();
                }

                if (!sources[input].standard_trigger())
                {
                    level = sources[input].level();
                }
            }

            uint64_t ioreg = local_input;
            ioreg |= low << 13;
            ioreg |= level << 15;

            if (x2apic_enabled())
            {
                if (remapping::enabled())
                {
                    TODO;
                }

                else
                {
                    // uint64_t target_cpu = get_cpu_for_interrupt();
                    // TODO: THIS IS BROKEN!!!
                    uint64_t target_cpu = id();

                    ioreg |= target_cpu << 56;
                }
            }

            else
            {
                ioreg |= 1 << 11;
                ioreg |= 0xFFull << 56;
                ioreg |= 1 << 8;
            }

            _write_register(0x10 + processor::translate_isa(input) * 2, ioreg & 0xFFFFFFFF);
            _write_register(0x10 + processor::translate_isa(input) * 2 + 1, (ioreg >> 32) & 0xFFFFFFFF);
        }

    private:
        uint32_t _apic_id;
        uint32_t _base_vector_number;
        uint32_t _size;

        virt_addr_t _base_address;

        uint32_t _global_nmi_vector;
        uint32_t _global_nmi_flags;

        bool _is_valid;
        bool _is_nmi_valid;

        uint32_t _read_register(uint8_t id)
        {
            *static_cast<volatile uint32_t *>(_base_address) = id;
            return *static_cast<volatile uint32_t *>(_base_address + 0x10);
        }

        void _write_register(uint8_t id, uint32_t val)
        {
            *static_cast<volatile uint32_t *>(_base_address) = id;
            *static_cast<volatile uint32_t *>(_base_address + 0x10) = val;
        }
    };
}
