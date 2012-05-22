/**
 * ReaverOS
 * loader/booter/screen.h
 * Screen drawing routines.
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

#include "screen.h"
#include "memory.h"
#include "paging.h"
#include "processor.h"

extern "C"
{
    void _reload_cr3(uint32);
}

namespace Screen
{
    OutputStream * bout = 0;
    const char * nl = "\n";
    const char * tab = "\t";
}

void Screen::Initialize(VideoMode * pVideoMode, void * pFont)
{
    VideoModeWrapper * pVideoModeWrapper = (VideoModeWrapper *)Memory::Place(sizeof(VideoModeWrapper));
    pVideoModeWrapper->Initialize(pFont, pVideoMode);
    Screen::bout = (OutputStream *)Memory::Place(sizeof(OutputStream));
    Screen::bout->Initialize(pVideoModeWrapper);
}

uint8 OutputStream::Base(uint8 base)
{
    if (!(base < 2 || base > 32))
    {
        this->m_iBase = base;
    }
    
    return this->m_iBase;
}

void OutputStream::Initialize(VideoModeWrapper * pVideoMode)
{
    this->m_iBase = 10;
    this->m_pVideoMode = pVideoMode;
}

uint64 Screen::SaveProcessedVideoModeDescription(uint64 pDestAddress)
{
    uint64 p = Memory::iFirstFreePageAddress;
    Memory::Map(pDestAddress, pDestAddress + 4096, Memory::iFirstFreePageAddress);
    Memory::Map(0x8000000, 0x8000000 + 4096, p);

    KernelVideoMode * kvideo = (KernelVideoMode *)0x8000000;

    kvideo->PhysBasePtr = Screen::bout->m_pVideoMode->m_pVideoMode->PhysBasePtr;
    kvideo->BytesPerScanLine = Screen::bout->m_pVideoMode->m_pVideoMode->LinearBytesPerScanLine;
    kvideo->XResolution = Screen::bout->m_pVideoMode->m_pVideoMode->XResolution;
    kvideo->YResolution = Screen::bout->m_pVideoMode->m_pVideoMode->YResolution;
    kvideo->BitsPerPixel = Screen::bout->m_pVideoMode->m_pVideoMode->BitsPerPixel;
    kvideo->RedMaskSize = Screen::bout->m_pVideoMode->m_pVideoMode->RedMaskSize;
    kvideo->RedFieldPosition = Screen::bout->m_pVideoMode->m_pVideoMode->RedFieldPosition;
    kvideo->GreenMaskSize = Screen::bout->m_pVideoMode->m_pVideoMode->GreenMaskSize;
    kvideo->GreenFieldPosition = Screen::bout->m_pVideoMode->m_pVideoMode->GreenFieldPosition;
    kvideo->BlueMaskSize = Screen::bout->m_pVideoMode->m_pVideoMode->BlueMaskSize;
    kvideo->BlueFieldPosition = Screen::bout->m_pVideoMode->m_pVideoMode->BlueFieldPosition;
    kvideo->ReservedMaskSize = Screen::bout->m_pVideoMode->m_pVideoMode->RsvdMaskSize;
    kvideo->ReservedFieldPosition = Screen::bout->m_pVideoMode->m_pVideoMode->RsvdFieldPosition;

    return pDestAddress + 4096;
}

void VideoModeWrapper::Initialize(void * pFont, VideoMode * pVideoMode)
{
    this->m_pVideoMode = pVideoMode;

    if (this->m_pVideoMode->LinearBlueFieldPosition == 0 && this->m_pVideoMode->LinearGreenFieldPosition == 0
            && this->m_pVideoMode->LinearRedFieldPosition == 0 && this->m_pVideoMode->LinearBytesPerScanLine == 0)
    {
        this->m_pVideoMode->LinearRedFieldPosition = this->m_pVideoMode->RedFieldPosition;
        this->m_pVideoMode->LinearGreenFieldPosition = this->m_pVideoMode->GreenFieldPosition;
        this->m_pVideoMode->LinearBlueFieldPosition = this->m_pVideoMode->BlueFieldPosition;
        this->m_pVideoMode->LinearRedMaskSize = this->m_pVideoMode->RedMaskSize;
        this->m_pVideoMode->LinearGreenMaskSize = this->m_pVideoMode->GreenMaskSize;
        this->m_pVideoMode->LinearBlueMaskSize = this->m_pVideoMode->BlueMaskSize;
        this->m_pVideoMode->LinearBytesPerScanLine = this->m_pVideoMode->BytesPerScanLine;
    }

    this->maxx = this->m_pVideoMode->XResolution / 8;
    this->maxy = this->m_pVideoMode->YResolution / 16;

    this->x = 0;
    this->y = 0;

    this->r = this->g = this->b = 0xc0;
    this->_ = 0;

    this->m_pFontData = pFont;
}

OutputStream & operator << (OutputStream & s, const char * str)
{
    while (*str)
    {
        s.m_pVideoMode->PrintCharacter(*str++);
    }

    return s;
}

void VideoModeWrapper::PrintCharacter(char c)
{
    if (c == 0)
    {
        return;
    }

    if (c == '\n')
    {
        this->y++;
        this->x = 0;
        return;
    }

    if (c == '\t')
    {
        this->x += (8 - this->x % 8);
        return;
    }

    switch (this->m_pVideoMode->BitsPerPixel)
    {
        case 16:
        {
            this->_put16(c);
            return;
        }

        case 32:
        {
            this->_put32(c);
            return;
        }
    }
}

void VideoModeWrapper::_put16(char c)
{
    uint8 * character = &((uint8 *)this->m_pFontData)[c * 16];
    uint16 * dest = (uint16 *)(this->m_pVideoMode->PhysBasePtr + this->y * this->m_pVideoMode->LinearBytesPerScanLine * 16
                    + this->x * this->m_pVideoMode->BitsPerPixel);

    uint16 iColor = ((this->r >> (8 - this->m_pVideoMode->LinearRedMaskSize)) << this->m_pVideoMode->LinearRedFieldPosition) |
                ((this->g >> (8 - this->m_pVideoMode->LinearGreenMaskSize)) << this->m_pVideoMode->LinearGreenFieldPosition) |
                ((this->b >> (8 - this->m_pVideoMode->LinearBlueMaskSize)) << this->m_pVideoMode->LinearBlueFieldPosition);

    uint16 iBgcolor = 0;
                    
    for (int i = 0; i < 16; i++)
    {
        uint8 data = character[i];

        for (uint8 i = 0; i < 8; i++)
        {
            dest[i] = (data >> (7 - i)) & 1 ? iColor : iBgcolor;
        }

        uint32 _ = (uint32)dest;
        _ += this->m_pVideoMode->BytesPerScanLine ;
        dest = (uint16 *)_;
    }

    this->x++;

    if (this->x > this->maxx)
    {
        this->x = 0;
        this->y++;
    }
}

void VideoModeWrapper::_put32(char c)
{
    uint8 * character = &((uint8 *)this->m_pFontData)[c * 16];
    uint32 * dest = (uint32 *)(this->m_pVideoMode->PhysBasePtr + this->y * this->m_pVideoMode->LinearBytesPerScanLine * 16
                    + this->x * this->m_pVideoMode->BitsPerPixel);

    uint32 iColor = (this->r << this->m_pVideoMode->LinearRedFieldPosition) |
                    (this->g << this->m_pVideoMode->LinearGreenFieldPosition) |
                    (this->b << this->m_pVideoMode->LinearBlueFieldPosition);

    uint32 iBgcolor = 0;
    
    for (int i = 0; i < 16; i++)
    {
        uint8 data = character[i];

        for (uint8 i = 0; i < 8; i++)
        {
            dest[i] = (data >> (7 - i)) & 1 ? iColor : iBgcolor;
        }

        uint32 _ = (uint32)dest;
        _ += this->m_pVideoMode->BytesPerScanLine;
        dest = (uint32 *)_;
    }

    this->x++;
    
    if (this->x > this->maxx)
    {
        this->x = 0;
        this->y++;
    }
}

void OutputStream::UpdatePagingStructures()
{    
    uint64 vidmem = this->m_pVideoMode->m_pVideoMode->PhysBasePtr;
    uint64 vidmemsize = this->m_pVideoMode->m_pVideoMode->YResolution *
            this->m_pVideoMode->m_pVideoMode->LinearBytesPerScanLine;

    Memory::Map(vidmem, vidmem + vidmemsize, vidmem, true);
}