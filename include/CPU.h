#include <cstdint>
#include <iostream>
#include <vector>
#include <fstream>
#include <memory>
#include "logger.h"

class Bus;
class CPU
{
    public:

        CPU();
        ~CPU();
        void tick();
        bool open_file(std::string name);
        void load(int);
        bool finished();
        bool finish = false;
        bool compare(int line);
        void connect_bus(std::shared_ptr<Bus> bus);
        uint8_t get_opcode();

        void reset();
        void soft_reset();

    private:
        int cycles = 0;
        uint8_t opcode;
        std::shared_ptr<Bus> bus;
        bool start_logging = false;
        //Registers
        uint8_t Accumulator;
        uint8_t X;
        uint8_t Y;
        uint16_t PC; //Program counter
        uint8_t SP; //Stack pointer
        uint8_t P; //Status register
        uint8_t OAMDMA;

        bool oamdma_flag;
        bool halt_cycle = false;
        bool get_cycle;
        bool alignment_needed;
        uint8_t dma_read;
        uint16_t dma_address;
        bool reset_flag;
        bool NMI = false;
        uint16_t jmp_address;
        uint16_t rel_address;
        uint16_t subroutine_address = 0x0000;
        uint16_t zero_page_addr = 0x0000;
        uint16_t absolute_addr = 0x0000;
        uint16_t effective_addr = 0x0000;
        bool page_crossing = true;
        uint8_t n_cycles = 0;
        int8_t offset = 0x00;     
        uint8_t data;
        uint8_t high_byte;
        uint8_t low_byte;
        uint16_t h = 0x0000;
        uint16_t l = 0x0000;
        
        uint8_t memory[0x800] = {0}; //2kb ram internal to cpu

        struct Instruction
        {
            int opcode;
            void (CPU::*function)();
            int cycles;
        };

        std::vector<CPU::Instruction> Instr;

        //Flag instructions

        void SEC();
        void CLC();
        void CLV();
        void SEI();
        void CLI();
        void SED();
        void CLD();

        //Transfer instructions

        void LDA_imm();
        void LDA_zp();
        void LDA_zpx();
        void LDA_abs();
        void LDA_absx();
        void LDA_absy();
        void LDA_indx();
        void LDA_indy();

        void LDX_imm();
        void LDX_zp();
        void LDX_zpy();
        void LDX_abs();
        void LDX_absy();


        void LDY_imm();
        void LDY_zp();
        void LDY_zpx();
        void LDY_abs();
        void LDY_absx();

        void STA_zp();
        void STA_zpx();
        void STA_abs();
        void STA_absx();
        void STA_absy();
        void STA_indx();
        void STA_indy();

        void STX_abs();
        void STX_zp();
        void STX_zpy();


        void STY_zp();
        void STY_zpx();
        void STY_abs();

        void TAX();
        void TAY();
        void TXA();
        void TYA();
        void TXS();
        void TSX();

        //Stack instructions
        void PHA();
        void PHP();
        void PLA();
        void PLP();

        //Decrement and increment instructions

        void DEC_abs();
        void DEC_zp();
        void DEC_zpx();
        void DEC_absx();

        void DEX();
        void DEY();

        void INC_abs();
        void INC_zp();
        void INC_zpx();
        void INC_absx();

        void INX();
        void INY();

        //Arithmetic operations

        void ADC_imm();
        void ADC_zp();
        void ADC_zpx();
        void ADC_abs();
        void ADC_absx();
        void ADC_absy();
        void ADC_indx();
        void ADC_indy();;

        void SBC_imm();
        void SBC_zp();
        void SBC_zpx();
        void SBC_abs();
        void SBC_absx();
        void SBC_absy();
        void SBC_indx();
        void SBC_indy();

        //Logical operations

        void AND_imm();
        void AND_zp();
        void AND_zpx();
        void AND_abs();
        void AND_absx();
        void AND_absy();
        void AND_indx();
        void AND_indy();

        void EOR_imm();
        void EOR_zp();
        void EOR_zpx();
        void EOR_abs();
        void EOR_absx();
        void EOR_absy();
        void EOR_indx();
        void EOR_indy();

        void ORA_imm();
        void ORA_zp();
        void ORA_zpx();
        void ORA_abs();
        void ORA_absx();
        void ORA_absy();
        void ORA_indx();
        void ORA_indy();


        //Shift &CPU:: Rotate Instructions

        void ASL_imm();
        void ASL_zp();
        void ASL_zpx();
        void ASL_abs();
        void ASL_absx();

        void LSR_imm();
        void LSR_zp();
        void LSR_zpx();
        void LSR_abs();
        void LSR_absx();

        void ROL_imm();
        void ROL_zp();
        void ROL_zpx();
        void ROL_abs();
        void ROL_absx();

        void ROR_imm();
        void ROR_zp();
        void ROR_zpx();
        void ROR_abs();
        void ROR_absx();

        //Comparison instructions

        void CMP_imm();
        void CMP_zp();
        void CMP_zpx();
        void CMP_abs();
        void CMP_absx();
        void CMP_absy();
        void CMP_indx();
        void CMP_indy();  

        void CPX_imm();
        void CPX_zp();
        void CPX_abs();

        void CPY_imm();
        void CPY_zp();
        void CPY_abs(); 

        //Conditional Branch Instructions

        void BCC();
        void BCS();
        void BEQ();
        void BMI();
        void BNE();
        void BPL();
        void BVC();
        void BVS();

        //Jumps &CPU:: Subroutines

        void JMP_abs();
        void JMP_ind(); 

        void JSR();
        void RTS();

        //Interrupts

        void BRK();
        void RTI();

        //Other

        void BIT_zp();
        void BIT_abs();  
        void NOP();
        void XXX();


        void fetch();
        void execute_instruction(int);
        void upd_negative_zero_flags(uint8_t);

        //Addressing modes

        template <unsigned C>
        void ie_zeropage();
        template <unsigned C>
        void ie_abs();
        template <unsigned C>
        void ie_indx();
        template <unsigned C>
        void ie_absxy(uint8_t reg);
        template <unsigned C>
        void ie_zpxy(uint8_t reg);
        template <unsigned C>
        void ie_indy();

        void ADC_calc();
        void CMP_calc(uint8_t reg);

        void write(uint16_t address, uint8_t value);
        uint8_t read(uint16_t address);

        void transfer_oam_bytes();

};