#include "psx.hpp"

#include <iostream>

PSX::PSX(uint8_t *bios) : bios(bios)
{
    cpu = new CPU(this);
}

PSX::~PSX()
{
    delete cpu;
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
    if (addr >= 0xBFC00000 && addr <= 0xBFC7D000)
    {
        return bios[addr - 0xBFC00000];
    }
    else
    {
        std::cerr << std::hex << "Unknown memory read from " << addr << std::endl;
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
    if (false)
    {
    }
    else
    {
        std::cerr << std::hex << "Unknown memory write to " << addr << " with value " << (int)value << std::endl;
    }
}

void PSX::WriteMemory16(uint32_t addr, uint16_t value)
{
    WriteMemory8(addr, value >> 8);
    WriteMemory8(addr + 1, value & 0xFF);
}

void PSX::WriteMemory32(uint32_t addr, uint32_t value)
{
    if (addr % 4 != 0)
    {
        // TODO: Add exception
        std::cerr << std::hex << "Memory is not alligned " << addr << std::endl;
        return;
    }

    if (addr == 0x1F801010)
    {
        std::cerr << "Unimplemented register: BIOS ROM Delay/Size" << std::endl;
    }
    else if (addr == 0x1F801060)
    {
        std::cerr << "Unimplemented register: RAM_SIZE" << std::endl;
    }
    else
    {
        WriteMemory16(addr, value >> 16);
        WriteMemory16(addr + 2, value & 0xFFFF);
    }
}