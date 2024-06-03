#include "cpu.hpp"

#include "psx.hpp"

#include <iostream>

CPU::CPU(PSX *psx) : psx(psx)
{
}

void CPU::RunInstruction()
{
    uint32_t opcode = psx->ReadMemory32(pc);
    pc += 4;

    uint8_t primary_opcode = opcode >> 26;

    if (primary_opcode == 0)
        RunSecondaryInstruction(opcode);
    else
        RunPrimaryInstruction(opcode);

    regs = out_regs;
}

void CPU::RunPrimaryInstruction(uint32_t opcode)
{
    switch (opcode >> 26)
    {
    case 0x9:
        ADDIU(opcode);
        break;
    case 0xD:
        ORI(opcode);
        break;
    case 0xF:
        LUI(opcode);
        break;
    case 0x2B:
        SW(opcode);
        break;
    default:
        // TODO: Add exception
        std::cerr << std::hex << "Unknown instruction exception " << opcode << std::endl;
        exit(1);
        break;
    }
}

void CPU::RunSecondaryInstruction(uint32_t opcode)
{
    switch (opcode & 0x3F)
    {
    case 0x0:
        SLL(opcode);
        break;
    default:
        // TODO: Add exception
        std::cerr << std::hex << "Unknown instruction exception " << opcode << std::endl;
        exit(1);
        break;
    }
}

void CPU::SetRegister(int index, uint32_t value)
{
    if (index == 0)
        return;
    out_regs[index - 1] = value;
}

uint32_t CPU::GetRegister(int index)
{
    if (index == 0)
        return 0;
    return regs[index - 1];
}

void CPU::SW(uint32_t opcode)
{
    uint32_t addr = (int16_t)IMM16(opcode) + GetRegister(RS(opcode));
    uint32_t value = GetRegister(RT(opcode));
    psx->WriteMemory32(addr, value);
}

void CPU::ADDIU(uint32_t opcode)
{
    uint32_t value = GetRegister(RS(opcode)) + (int16_t)IMM16(opcode);
    SetRegister(RT(opcode), value);
}

void CPU::ORI(uint32_t opcode)
{
    uint32_t value = GetRegister(RS(opcode)) | IMM16(opcode);
    SetRegister(RT(opcode), value);
}

void CPU::SLL(uint32_t opcode)
{
    uint32_t value = GetRegister(RT(opcode)) << IMM5(opcode);
    SetRegister(RD(opcode), value);
}

void CPU::LUI(uint32_t opcode)
{
    uint32_t value = IMM16(opcode) << 16;
    SetRegister(RT(opcode), value);
}