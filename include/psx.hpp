#pragma once

#include <cstdint>

#include "cpu.hpp"

class PSX
{
public:
    PSX(uint8_t *bios);
    ~PSX();

    void Run();

    uint8_t ReadMemory8(uint32_t addr);
    uint16_t ReadMemory16(uint32_t addr);
    uint32_t ReadMemory32(uint32_t addr);

    void WriteMemory8(uint32_t addr, uint8_t value);
    void WriteMemory16(uint32_t addr, uint16_t value);
    void WriteMemory32(uint32_t addr, uint32_t value);

    uint32_t MirrorAddress(uint32_t addr);

private:
    uint8_t *bios;
    uint8_t *ram;
    CPU *cpu;
};