/**
 * ReaverOS
 * kernel/memory/heap.cpp
 * Kernel heap implementation.
 */

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

#include "heap.h"
#include "vmm.h"

Memory::Heap::Heap(uint64 start)
        : m_pBiggest((AllocationBlockHeader *)start), m_pSmallest((AllocationBlockHeader *)start),
          m_iStart(start), m_iEnd(start + 4 * 4 * 1024)
{
    Memory::VMM::MapPage(start);
    Memory::VMM::MapPage(start + 4096);
    Memory::VMM::MapPage(start + 2 * 4096);
    Memory::VMM::MapPage(start + 3 * 4096);

    this->m_pBiggest->Magic = 0xFEA7EFA1;
    this->m_pBiggest->Size = 4 * 4 * 1024 - sizeof(AllocationBlockHeader) - sizeof(AllocationBlockFooter);
    this->m_pBiggest->Footer()->Magic = 0xFEA7EFA1;
    this->m_pBiggest->Footer()->Header = this->m_pBiggest;
    this->m_pBiggest->Bigger = nullptr;
    this->m_pBiggest->Smaller = nullptr;
    this->m_pBiggest->Flags = 0;
}

Memory::Heap::~Heap()
{
    for (uint64 s = this->m_iStart; s < this->m_iEnd; s += 4096)
    {
        Memory::VMM::UnmapPage(s);
    }
}

// heap uses double-ended queue. I could probably implement some kind of forward and
// reverse iterators to reduce code duplication, but as for now, I just want to have
// this working, optimization (at source and execution levels) will be done later
void * Memory::Heap::Alloc(uint64 iSize)
{    
    this->m_pLock.Lock();

    if (this->m_pBiggest == nullptr || this->m_pSmallest == nullptr)
    {
        if (this->m_pSmallest != nullptr || this->m_pBiggest != nullptr)
        {
            // I'm yet to write anything like "PANIC()"...
            //            PANIC("Critical heap error: Biggest == nullptr, but Smallest != nullptr!");
            __asm("hlt");
        }
    }
    
    AllocationBlockHeader * list = nullptr;

    if (abs(this->m_pBiggest->Size - iSize) < abs(this->m_pSmallest->Size - iSize))
    {
        list = this->m_pBiggest;

        while (list->Size != iSize && list->Smaller != nullptr && list->Smaller->Size >= iSize)
        {
            list = list->Smaller;
        }

        if (list == this->m_pBiggest)
        {
            if (this->m_pBiggest->Smaller == nullptr)
            {
                if (this->m_pBiggest->Size <= iSize)
                {
                    this->_expand();
                    return this->Alloc(iSize);
                }

                else
                {
                    if (list->Size - iSize <= sizeof(AllocationBlockHeader) + sizeof(AllocationBlockFooter))
                    {
                        this->_expand();
                        return this->Alloc(iSize);
                    }

                    else
                    {
                        AllocationBlockHeader * newhead = nullptr;
                        uint64 s = list->Size;
                        list->Size = iSize;
                        list->Flags |= 1;
                        list->Footer()->Header = list;
                        list->Footer()->Magic = 0xFEA7EFA1;
                        
                        newhead = list->Next();
                        newhead->Magic = 0xFEA7EFA1;
                        newhead->Flags = 0;
                        newhead->Bigger = nullptr;
                        newhead->Smaller = nullptr;
                        newhead->Size = s - iSize - sizeof(AllocationBlockHeader) - sizeof(AllocationBlockFooter);
                        newhead->Footer()->Magic = 0xFEA7EFA1;
                        newhead->Footer()->Header = newhead;

                        this->_insert(newhead);

                        this->m_pBiggest = list->Smaller;

                        return this->_validate((void *)((uint8 *)list + sizeof(AllocationBlockHeader)));
                    }
                }
            }

            this->m_pBiggest = list->Smaller;
        }

        if (list->Size == iSize)
        {
            if (list->Bigger != nullptr)
            {
                list->Bigger->Smaller = list->Smaller;
            }
            
            else if (list->Smaller != nullptr)
            {
                list->Smaller->Bigger = list->Bigger;
            }
            
            list->Smaller = nullptr;
            list->Bigger = nullptr;

            list->Flags |= 1;

            return this->_validate((void *)((char *)list + sizeof(AllocationBlockHeader)));
        }

        else
        {
            if (list->Size - iSize > sizeof(AllocationBlockHeader) + sizeof(AllocationBlockFooter) + 4)
            {
                list->Flags |= 1;

                uint64 s = list->Size;
                list->Size = iSize;
                AllocationBlockFooter * newfoot = list->Footer();
                AllocationBlockHeader * newhead = (AllocationBlockHeader *)((uint8 *)newfoot + sizeof(AllocationBlockFooter));

                newhead->Magic = 0xFEA7EFA1;
                newhead->Flags = 0;
                newhead->Bigger = nullptr;
                newhead->Smaller = nullptr;
                newhead->Size = s - iSize - sizeof(AllocationBlockHeader) - sizeof(AllocationBlockFooter);

                newfoot->Magic = 0xFEA7EFA1;
                newfoot->Header = list;

                this->_insert(newhead);
                
                newhead->Footer()->Header = newhead;

                return this->_validate((void *)((uint8 *)list + sizeof(AllocationBlockHeader)));
            }

            // no real reason to create additional block...
            else
            {
                list->Bigger->Smaller = list->Smaller;
                list->Smaller->Bigger = list->Bigger;

                list->Smaller = nullptr;
                list->Bigger = nullptr;

                list->Flags |= 1;

                return this->_validate((void *)((uint8 *)list + sizeof(AllocationBlockHeader)));
            }
        }
    }

    else
    {
        list = this->m_pSmallest;
        
        while (list->Size != iSize && list->Bigger != nullptr && list->Bigger->Size <= iSize)
        {
            list = list->Smaller;
        }
        
        if (list == this->m_pSmallest)
        {
            if (this->m_pSmallest->Bigger == nullptr)
            {
                if (this->m_pSmallest->Size <= iSize)
                {
                    this->_expand();
                    return this->Alloc(iSize);
                }
                
                else
                {
                    if (list->Size - iSize <= sizeof(AllocationBlockHeader) + sizeof(AllocationBlockFooter))
                    {
                        this->_expand();
                        return this->Alloc(iSize);
                    }
                    
                    else
                    {
                        AllocationBlockHeader * newhead = nullptr;
                        uint64 s = list->Size;
                        list->Size = iSize;
                        list->Flags |= 1;
                        list->Footer()->Header = list;
                        list->Footer()->Magic = 0xFEA7EFA1;
                        
                        newhead = list->Next();
                        newhead->Magic = 0xFEA7EFA1;
                        newhead->Flags = 0;
                        newhead->Bigger = nullptr;
                        newhead->Smaller = nullptr;
                        newhead->Size = s - iSize - sizeof(AllocationBlockHeader) - sizeof(AllocationBlockFooter);
                        newhead->Footer()->Magic = 0xFEA7EFA1;
                        newhead->Footer()->Header = newhead;
                        
                        this->_insert(newhead);

                        this->m_pSmallest = list->Bigger;
                        
                        return this->_validate((void *)((uint8 *)list + sizeof(AllocationBlockHeader)));
                    }
                }
            }
            
            this->m_pSmallest = list->Bigger;
        }
        
        if (list->Size == iSize)
        {
            if (list->Smaller != nullptr)
            {
                list->Smaller->Bigger = list->Bigger;
            }
            
            else if (list->Bigger != nullptr)
            {
                list->Bigger->Smaller = list->Smaller;
            }
            
            list->Smaller = nullptr;
            list->Bigger = nullptr;
            
            list->Flags |= 1;
            
            return this->_validate((void *)((uint8 *)list + sizeof(AllocationBlockHeader)));
        }
        
        else
        {
            if (list->Size - iSize > sizeof(AllocationBlockHeader) + sizeof(AllocationBlockFooter) + 4)
            {
                list->Flags |= 1;

                uint64 s = list->Size;
                list->Size = iSize;
                AllocationBlockFooter * newfoot = list->Footer();
                AllocationBlockHeader * newhead = (AllocationBlockHeader *)((uint8 *)newfoot + sizeof(AllocationBlockFooter));
                
                newhead->Magic = 0xFEA7EFA1;
                newhead->Flags = 0;
                newhead->Bigger = nullptr;
                newhead->Smaller = nullptr;
                newhead->Size = s - iSize - sizeof(AllocationBlockHeader) - sizeof(AllocationBlockFooter);

                newfoot->Magic = 0xFEA7EFA1;
                newfoot->Header = list;
                
                this->_insert(newhead);
                
                newhead->Footer()->Header = newhead;
                
                return this->_validate((void *)((uint8 *)list + sizeof(AllocationBlockHeader)));
            }
            
            // no real reason to create additional block...
            else
            {
                list->Smaller->Bigger = list->Bigger;
                list->Bigger->Smaller = list->Smaller;
                
                list->Smaller = nullptr;
                list->Bigger = nullptr;
                
                list->Flags |= 1;
                
                return this->_validate((void *)((uint8 *)list + sizeof(AllocationBlockHeader)));
            }
        }
    }

    this->m_pLock.Unlock();
}

void * Memory::Heap::AllocAligned(uint64 iSize)
{
    this->m_pLock.Lock();
    
    if (this->m_pBiggest == nullptr || this->m_pSmallest == nullptr)
    {
        if (this->m_pSmallest != nullptr || this->m_pBiggest != nullptr)
        {
            // I'm yet to write anything like "PANIC()"...
            //            PANIC("Critical heap error: Biggest == nullptr, but Smallest != nullptr!");
            __asm("hlt");
        }
    }

    uint64 iNumPages = iSize / 4096 + (iSize % 4096 == 0 ? 0 : 1);

    this->m_pLock.Unlock();
}

void Memory::Heap::Free(void * pAddr)
{
    this->m_pLock.Lock();

    if (this->m_pBiggest == nullptr || this->m_pSmallest == nullptr)
    {
        if (this->m_pSmallest != nullptr || this->m_pBiggest != nullptr)
        {
            // I'm yet to write anything like "PANIC()"...
            //            PANIC("Critical heap error: Biggest == nullptr, but Smallest != nullptr!");
            __asm("hlt");
        }
    }
    
    this->_validate(pAddr);
    AllocationBlockHeader * newhead = (AllocationBlockHeader *)((uint8 *)pAddr + sizeof(AllocationBlockHeader));
    newhead->Flags ^= 1;
    newhead->Smaller = nullptr;
    newhead->Bigger = nullptr;
    this->_insert(newhead);
    
    this->m_pLock.Unlock();
}

void Memory::Heap::_expand()
{
    if (this->m_pBiggest == nullptr || this->m_pSmallest == nullptr)
    {
        if (this->m_pSmallest != nullptr || this->m_pBiggest != nullptr)
        {
            // I'm yet to write anything like "PANIC()"...
            //            PANIC("Critical heap error: Biggest == nullptr, but Smallest != nullptr!");
            __asm("hlt");
        }
    }

    Memory::VMM::MapPage(this->m_iEnd);

    AllocationBlockHeader * newhead = (AllocationBlockHeader *)this->m_iEnd;
    newhead->Magic = 0xFEA7EFA1;
    newhead->Size = 4096 - sizeof(AllocationBlockHeader) - sizeof(AllocationBlockFooter);
    newhead->Flags = 0;
    newhead->Bigger = nullptr;
    newhead->Smaller = nullptr;

    AllocationBlockFooter * newfoot = newhead->Footer();
    newfoot->Magic = 0xFEA7EFA1;
    newfoot->Header = newhead;

    this->_insert(newhead);
    
    this->m_iEnd += 4096;
}

void Memory::Heap::_insert(Memory::AllocationBlockHeader * newhead)
{
    if (this->m_pBiggest == nullptr || this->m_pSmallest == nullptr)
    {
        if (this->m_pSmallest != nullptr || this->m_pBiggest != nullptr)
        {
            // I'm yet to write anything like "PANIC()"...
            //            PANIC("Critical heap error: Biggest == nullptr, but Smallest != nullptr!");
            __asm("hlt");
        }
        this->m_pBiggest = newhead;
        this->m_pSmallest = newhead;

        return;
    }
    
    if ((uint64)newhead->Previous() >= this->m_iStart && newhead->Previous()->Flags & 1 == 0)
    {
        newhead->Previous()->Size += newhead->Size + sizeof(AllocationBlockHeader) + sizeof(AllocationBlockFooter);
        newhead->Footer()->Header = newhead->Previous();

        newhead = newhead->Previous();

        if (newhead->Bigger != nullptr)
        {
            newhead->Bigger->Smaller = newhead->Smaller;
        }

        else
        {
            this->m_pBiggest = newhead->Smaller;
        }

        if (newhead->Smaller != nullptr)
        {
            newhead->Smaller->Bigger = newhead->Bigger;
        }

        else
        {
            this->m_pSmallest = newhead->Bigger;
        }

        if ((uint64)newhead->Footer() + sizeof(AllocationBlockFooter) == this->m_iEnd &&
            (uint64)this->m_iEnd - (uint64)newhead + sizeof(AllocationBlockHeader) >= 4096)
        {
            
        }

        this->_insert(newhead);
        return;
    }

    if ((uint64)newhead->Next()->Footer() + sizeof(AllocationBlockFooter) <= (uint64)this->m_iEnd &&
        newhead->Next()->Flags & 1 == 0)
    {
        newhead->Size += newhead->Next()->Size + sizeof(AllocationBlockHeader) + sizeof(AllocationBlockFooter);
        newhead->Next()->Footer()->Header = newhead;
        
        if (newhead->Bigger != nullptr)
        {
            newhead->Bigger->Smaller = newhead->Smaller;
        }
        
        else
        {
            this->m_pBiggest = newhead->Smaller;
        }
        
        if (newhead->Smaller != nullptr)
        {
            newhead->Smaller->Bigger = newhead->Bigger;
        }
        
        else
        {
            this->m_pSmallest = newhead->Bigger;
        }
        
        this->_insert(newhead);
        return;
    }

    if (abs(this->m_pBiggest->Size - newhead->Size) < abs(this->m_pSmallest->Size - newhead->Size))
    {
        AllocationBlockHeader * list = this->m_pBiggest;

        while (list->Size != newhead->Size && list->Smaller != nullptr && list->Smaller->Size >= newhead->Size)
        {
            list = list->Smaller;
        }

        newhead->Smaller = list->Smaller;
        newhead->Bigger = list;
        list->Smaller->Bigger = newhead;
        list->Smaller = newhead;
    }

    else
    {
        AllocationBlockHeader * list = this->m_pSmallest;

        while (list->Size != newhead->Size && list->Bigger != nullptr && list->Bigger->Size <= newhead->Size)
        {
            list = list->Bigger;
        }

        newhead->Bigger = list->Bigger;
        newhead->Smaller = list;
        list->Bigger->Smaller = newhead;
        list->Bigger = newhead;
    }
}

void * Memory::Heap::_validate(void * pAddr, bool bShouldBeAllocated)
{
    AllocationBlockHeader * head = (AllocationBlockHeader *)((uint8 *)pAddr - sizeof(AllocationBlockHeader));
    if (head->Magic != 0xFEA7EFA1 || head->Footer()->Magic != 0xFEA7EFA1)
    {
        // PANIC("Critical heap error: invalid allocation block!");
        __asm("hlt");
    }

    if (bShouldBeAllocated && head->Flags & 1 != 1)
    {
        __asm("hlt");
    }

    return pAddr;
}
