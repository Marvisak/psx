#include "psx.hpp"

#include <iostream>

PSX::PSX(uint8_t *bios) : bios(bios)
{
    cpu = new CPU(this);
    ram = new uint8_t[0x200000];
}

PSX::~PSX()
{
    delete cpu;
    delete[] ram;
}

void PSX::Run()
{
    while (true)
    {
        cpu->RunInstruction();
    }
}

uint8_t PSX::ReadMemory8(uint32_t addr)
{
    addr = MirrorAddress(addr);
    if (addr <= 0x200000)
    {
        return ram[addr];
    }
    else if (addr >= 0x1FC00000 && addr <= 0x1FC7D000)
    {
        return bios[addr - 0x1FC00000];
    }
    else
    {
        std::cerr << std::hex << "Unknown memory read from " << addr << std::endl;
        exit(0);
        return 0;
    }
}

uint16_t PSX::ReadMemory16(uint32_t addr)
{
    return ReadMemory8(addr + 1) << 8 | ReadMemory8(addr);
}

uint32_t PSX::ReadMemory32(uint32_t addr)
{
    if (addr % 4 != 0)
    {
        // TODO: Add exception
        std::cerr << std::hex << "Memory is not alligned " << addr << std::endl;
        return 0;
    }
    return ReadMemory16(addr + 2) << 16 | ReadMemory16(addr);
}

void PSX::WriteMemory8(uint32_t addr, uint8_t value)
{
    addr = MirrorAddress(addr);
    if (addr <= 0x1F4000)
    {
        ram[addr] = value;
    }
    else
    {
        std::cerr << std::hex << "Unknown memory write to " << addr << " with value " << (int)value << std::endl;
        exit(0);
    }
}

void PSX::WriteMemory16(uint32_t addr, uint16_t value)
{
    if (addr % 2 != 0)
    {
        // TODO: Add exception
        std::cerr << std::hex << "Memory is not alligned " << addr << std::endl;
        return;
    }

    if (addr >= 0x1F801D80 && addr <= 0x1F801D86)
    {
        std::cerr << "Unimplemented SPU Register: " << addr << std::endl;
    }
    else
    {
        WriteMemory8(addr, value >> 8);
        WriteMemory8(addr + 1, value & 0xFF);
    }
}

void PSX::WriteMemory32(uint32_t addr, uint32_t value)
{
    if (addr % 4 != 0)
    {
        // TODO: Add exception
        std::cerr << std::hex << "Memory is not alligned " << addr << std::endl;
        return;
    }

    if (addr >= 0x1F801000 && addr <= 0x1F801060)
    {
        std::cerr << "Unimplemented Memory Control Register: " << std::hex << addr << std::endl;
    }
    if (addr >= 0x1F801000 && addr <= 0x1F801060)
    {
        std::cerr << "Unimplemented Memory Control Register: " << std::hex << addr << std::endl;
    }
    else if (addr == 0xFFFE0130)
    {
        std::cerr << "Unimplemented Cache Control Register" << std::endl;
    }
    else
    {
        WriteMemory16(addr, value >> 16);
        WriteMemory16(addr + 2, value & 0xFFFF);
    }
}

uint32_t PSX::MirrorAddress(uint32_t addr)
{
    int index = addr >> 29;
    if (index > 1 && index < 4)
    {
        // TODO: Add exception
        std::cerr << "Attempted to access forbidden part of KUSEG" << std::endl;
        exit(0);
    }
    else if (index == 4)
    {
        return addr & 0x7FFFFFFF;
    }
    else if (index == 5)
    {
        return addr & 0x1FFFFFFF;
    }
    return addr;
}