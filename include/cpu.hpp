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

enum class ExceptionType : uint8_t
{
    LoadAddressError = 0x4,
    StoreAddressError = 0x5,
    SysCall = 0x8,
    ReservedInstruction = 0xA,
    Overflow = 0xC,
};

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

    void Exception(ExceptionType type);

    void LB(uint32_t opcode);
    void LBU(uint32_t opcode);
    void LW(uint32_t opcode);

    void SB(uint32_t opcode);
    void SH(uint32_t opcode);
    void SW(uint32_t opcode);

    void ADD(uint32_t opcode);
    void ADDU(uint32_t opcode);
    void SUBU(uint32_t opcode);
    void ADDI(uint32_t opcode);
    void ADDIU(uint32_t opcode);

    void SLT(uint32_t opcode);
    void SLTU(uint32_t opcode);
    void SLTI(uint32_t opcode);
    void SLTIU(uint32_t opcode);

    void AND(uint32_t opcode);
    void OR(uint32_t opcode);
    void ANDI(uint32_t opcode);
    void ORI(uint32_t opcode);

    void SLL(uint32_t opcode);
    void SRL(uint32_t opcode);
    void SRA(uint32_t opcode);
    void LUI(uint32_t opcode);

    void DIV(uint32_t opcode);
    void DIVU(uint32_t opcode);
    void MFHI(uint32_t opcode);
    void MFLO(uint32_t opcode);
    void MTHI(uint32_t opcode);
    void MTLO(uint32_t opcode);

    void J(uint32_t opcode);
    void JAL(uint32_t opcode);
    void JR(uint32_t opcode);
    void JALR(uint32_t opcode);
    void BEQ(uint32_t opcode);
    void BNE(uint32_t opcode);
    void BLEZ(uint32_t opcode);
    void BGTZ(uint32_t opcode);
    void Branch(uint32_t opcode);

    void SYSCALL(uint32_t opcode);

    void MTC0(uint32_t opcode);
    void MFC0(uint32_t opcode);
    void RFE(uint32_t opcode);

    void HandleCoprocessor0(uint32_t opcode);

private:
    PSX *psx;

    struct
    {
        int reg = 0;
        uint32_t value = 0;
    } load_slot;

    std::array<uint32_t, 31> regs{};
    std::array<uint32_t, 31> out_regs{};

    union
    {
        uint32_t value = 0x10900000;
        struct
        {
            uint16_t pad;
            bool isolate_cache : 1;
            uint8_t pad2 : 5;
            bool exception_vector : 1;
        };
    } sr;

    union
    {
        uint32_t value = 0x0;
        struct
        {
            uint8_t _ : 2;
            ExceptionType excode : 5;
        };
    } cause;
    uint32_t epc;

    uint32_t next_pc = 0xBFC00004;
    uint32_t pc = 0xBFC00000;
    uint32_t hi = 0x0;
    uint32_t lo = 0x0;
};