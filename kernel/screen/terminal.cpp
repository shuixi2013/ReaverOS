/**
 * ReaverOS
 * kernel/screen/terminal.cpp
 * Terminal implementation..
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

#include "terminal.h"

Screen::Terminal::Terminal(Screen::TerminalDriver * pDrv, const Lib::String & sName)
            : m_sName(sName), m_pDriver(pDrv)
{
}

Screen::Terminal::~Terminal()
{
    delete m_pDriver;
}

void Screen::Terminal::ScrollUp()
{
    this->m_pDriver->ScrollUp();
}

void Screen::Terminal::ScrollDown()
{
    this->m_pDriver->ScrollDown();
}

Screen::ReaverTerminal::ReaverTerminal(Screen::TerminalDriver * pDrv, const Lib::String & sName)
        : Terminal(pDrv, sName)
{
}

Screen::ReaverTerminal::~ReaverTerminal()
{
}

void Screen::ReaverTerminal::Print(const Lib::String & sString)
{

}

Screen::BootTerminal::BootTerminal(Screen::VideoMode * pVideoMode)
    : Terminal(0)
{

}

Screen::BootTerminal::~BootTerminal()
{
}

void Screen::BootTerminal::Print(const Lib::String & )
{

}
