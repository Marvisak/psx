#pragma once

#include <cstdint>
#include <array>

#define IMM26(opcode) (opcode & 0x3FFFFFF)
#define IMM16(opcode) (opcode & 0xFFFF)
#define IMM5(opcode) (opcode >> 6 & 0x1F)
#define RT(opcode) (opcode >> 16 & 0x1F)
#define RD(opcode) (opcode >> 11 & 0x1F)
#define RS(opcode) (opcode >> 21 & 0x1F)
#define COP(opcode) (opcode >> 21 & 0x1F)

class PSX;
class CPU
{
public:
    CPU(PSX *psx);
    void RunInstruction();
    void RunPrimaryInstruction(uint32_t opcode);
    void RunSecondaryInstruction(uint32_t opcode);

    uint32_t GetRegister(int index);
    void SetRegister(int index, uint32_t value);

    void LW(uint32_t opcode);

    void SW(uint32_t opcode);
    void SH(uint32_t opcode);

    void ADDU(uint32_t opcode);
    void ADDI(uint32_t opcode);
    void ADDIU(uint32_t opcode);

    void SLTU(uint32_t opcode);

    void OR(uint32_t opcode);
    void ORI(uint32_t opcode);

    void SLL(uint32_t opcode);
    void LUI(uint32_t opcode);

    void J(uint32_t opcode);
    void BNE(uint32_t opcode);

    void MTC0(uint32_t opcode);

    void HandleCoprocessor0(uint32_t opcode);

private:
    PSX *psx;

    uint32_t next_instruction;

    struct
    {
        int reg;
        uint32_t value;
    } load_slot;

    std::array<uint32_t, 31> regs;
    std::array<uint32_t, 31> out_regs;

    union
    {
        uint32_t value;
        struct
        {
            uint16_t pad;
            bool isolate_cache : 1;
        };
    } sr;

    uint32_t pc = 0xBFC00000;
};