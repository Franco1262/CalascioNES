#include <cstdint>
#include <iostream>
#include <vector>
#include <fstream>
#include <memory>
#include "nlohmann/json.hpp"



using json = nlohmann::json;

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
        void reset();
        uint8_t get_opcode();

    private:

        uint8_t opcode;
        json json_data;
        std::shared_ptr<Bus> bus;

        //Registers
        uint8_t Accumulator;
        uint8_t X;
        uint8_t Y;
        uint16_t PC; //Program counter
        uint8_t SP; //Stack pointer
        uint8_t P; //Status register
        uint8_t OAMDMA;

        bool oamdma_flag;
        bool odd_cycle;
        int cycles_dma;
        bool alignment_needed;
        uint8_t dma_read;
        uint16_t dma_address;
       
        
        uint8_t memory[0x800]; //2kb ram internal to cpu


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

        void LDA();
        void LDX();
        void LDY();
        void STA();
        void STX();
        void STY();
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

        void DEC();
        void DEX();
        void DEY();
        void INC();
        void INX();
        void INY();

        //Arithmetic operations

        void ADC();
        void SBC();

        //Logical operations

        void AND();
        void EOR();
        void ORA();

        //Shift &CPU:: Rotate Instructions

        void ASL();
        void LSR();
        void ROL();
        void ROR();

        //Comparison instructions

        void CMP();
        void CPX();
        void CPY();

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

        void JMP();
        void JSR();
        void RTS();

        //Interrupts

        void BRK();
        void RTI();

        //Other

        void BIT();
        void NOP();
        void XXX();

        //Useful variables

        //Jump address for JMP instruction
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