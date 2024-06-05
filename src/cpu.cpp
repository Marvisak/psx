#include "cpu.hpp"
#include "psx.hpp"

#include "spdlog/spdlog.h"

#ifdef WIN32
#include <intrin.h>
#endif

CPU::CPU(PSX *psx) : psx(psx)
{
}

void CPU::RunInstruction()
{
    uint32_t opcode = psx->ReadMemory32(pc);

    pc = next_pc;
    next_pc += 4;

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
    case 0x1:
        Branch(opcode);
        break;
    case 0x2:
        J(opcode);
        break;
    case 0x3:
        JAL(opcode);
        break;
    case 0x4:
        BEQ(opcode);
        break;
    case 0x5:
        BNE(opcode);
        break;
    case 0x6:
        BLEZ(opcode);
        break;
    case 0x7:
        BGTZ(opcode);
        break;
    case 0x8:
        ADDI(opcode);
        break;
    case 0x9:
        ADDIU(opcode);
        break;
    case 0xA:
        SLTI(opcode);
        break;
    case 0xB:
        SLTIU(opcode);
        break;
    case 0xC:
        ANDI(opcode);
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
    case 0x20:
        LB(opcode);
        break;
    case 0x23:
        LW(opcode);
        break;
    case 0x24:
        LBU(opcode);
        break;
    case 0x28:
        SB(opcode);
        break;
    case 0x29:
        SH(opcode);
        break;
    case 0x2B:
        SW(opcode);
        break;
    default:
        Exception(ExceptionType::ReservedInstruction);
        spdlog::error("Unknown instruction exception: {:08X}", opcode);
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
    case 0x2:
        SRL(opcode);
        break;
    case 0x3:
        SRA(opcode);
        break;
    case 0x8:
        JR(opcode);
        break;
    case 0x9:
        JALR(opcode);
        break;
    case 0xC:
        SYSCALL(opcode);
        break;
    case 0x10:
        MFHI(opcode);
        break;
    case 0x11:
        MTHI(opcode);
        break;
    case 0x12:
        MFLO(opcode);
        break;
    case 0x13:
        MTLO(opcode);
        break;
    case 0x1A:
        DIV(opcode);
        break;
    case 0x1B:
        DIVU(opcode);
        break;
    case 0x20:
        ADD(opcode);
        break;
    case 0x21:
        ADDU(opcode);
        break;
    case 0x23:
        SUBU(opcode);
        break;
    case 0x24:
        AND(opcode);
        break;
    case 0x25:
        OR(opcode);
        break;
    case 0x2A:
        SLT(opcode);
        break;
    case 0x2B:
        SLTU(opcode);
        break;
    default:
        Exception(ExceptionType::ReservedInstruction);
        spdlog::error("Unknown instruction exception: {:08X}", opcode);
        exit(1);
        break;
    }
}

void CPU::Exception(ExceptionType type)
{
    uint32_t vector = sr.exception_vector ? 0xBFC00180 : 0x80000080;

    uint8_t mode = sr.value & 0x3F;
    sr.value &= ~0x3f;
    sr.value |= (mode << 2) & 0x3F;

    cause.excode = type;

    epc = next_pc;
    pc = vector;
    next_pc = pc + 4;
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

void CPU::LB(uint32_t opcode)
{
    if (sr.isolate_cache)
    {
        spdlog::debug("Cache Isolate enabled, ignoring read");
        return;
    }

    uint32_t addr = (int16_t)IMM16(opcode) + GetRegister(RS(opcode));
    load_slot.reg = RT(opcode);
    load_slot.value = (int8_t)psx->ReadMemory8(addr);
}

void CPU::LBU(uint32_t opcode)
{
    if (sr.isolate_cache)
    {
        spdlog::debug("Cache Isolate enabled, ignoring read");
        return;
    }

    uint32_t addr = (int16_t)IMM16(opcode) + GetRegister(RS(opcode));
    load_slot.reg = RT(opcode);
    load_slot.value = psx->ReadMemory8(addr);
}

void CPU::LW(uint32_t opcode)
{
    if (sr.isolate_cache)
    {
        spdlog::debug("Cache Isolate enabled, ignoring read");
        return;
    }

    uint32_t addr = (int16_t)IMM16(opcode) + GetRegister(RS(opcode));
    load_slot.reg = RT(opcode);
    load_slot.value = psx->ReadMemory32(addr);
}

void CPU::SB(uint32_t opcode)
{
    if (sr.isolate_cache)
    {
        spdlog::debug("Cache Isolate enabled, ignoring write");
        return;
    }
    uint32_t addr = (int16_t)IMM16(opcode) + GetRegister(RS(opcode));
    uint32_t value = GetRegister(RT(opcode)) & 0xFF;
    psx->WriteMemory8(addr, value);
}

void CPU::SH(uint32_t opcode)
{
    if (sr.isolate_cache)
    {
        spdlog::debug("Cache Isolate enabled, ignoring write");
        return;
    }
    uint32_t addr = (int16_t)IMM16(opcode) + GetRegister(RS(opcode));
    uint32_t value = GetRegister(RT(opcode)) & 0xFFFF;
    psx->WriteMemory16(addr, value);
}

void CPU::SW(uint32_t opcode)
{
    if (sr.isolate_cache)
    {
        spdlog::debug("Cache Isolate enabled, ignoring write");
        return;
    }
    uint32_t addr = (int16_t)IMM16(opcode) + GetRegister(RS(opcode));
    uint32_t value = GetRegister(RT(opcode));
    psx->WriteMemory32(addr, value);
}

void CPU::ADD(uint32_t opcode)
{
#ifdef WIN32
    uint32_t value;
    if (_addcarry_u32(0, GetRegister(RS(opcode)), GetRegister(RT(opcode)), &value))
#else
    uint32_t value;
    if (__builtin_add_overflow(GetRegister(RS(opcode)), IMM16(opcode), &value))
#endif
    {
        spdlog::error("ADD overflow");
        Exception(ExceptionType::Overflow);
    }
    SetRegister(RD(opcode), value);
}

void CPU::ADDU(uint32_t opcode)
{
    uint32_t value = GetRegister(RS(opcode)) + GetRegister(RT(opcode));
    SetRegister(RD(opcode), value);
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
        spdlog::error("ADDI overflow");
        Exception(ExceptionType::Overflow);
    }
    SetRegister(RT(opcode), value);
}

void CPU::SUBU(uint32_t opcode)
{
    uint32_t value = GetRegister(RS(opcode)) - GetRegister(RT(opcode));
    SetRegister(RD(opcode), value);
}

void CPU::ADDIU(uint32_t opcode)
{
    uint32_t value = GetRegister(RS(opcode)) + (int16_t)IMM16(opcode);
    SetRegister(RT(opcode), value);
}

void CPU::SLT(uint32_t opcode)
{
    bool value = (int32_t)GetRegister(RS(opcode)) < (int32_t)GetRegister(RT(opcode));
    SetRegister(RD(opcode), value);
}

void CPU::SLTU(uint32_t opcode)
{
    bool value = GetRegister(RS(opcode)) < GetRegister(RT(opcode));
    SetRegister(RD(opcode), value);
}

void CPU::SLTI(uint32_t opcode)
{
    bool value = (int32_t)GetRegister(RS(opcode)) < (int16_t)IMM16(opcode);
    SetRegister(RT(opcode), value);
}

void CPU::SLTIU(uint32_t opcode)
{
    bool value = GetRegister(RS(opcode)) < IMM16(opcode);
    SetRegister(RT(opcode), value);
}

void CPU::AND(uint32_t opcode)
{
    uint32_t value = GetRegister(RS(opcode)) & GetRegister(RT((opcode)));
    SetRegister(RD(opcode), value);
}

void CPU::OR(uint32_t opcode)
{
    uint32_t value = GetRegister(RS(opcode)) | GetRegister(RT((opcode)));
    SetRegister(RD(opcode), value);
}

void CPU::ANDI(uint32_t opcode)
{
    uint32_t value = GetRegister(RS(opcode)) & IMM16(opcode);
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

void CPU::SRL(uint32_t opcode)
{
    uint32_t value = GetRegister(RT(opcode)) >> IMM5(opcode);
    SetRegister(RD(opcode), value);
}

void CPU::SRA(uint32_t opcode)
{
    uint32_t value = (int32_t)GetRegister(RT(opcode)) >> IMM5(opcode);
    SetRegister(RD(opcode), value);
}

void CPU::LUI(uint32_t opcode)
{
    uint32_t value = IMM16(opcode) << 16;
    SetRegister(RT(opcode), value);
}

void CPU::DIV(uint32_t opcode)
{
    // TODO: Handle timing
    auto n = (int32_t)GetRegister(RS(opcode));
    auto d = (int32_t)GetRegister(RT(opcode));
    if (d == 0)
    {
        hi = d;
        if (n >= 0)
            lo = 0xFFFFFFFF;
        else
            lo = 1;
    }
    else if (n == 0x80000000 && d == -1)
    {
        hi = 0;
        lo = 0x80000000;
    }
    else
    {
        hi = n % d;
        lo = n / d;
    }
}

void CPU::DIVU(uint32_t opcode)
{
    uint32_t n = GetRegister(RS(opcode));
    uint32_t d = GetRegister(RT(opcode));

    if (d == 0)
    {
        hi = n;
        lo = 0xFFFFFFFF;
    }
    else
    {
        hi = n % d;
        lo = n / d;
    }
}

void CPU::MFHI(uint32_t opcode)
{
    // TODO: Handle stalling
    SetRegister(RD(opcode), hi);
}

void CPU::MFLO(uint32_t opcode)
{
    // TODO: Handle stalling
    SetRegister(RD(opcode), lo);
}

void CPU::MTHI(uint32_t opcode)
{
    hi = GetRegister(RS(opcode));
}

void CPU::MTLO(uint32_t opcode)
{
    lo = GetRegister(RS(opcode));
}

void CPU::J(uint32_t opcode)
{
    uint32_t addr = next_pc & 0xF0000000 | IMM26(opcode) << 2;
    next_pc = addr;
}

void CPU::JAL(uint32_t opcode)
{
    SetRegister(31, next_pc);
    uint32_t addr = next_pc & 0xF0000000 | IMM26(opcode) << 2;
    next_pc = addr;
}

void CPU::JR(uint32_t opcode)
{
    next_pc = GetRegister(RS(opcode));
}

void CPU::JALR(uint32_t opcode)
{
    SetRegister(31, next_pc);
    next_pc = GetRegister(RS(opcode));
}

void CPU::BEQ(uint32_t opcode)
{
    if (GetRegister(RS(opcode)) == GetRegister(RT(opcode)))
    {
        int16_t offset = (int16_t)IMM16(opcode) << 2;
        next_pc += offset - 4;
    }
}

void CPU::BNE(uint32_t opcode)
{
    if (GetRegister(RS(opcode)) != GetRegister(RT(opcode)))
    {
        int16_t offset = (int16_t)IMM16(opcode) << 2;
        next_pc += offset - 4;
    }
}

void CPU::BLEZ(uint32_t opcode)
{
    if ((int32_t)GetRegister(RS(opcode)) <= 0)
    {
        int16_t offset = (int16_t)IMM16(opcode) << 2;
        next_pc += offset - 4;
    }
}

void CPU::BGTZ(uint32_t opcode)
{
    if ((int32_t)GetRegister(RS(opcode)) > 0)
    {
        int16_t offset = (int16_t)IMM16(opcode) << 2;
        next_pc += offset - 4;
    }
}

void CPU::Branch(uint32_t opcode)
{
    bool bgez = opcode & 0x10000;
    bool link = opcode & 0x100000;

    int32_t reg = (int32_t)GetRegister(RS(opcode));
    bool branch = bgez ? reg >= 0 : reg < 0;

    if (link)
        SetRegister(31, pc);

    if (branch)
    {
        int16_t offset = (int16_t)IMM16(opcode) << 2;
        next_pc += offset - 4;
    }
}

void CPU::SYSCALL(uint32_t opcode)
{
    Exception(ExceptionType::SysCall);
}

void CPU::MTC0(uint32_t opcode)
{
    uint32_t value = GetRegister(RT(opcode));
    if (RD(opcode) == 12)
    {
        if (value != 0x10000 && value != 0x0)
        {
            spdlog::error("SR unknown value: {:08X}", value);
            exit(0);
        }
        sr.value = value;
    }
    else if (value != 0)
    {
        spdlog::error("Unhandled COP0 register write to {:08X} with value {:08X}", RD(opcode), value);
    }
}

void CPU::MFC0(uint32_t opcode)
{
    load_slot.reg = RT(opcode);
    if (RD(opcode) == 12)
    {
        load_slot.value = sr.value;
    }
    else if (RD(opcode) == 13)
    {
        load_slot.value = cause.value;
    }
    else if (RD(opcode) == 14)
    {
        load_slot.value = epc;
    }
    else
    {
        spdlog::error("Unhandled COP0 register read from {:08X} to {:08X}", RD(opcode), RT(opcode));
        exit(0);
    }
}

void CPU::RFE(uint32_t opcode)
{
    uint8_t mode = sr.value & 0x3F;
    sr.value &= !0x3F;
    sr.value |= mode >> 2;
}

void CPU::HandleCoprocessor0(uint32_t opcode)
{
    switch (COP(opcode))
    {
    case 0x0:
        MFC0(opcode);
        break;
    case 0x4:
        MTC0(opcode);
        break;
    case 0x10:
        RFE(opcode);
        break;
    default:
        spdlog::error("Unknown coprocessor instruction exception: {:08X}", opcode);
        break;
    }
}