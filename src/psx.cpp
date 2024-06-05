#include "psx.hpp"

#include "spdlog/spdlog.h"

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
    else if (addr >= 0x1F000000 && addr <= 0x1F800000)
    {
        return 0xFF;
    }
    else if (addr >= 0x1FC00000 && addr <= 0x1FC80000)
    {
        return bios[addr - 0x1FC00000];
    }
    else
    {
        spdlog::error("Unknown memory read from {:08X}", addr);
        exit(0);
        return 0;
    }
}

uint16_t PSX::ReadMemory16(uint32_t addr)
{
    if (addr % 2 != 0)
    {
        // TODO: Add exception
        spdlog::error("Memory is not alligned {:08X}", addr);
        exit(0);
        return 0;
    }

    return ReadMemory8(addr + 1) << 8 | ReadMemory8(addr);
}

uint32_t PSX::ReadMemory32(uint32_t addr)
{
    if (addr % 4 != 0)
    {
        // TODO: Add exception
        spdlog::error("Memory is not alligned {:08X}", addr);
        exit(0);
        return 0;
    }

    if (addr == 0x1F801070)
    {
        return 0x0;
    }
    else if (addr == 0x1F801074)
    {
        return 0x0;
    }
    else
    {
        return ReadMemory16(addr + 2) << 16 | ReadMemory16(addr);
    }
}

void PSX::WriteMemory8(uint32_t addr, uint8_t value)
{
    addr = MirrorAddress(addr);
    if (addr <= 0x200000)
    {
        ram[addr] = value;
    }
    else if (addr == 0x1F802041)
    {
        spdlog::info("BIOS Boot Progress: {:X}", value);
    }
    else
    {
        spdlog::error("Unknown memory write to {:08X} with value {:02X}", addr, value);
        exit(0);
    }
}

void PSX::WriteMemory16(uint32_t addr, uint16_t value)
{
    if (addr % 2 != 0)
    {
        // TODO: Add exception
        spdlog::error("Memory is not alligned {:04X}", addr);
        exit(0);
        return;
    }

    if (addr >= 0x1F801100 && addr <= 0x1F801128)
    {
        spdlog::warn("Unimplemented Timer Register: {:08X}", addr);
    }
    else if (addr >= 0x1F801D80 && addr <= 0x1F801D86)
    {
        spdlog::warn("Unimplemented SPU Register: {:08X}", addr);
    }
    else
    {
        WriteMemory8(addr, value & 0xFF);
        WriteMemory8(addr + 1, value >> 8);
    }
}

void PSX::WriteMemory32(uint32_t addr, uint32_t value)
{
    if (addr % 4 != 0)
    {
        // TODO: Add exception
        spdlog::error("Memory is not alligned {:08X}", addr);
        exit(0);
        return;
    }

    if (addr >= 0x1F801000 && addr <= 0x1F801060)
    {
        spdlog::warn("Unimplemented Memory Control Register: {:08X}", addr);
    }
    else if (addr == 0x1F801070)
    {
        spdlog::warn("Unimplemented IRQ Stat Register: {:08X}", addr);
    }
    else if (addr == 0x1F801074)
    {
        spdlog::warn("Unimplemented IRQ Mask Register: {:08X}", addr);
    }
    else if (addr == 0xFFFE0130)
    {
        spdlog::warn("Unimplemented Cache Control Register: {:08X}", addr);
    }
    else
    {
        WriteMemory16(addr, value & 0xFFFF);
        WriteMemory16(addr + 2, value >> 16);
    }
}

uint32_t PSX::MirrorAddress(uint32_t addr)
{
    int index = addr >> 29;
    if (index > 1 && index < 4)
    {
        // TODO: Add exception
        spdlog::error("Attempted to access forbidden part of KUSEG");
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