#include "cpu.hpp"

#include "psx.hpp"

#include <iostream>

#ifdef WIN32
#include <intrin.h>
#endif

CPU::CPU(PSX *psx) : psx(psx)
{
}

void CPU::RunInstruction()
{
    uint32_t opcode = next_instruction;
    next_instruction = psx->ReadMemory32(pc);

    pc += 4;

    SetRegister(load_slot.reg, load_slot.value);
    load_slot.reg = 0;

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
    case 0x2:
        J(opcode);
        break;
    case 0x5:
        BNE(opcode);
        break;
    case 0x8:
        ADDI(opcode);
        break;
    case 0x9:
        ADDIU(opcode);
        break;
    case 0xD:
        ORI(opcode);
        break;
    case 0xF:
        LUI(opcode);
        break;
    case 0x10:
        HandleCoprocessor0(opcode);
        break;
    case 0x23:
        LW(opcode);
        break;
    case 0x29:
        SH(opcode);
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
    case 0x21:
        ADDU(opcode);
        break;
    case 0x25:
        OR(opcode);
        break;
    case 0x2B:
        SLTU(opcode);
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

void CPU::LW(uint32_t opcode)
{
    if (sr.isolate_cache)
    {
        std::cerr << "Cache Isolate enabled, ignoring read" << std::endl;
        return;
    }

    uint32_t addr = (int16_t)IMM16(opcode) + GetRegister(RS(opcode));

    load_slot.reg = RT(opcode);
    load_slot.value = psx->ReadMemory32(addr);
}

void CPU::SW(uint32_t opcode)
{
    if (sr.isolate_cache)
    {
        std::cerr << "Cache Isolate enabled, ignoring write" << std::endl;
        return;
    }
    uint32_t addr = (int16_t)IMM16(opcode) + GetRegister(RS(opcode));
    uint32_t value = GetRegister(RT(opcode));
    psx->WriteMemory32(addr, value);
}

void CPU::SH(uint32_t opcode)
{
    if (sr.isolate_cache)
    {
        std::cerr << "Cache Isolate enabled, ignoring write" << std::endl;
        return;
    }
    uint32_t addr = (int16_t)IMM16(opcode) + GetRegister(RS(opcode));
    uint32_t value = GetRegister(RT(opcode)) & 0xFFFF;
    psx->WriteMemory16(addr, value);
}

void CPU::ADDU(uint32_t opcode)
{
    uint32_t value = GetRegister(RS(opcode)) + IMM16(opcode);
    SetRegister(RT(opcode), value);
}

void CPU::ADDI(uint32_t opcode)
{
#ifdef WIN32
    int32_t value;
    if (_add_overflow_i32(0, GetRegister(RS(opcode)), (int16_t)IMM16(opcode), &value))
#else
    uint32_t value;
    if (__builtin_add_overflow(GetRegister(RS(opcode)), (int16_t)IMM16(opcode), &value))
#endif
    {
        // TODO: Add exception
        std::cerr << "ADDI overflow" << std::endl;
        exit(1);
    }
    SetRegister(RT(opcode), value);
}

void CPU::ADDIU(uint32_t opcode)
{
    uint32_t value = GetRegister(RS(opcode)) + (int16_t)IMM16(opcode);
    SetRegister(RT(opcode), value);
}

void CPU::SLTU(uint32_t opcode)
{
    bool value = GetRegister(RS(opcode)) < GetRegister(RT(opcode));
    SetRegister(RD(opcode), value);
}

void CPU::OR(uint32_t opcode)
{
    uint32_t value = GetRegister(RS(opcode)) | GetRegister(RT((opcode)));
    SetRegister(RD(opcode), value);
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

void CPU::J(uint32_t opcode)
{
    uint32_t addr = pc & 0xF0000000 | IMM26(opcode) << 2;
    pc = addr;
}

void CPU::BNE(uint32_t opcode)
{
    if (GetRegister(RS(opcode)) != GetRegister(RT(opcode)))
    {
        int16_t offset = (int16_t)IMM16(opcode) << 2;
        pc += offset - 4;
    }
}

void CPU::MTC0(uint32_t opcode)
{
    uint32_t value = GetRegister(RT(opcode));
    if (RD(opcode) == 12)
    {
        if (value != 0x10000 && value != 0x0)
        {
            std::cout << std::hex << "SR unknown value " << value << std::endl;
            exit(0);
        }
        sr.value = value;
    }
    else if (value != 0)
    {
        std::cerr << "Unhanled COP0 register write " << RD(opcode) << " with value " << value << std::endl;
    }
}

void CPU::HandleCoprocessor0(uint32_t opcode)
{
    switch (COP(opcode))
    {
    case 0x4:
        MTC0(opcode);
        break;
    default:
        std::cerr << std::hex << "Unknown coprocessor instruction exception " << opcode << std::endl;
        break;
    }
}