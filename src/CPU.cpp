#include "CPU.h"
#include "Bus.h"
#include <sstream>
#include <iomanip>

CPU::CPU()
{
    Instr = 
    {
        {0x00, &CPU::BRK, 7 },     {0x01, &CPU::ORA_indx, 6 },    {0x02, &CPU::XXX, 2 },         {0x03, &CPU::XXX, 8},
        {0x04, &CPU::NOP, 3 },     {0x05, &CPU::ORA_zp, 3 },      {0x06, &CPU::ASL_zp, 5 },      {0x07, &CPU::XXX, 5},
        {0x08, &CPU::PHP, 3 },     {0x09, &CPU::ORA_imm, 2 },     {0x0A, &CPU::ASL_imm, 2 },     {0x0B, &CPU::XXX, 2},
        {0x0C, &CPU::NOP, 4 },     {0x0D, &CPU::ORA_abs, 4 },     {0x0E, &CPU::ASL_abs, 6 },     {0x0F, &CPU::XXX, 6},
        {0x10, &CPU::BPL, 4 },     {0x11, &CPU::ORA_indy, 6 },    {0x12, &CPU::XXX, 2 },         {0x13, &CPU::XXX, 8},
        {0x14, &CPU::NOP, 4 },     {0x15, &CPU::ORA_zpx, 4 },     {0x16, &CPU::ASL_zpx, 6 },     {0x17, &CPU::XXX, 6},
        {0x18, &CPU::CLC, 2 },     {0x19, &CPU::ORA_absy, 5 },    {0x1A, &CPU::NOP, 2 },         {0x1B, &CPU::XXX, 7},
        {0x1C, &CPU::XXX, 4 },     {0x1D, &CPU::ORA_absx, 5 },    {0x1E, &CPU::ASL_absx, 7 },    {0x1F, &CPU::XXX, 7},
        {0x20, &CPU::JSR, 6 },     {0x21, &CPU::AND_indx, 6 },    {0x22, &CPU::XXX, 2 },         {0x23, &CPU::XXX, 8},
        {0x24, &CPU::BIT_zp, 3 },  {0x25, &CPU::AND_zp, 3 },      {0x26, &CPU::ROL_zp, 5 },      {0x27, &CPU::XXX, 5},
        {0x28, &CPU::PLP, 4 },     {0x29, &CPU::AND_imm, 2 },     {0x2A, &CPU::ROL_imm, 2 },     {0x2B, &CPU::XXX, 2},
        {0x2C, &CPU::BIT_abs, 4},  {0x2D, &CPU::AND_abs, 4 },     {0x2E, &CPU::ROL_abs, 6 },     {0x2F, &CPU::XXX, 6},
        {0x30, &CPU::BMI, 4 },     {0x31, &CPU::AND_indy, 6 },    {0x32, &CPU::XXX, 2 },         {0x33, &CPU::XXX, 8},
        {0x34, &CPU::XXX, 4 },     {0x35, &CPU::AND_zpx, 4 },     {0x36, &CPU::ROL_zpx, 6 },     {0x37, &CPU::XXX, 6},
        {0x38, &CPU::SEC, 2 },     {0x39, &CPU::AND_absy, 5 },    {0x3A, &CPU::NOP, 2 },         {0x3B, &CPU::XXX, 7},
        {0x3C, &CPU::XXX, 4 },     {0x3D, &CPU::AND_absx, 5 },    {0x3E, &CPU::ROL_absx, 7 },    {0x3F, &CPU::XXX, 7},
        {0x40, &CPU::RTI, 6 },     {0x41, &CPU::EOR_indx, 6 },    {0x42, &CPU::XXX, 2 },         {0x43, &CPU::XXX, 8},
        {0x44, &CPU::NOP, 3 },     {0x45, &CPU::EOR_zp, 3 },      {0x46, &CPU::LSR_zp, 5 },      {0x47, &CPU::XXX, 5},
        {0x48, &CPU::PHA, 3 },     {0x49, &CPU::EOR_imm, 2 },     {0x4A, &CPU::LSR_imm, 2 },     {0x4B, &CPU::XXX, 2},
        {0x4C, &CPU::JMP_abs, 3 }, {0x4D, &CPU::EOR_abs, 4 },     {0x4E, &CPU::LSR_abs, 6 },     {0x4F, &CPU::XXX, 6},
        {0x50, &CPU::BVC, 4 },     {0x51, &CPU::EOR_indy, 6 },    {0x52, &CPU::XXX, 2 },         {0x53, &CPU::XXX, 8},
        {0x54, &CPU::XXX, 4 },     {0x55, &CPU::EOR_zpx, 4 },     {0x56, &CPU::LSR_zpx, 6 },     {0x57, &CPU::XXX, 6},
        {0x58, &CPU::CLI, 2 },     {0x59, &CPU::EOR_absy, 5 },    {0x5A, &CPU::NOP, 2 },         {0x5B, &CPU::XXX, 7},
        {0x5C, &CPU::XXX, 4 },     {0x5D, &CPU::EOR_absx, 5 },    {0x5E, &CPU::LSR_absx, 7 },    {0x5F, &CPU::XXX, 7},
        {0x60, &CPU::RTS, 6 },     {0x61, &CPU::ADC_indx, 6 },    {0x62, &CPU::XXX, 2 },         {0x63, &CPU::XXX, 8},
        {0x64, &CPU::XXX, 3 },     {0x65, &CPU::ADC_zp, 3 },      {0x66, &CPU::ROR_zp, 5 },      {0x67, &CPU::XXX, 5},
        {0x68, &CPU::PLA, 4 },     {0x69, &CPU::ADC_imm, 2 },     {0x6A, &CPU::ROR_imm, 2 },     {0x6B, &CPU::XXX, 2},
        {0x6C, &CPU::JMP_ind, 5 }, {0x6D, &CPU::ADC_abs, 4 },     {0x6E, &CPU::ROR_abs, 6 },     {0x6F, &CPU::XXX, 6},
        {0x70, &CPU::BVS, 4 },     {0x71, &CPU::ADC_indy, 6 },    {0x72, &CPU::XXX, 2 },         {0x73, &CPU::XXX, 8},
        {0x74, &CPU::XXX, 4 },     {0x75, &CPU::ADC_zpx, 4 },     {0x76, &CPU::ROR_zpx, 6 },     {0x77, &CPU::XXX, 6},
        {0x78, &CPU::SEI, 2 },     {0x79, &CPU::ADC_absy, 5 },    {0x7A, &CPU::NOP, 2 },         {0x7B, &CPU::XXX, 7},
        {0x7C, &CPU::XXX, 4 },     {0x7D, &CPU::ADC_absx, 5 },    {0x7E, &CPU::ROR_absx, 7 },    {0x7F, &CPU::XXX, 7},
        {0x80, &CPU::XXX, 2 },     {0x81, &CPU::STA_indx, 6 },    {0x82, &CPU::XXX, 2 },         {0x83, &CPU::XXX, 6},
        {0x84, &CPU::STY_zp, 3 },  {0x85, &CPU::STA_zp, 3 },      {0x86, &CPU::STX_zp, 3 },      {0x87, &CPU::XXX, 3},
        {0x88, &CPU::DEY, 2 },     {0x89, &CPU::XXX, 2 },         {0x8A, &CPU::TXA, 2 },         {0x8B, &CPU::XXX, 2},
        {0x8C, &CPU::STY_abs, 4 }, {0x8D, &CPU::STA_abs, 4 },     {0x8E, &CPU::STX_abs, 4 },     {0x8F, &CPU::XXX, 4},
        {0x90, &CPU::BCC, 4 },     {0x91, &CPU::STA_indy, 6 },    {0x92, &CPU::XXX, 2 },         {0x93, &CPU::XXX, 6},
        {0x94, &CPU::STY_zpx, 4 }, {0x95, &CPU::STA_zpx, 4 },     {0x96, &CPU::STX_zpy, 4 },     {0x97, &CPU::XXX, 4},
        {0x98, &CPU::TYA, 2 },     {0x99, &CPU::STA_absy, 5 },    {0x9A, &CPU::TXS, 2 },         {0x9B, &CPU::XXX, 5},
        {0x9C, &CPU::XXX, 5 },     {0x9D, &CPU::STA_absx, 5 },    {0x9E, &CPU::XXX, 5 },         {0x9F, &CPU::XXX, 5},
        {0xA0, &CPU::LDY_imm, 2 }, {0xA1, &CPU::LDA_indx, 6 },    {0xA2, &CPU::LDX_imm, 2 },     {0xA3, &CPU::XXX, 6},
        {0xA4, &CPU::LDY_zp, 3 },  {0xA5, &CPU::LDA_zp,   3 },    {0xA6, &CPU::LDX_zp, 3 },      {0xA7, &CPU::XXX, 3},
        {0xA8, &CPU::TAY, 2 },     {0xA9, &CPU::LDA_imm,  2 },    {0xAA, &CPU::TAX, 2 },         {0xAB, &CPU::XXX, 2},
        {0xAC, &CPU::LDY_abs, 4 }, {0xAD, &CPU::LDA_abs,  4 },    {0xAE, &CPU::LDX_abs, 4 },     {0xAF, &CPU::XXX, 4},
        {0xB0, &CPU::BCS, 4 },     {0xB1, &CPU::LDA_indy, 6 },    {0xB2, &CPU::XXX, 2 },         {0xB3, &CPU::XXX, 5},
        {0xB4, &CPU::LDY_zpx, 4 }, {0xB5, &CPU::LDA_zpx,  4 },    {0xB6, &CPU::LDX_zpy, 4 },     {0xB7, &CPU::XXX, 4},
        {0xB8, &CPU::CLV, 2 },     {0xB9, &CPU::LDA_absy, 5 },    {0xBA, &CPU::TSX, 2 },         {0xBB, &CPU::XXX, 4},
        {0xBC, &CPU::LDY_absx, 5 },{0xBD, &CPU::LDA_absx, 5 },    {0xBE, &CPU::LDX_absy, 5 },    {0xBF, &CPU::XXX, 4},
        {0xC0, &CPU::CPY_imm, 2 }, {0xC1, &CPU::CMP_indx, 6 },    {0xC2, &CPU::XXX, 2 },         {0xC3, &CPU::XXX, 8},
        {0xC4, &CPU::CPY_zp, 3 },  {0xC5, &CPU::CMP_zp, 3   },    {0xC6, &CPU::DEC_zp, 5 },      {0xC7, &CPU::XXX, 5},
        {0xC8, &CPU::INY, 2 },     {0xC9, &CPU::CMP_imm, 2  },    {0xCA, &CPU::DEX, 2 },         {0xCB, &CPU::XXX, 2},
        {0xCC, &CPU::CPY_abs, 4 }, {0xCD, &CPU::CMP_abs, 4  },    {0xCE, &CPU::DEC_abs, 6 },     {0xCF, &CPU::XXX, 6},
        {0xD0, &CPU::BNE, 4 },     {0xD1, &CPU::CMP_indy, 6 },    {0xD2, &CPU::XXX, 2 },         {0xD3, &CPU::XXX, 8},
        {0xD4, &CPU::XXX, 4 },     {0xD5, &CPU::CMP_zpx, 4  },    {0xD6, &CPU::DEC_zpx, 6 },     {0xD7, &CPU::XXX, 6},
        {0xD8, &CPU::CLD, 2 },     {0xD9, &CPU::CMP_absy, 5 },    {0xDA, &CPU::NOP, 2 },         {0xDB, &CPU::XXX, 7},
        {0xDC, &CPU::XXX, 4 },     {0xDD, &CPU::CMP_absx, 5 },    {0xDE, &CPU::DEC_absx, 7 },    {0xDF, &CPU::XXX, 7},
        {0xE0, &CPU::CPX_imm, 2 }, {0xE1, &CPU::SBC_indx, 6 },    {0xE2, &CPU::XXX, 2 },         {0xE3, &CPU::XXX, 8},
        {0xE4, &CPU::CPX_zp, 3 },  {0xE5, &CPU::SBC_zp, 3   },    {0xE6, &CPU::INC_zp, 5 },      {0xE7, &CPU::XXX, 5},
        {0xE8, &CPU::INX, 2 },     {0xE9, &CPU::SBC_imm, 2  },    {0xEA, &CPU::NOP, 2 },         {0xEB, &CPU::XXX, 2},
        {0xEC, &CPU::CPX_abs, 4 }, {0xED, &CPU::SBC_abs, 4  },    {0xEE, &CPU::INC_abs, 6 },     {0xEF, &CPU::XXX, 6},
        {0xF0, &CPU::BEQ, 4 },     {0xF1, &CPU::SBC_indy, 6 },    {0xF2, &CPU::XXX, 2 },         {0xF3, &CPU::XXX, 8},
        {0xF4, &CPU::XXX, 4 },     {0xF5, &CPU::SBC_zpx, 4  },    {0xF6, &CPU::INC_zpx, 6 },     {0xF7, &CPU::XXX, 6},
        {0xF8, &CPU::SED, 2 },     {0xF9, &CPU::SBC_absy, 5 },    {0xFA, &CPU::NOP, 2 },         {0xFB, &CPU::XXX, 7},
        {0xFC, &CPU::XXX, 4 },     {0xFD, &CPU::SBC_absx, 5 },    {0xFE, &CPU::INC_absx, 7 },    {0xFF, &CPU::XXX, 7},
    };

    get_cycle = false;
    n_cycles = 0;
    PC = 0x0000;
    reset_flag = true;
    oamdma_flag = false;
}
CPU::~CPU() {}

void CPU::write(uint16_t address, uint8_t value)
{
    if(address < 0x2000)
        memory[address & 0x7FF] = value;
        
    else if(address == 0x4014)
    {
        OAMDMA = value;
        oamdma_flag = true;
        halt_cycle = true;
        if(get_cycle)
            alignment_needed = false;
        else
            alignment_needed = true;
        dma_address = 0x0000 | (OAMDMA << 8);
    }  
    else
        bus->cpu_writes(address, value);
}

uint8_t CPU::read(uint16_t address)
{
    uint8_t value;
    if(address  < 0x2000)
        value = memory[address & 0x7FF];
    else
        value = bus->cpu_reads(address);

    return value; 
}

void CPU::connect_bus(std::shared_ptr<Bus> bus)
{
    this->bus = bus;
}

void CPU::transfer_oam_bytes()
{
    switch(get_cycle)
    {
        case 1:
        {
            dma_read = read(dma_address);
            break;
        }
        case 0:
        {
            if((dma_address & 0x00FF) == 0xFF)
                oamdma_flag = false;        

            write(0x2004, dma_read);
            dma_address++;    
            break;
        }
    }
}

bool CPU::is_new_instruction()
{
    return new_instruction;
}

void CPU::set_nmi(bool value)
{
    pending_NMI = value;
}

void CPU::poll_interrupts()
{
    if(pending_NMI)
    {
        NMI = true;
        pending_NMI = false;
    }
    
    else if(bus->get_irq() && !(P & 0x4))
    {
        IRQ = true;
    }
    else
    {
        IRQ = false;
    }
}

void CPU::tick()
{
    get_cycle = !get_cycle;
    if(reset_flag)
        reset();

    else if(oamdma_flag)
    {
        if(halt_cycle)
            halt_cycle = false;
        else if(alignment_needed)
            alignment_needed = false;
        else
            transfer_oam_bytes();  
    }   
    
    else
    {
        if (n_cycles == 0)
        {
            fetch();
            if(Instr[opcode].cycles == 2)
                poll_interrupts();
            new_instruction = true;
        }
        
        else if(n_cycles < Instr[opcode].cycles)
        {
            (this->*Instr[opcode].function)();
            if(n_cycles == (Instr[opcode].cycles-1) && opcode != 0x00 && !branch_polled)
                poll_interrupts();
        } 
             
        if(n_cycles == Instr[opcode].cycles)
        {
            branch_polled = false;
            n_cycles = 0;
        }   
    }
}

void CPU::fetch()
{
    if(NMI)
        opcode = 0x00;

        
    else if(IRQ && !(P & 0x4))
        opcode = 0x00;
    
    
 
    else
    {
        opcode = read(PC);
        switch(opcode)
        {
            case 0x30:
            case 0xD0:
            case 0x10:
            case 0x50:
            case 0x70:
            case 0x90:
            case 0xB0:
            case 0xF0:
                branch_polled = true;   
                poll_interrupts();
                break;
        }
        PC++;
    }

    n_cycles++;
}

uint8_t CPU::get_opcode()
{
    return opcode;
}

void CPU::upd_negative_zero_flags(uint8_t byte)
{
    if (byte == 0x00)
        P = P | 0x02;
    else
        P =  P  & 0xFD;
    P = (P  & 0x7F) | (byte & 0x80);
}



//Addressing modes for instructions that perform operations inside the cpu

template <unsigned C>
void CPU::ie_zeropage()
{
    if constexpr(C == 1)
    {
        effective_addr = read(PC) & 0x00FF;
        PC++;
        n_cycles++;
    }
    
    else if constexpr(C == 2)
    {
        data = read(effective_addr);   
        n_cycles++;     
    } 
}

template <unsigned C>
void CPU::ie_abs()
{
    if constexpr(C == 1)
    {
        effective_addr = 0x0000;
        effective_addr |= read(PC);
        PC++;
        n_cycles++;        
    }

    else if constexpr(C == 2)
    {
        effective_addr |= ((read(PC) & 0x00FF) << 8);
        PC++;
        n_cycles++;           
    }

    else
    {
        data = read(effective_addr);
        n_cycles++;
    }   
}

template <unsigned C>
void CPU::ie_indx()
{
    if constexpr(C == 1)
    {
        effective_addr = 0x0000;
        zero_page_addr = read(PC) & 0x00FF;
        PC++;
        n_cycles++;       
    }

    else if constexpr(C == 2) { n_cycles++;}

    else if constexpr(C == 3)
    {
        effective_addr |= read((zero_page_addr + X) & 0xFF);
        n_cycles++;       
    }

    else if constexpr(C == 4)
    {
        effective_addr |= (uint16_t)(read((zero_page_addr + X + 1) & 0xFF) << 8);
        n_cycles++;       
    }

    else
    {
        data = read(effective_addr);
        n_cycles++;
    }
}

template <unsigned C>
void CPU::ie_absxy(uint8_t reg)
{
    if constexpr(C == 1)
    {
        effective_addr = 0x0000;
        absolute_addr = 0x0000;
        h = 0x0000;
        l = 0x0000;
        absolute_addr |= read(PC);
        PC++;
        n_cycles++;
    }

    else if constexpr(C == 2)
    {
        absolute_addr |= ((uint16_t)read(PC) << 8);
        PC++;
        n_cycles++; 
    }

    else if constexpr(C == 3)
    {

        high_byte = ((0xFF00 & absolute_addr) >> 8);
        low_byte = 0x00FF & absolute_addr;
        h = high_byte << 8;
        l = low_byte + reg;
        effective_addr = h | l;

        //No page crossing, skips T4
        if((int)(effective_addr & 0x00FF) >= reg)
        {
            page_crossing = false;
            data = read(effective_addr);
            n_cycles++;
        }
        n_cycles++;
    }

    //Page crossed, T4
    else
    {
        h = 0x0000;
        l = 0x0000;
        h = (high_byte + 1) << 8;
        l = (low_byte + reg) & 0xff;
        effective_addr = h | l;
        data = read(effective_addr);
        n_cycles++;
    }
}

template <unsigned C>
void CPU::ie_zpxy(uint8_t reg)
{
    if constexpr(C == 1)
    {
        effective_addr = 0x0000;
        zero_page_addr = read(PC) & 0x00FF;
        PC++;
        n_cycles++;        
    }
    else if constexpr(C == 2)
        n_cycles++;        
    
    else
    {
        effective_addr = (zero_page_addr + reg) & 0xFF;
        data = read(effective_addr);
        n_cycles++;
    }
}

template <unsigned C>
void CPU::ie_indy()
{
    if constexpr(C == 1)
    {
        h = 0x0000;
        l = 0x0000;
        effective_addr = 0x0000;
        absolute_addr = 0x0000;
        zero_page_addr = read(PC) & 0x00FF;
        n_cycles++;
        PC++;        
    }

    else if constexpr(C == 2)
    {
        absolute_addr |= read(zero_page_addr);
        n_cycles++;       
    }

    else if constexpr(C == 3)    
    {
        absolute_addr |= (read((zero_page_addr + 1) & 0xFF) << 8);
        n_cycles++;
    }

    else if constexpr(C == 4)
    {
        low_byte = 0x00FF & absolute_addr;
        high_byte = (0xFF00 & absolute_addr) >> 8;

        h = high_byte << 8;
        l = (low_byte + Y) & 0xFF;
        effective_addr = h | l;

        //No page crossing, skips T4
        if((int)(effective_addr & 0x00FF) >= Y)
        {
            page_crossing = false;
            data = read(effective_addr);
            n_cycles++;
        }
        n_cycles++;
    }

    else
    {
        h = 0x0000;
        l = 0x0000;
        h = (high_byte + 1) << 8;
        l = (low_byte + Y) & 0xff;
        effective_addr = h | l;
        data = read(effective_addr);
        n_cycles++;
    }              
}


//Useful functions for calculations

void CPU::ADC_calc()
{
    uint16_t calc;
    uint8_t old;

    old = Accumulator;
    calc = Accumulator + data + (P & 0x01); //0xFF + 0x2 + 0x1 = 258 = 0x102
    if(calc > 0xFF)
    {
        Accumulator = calc & 0x00FF;
        P |= 0x01;
    }
    else
    {
        P &= 0xFE;
        Accumulator = uint8_t(calc);
    }
    if(((old & 0x80) == (data & 0x80)) && ((Accumulator & 0x80) != (data & 0x80)))
        P |= 0x40;
    else
        P &= 0xBF;
}

void CPU::CMP_calc(uint8_t reg)
{
    uint8_t calc;

    calc = reg - data;
    if(reg >= data)               
        P |= 0x01;
    else
        P &= 0xFE;
    if(reg == data)
        P |= 0x02;
    else
        P &= 0xFD;
    P = (P  & 0x7F) | (calc & 0x80);
}

//Flag instructions

void CPU::SEC()
{
    P |= 0x01;
    n_cycles++;
}
void CPU::CLC()
{
    P &= 0xFE;
    n_cycles++;
}
void CPU::CLV()
{
    P &= 0xBF;
    n_cycles++;
}
void CPU::SEI()
{
    P |= 0x04;
    n_cycles++;
}
void CPU::CLI()
{
    P &= 0xFB;
    n_cycles++;
}
void CPU::SED()
{
    P |= 0x08;
    n_cycles++;
}
void CPU::CLD()
{
    P &= 0xF7;
    n_cycles++;
}

//Transfer instructions

void CPU::LDA_imm()
{
    data = read(PC);
    Accumulator = data;
    PC++;
    n_cycles++; 
    upd_negative_zero_flags(data);      
}

void CPU::LDA_zp()
{
    switch(n_cycles)
    {
        case 1: { ie_zeropage<1>(); break; }
        case 2:
        {
            ie_zeropage<2>();
            Accumulator = data;
            upd_negative_zero_flags(data);
            break;
        }
    } 
}

void CPU::LDA_zpx()
{
    switch(n_cycles)
    {
        case 1: { ie_zpxy<1>(X); break; }
        case 2: { ie_zpxy<2>(X); break; }
        case 3:
        {
            ie_zpxy<3>(X);
            Accumulator = data;
            upd_negative_zero_flags(data);
            break; 
        }          
    }  
}

void CPU::LDA_abs()
{
    switch(n_cycles)
    {
        case 1: { ie_abs<1>(); break; }
        case 2: { ie_abs<2>(); break; }

        case 3:
            ie_abs<3>();
            Accumulator = data;
            upd_negative_zero_flags(data);
            break;         
    }
}

void CPU::LDA_absx()
{
    switch(n_cycles)
    {
        case 1: { ie_absxy<1>(X); break; } 
        case 2: { ie_absxy<2>(X); break; }                
        case 3:
            ie_absxy<3>(X);
            //No page crossing, skips cycle 5
            if(!page_crossing)
            {
                page_crossing = true;
                Accumulator = data; 
                upd_negative_zero_flags(data);
            }              
            break; 
        //Page crossed 
        case 4:
            ie_absxy<4>(X);            
            Accumulator = data;
            upd_negative_zero_flags(data);
            break;       
    }   
}

void CPU::LDA_absy()
{
    switch(n_cycles)
    {
        case 1: { ie_absxy<1>(Y); break; } 
        case 2: { ie_absxy<2>(Y); break; }                
        case 3:
            ie_absxy<3>(Y);

            //No page crossing, skips cycle 5
            if(!page_crossing)
            {
                page_crossing = true;
                Accumulator = data;
                upd_negative_zero_flags(data);
            }
                                
            break; 
        //Page crossed 
        case 4:
            ie_absxy<4>(Y);
            Accumulator = data;
            upd_negative_zero_flags(data);
            break;       
    }     
}

void CPU::LDA_indx()
{
    switch(n_cycles)
    {
        case 1: { ie_indx<1>(); break; }
        case 2: { ie_indx<2>(); break; }
        case 3: { ie_indx<3>(); break; }
        case 4: { ie_indx<4>(); break; }
        case 5:
            ie_indx<5>();
            Accumulator = data;
            upd_negative_zero_flags(data);
            break;
    }
}

void CPU::LDA_indy()
{
    switch(n_cycles)
    {
        case 1: { ie_indy<1>(); break; }
        case 2: { ie_indy<2>(); break; }
        case 3: { ie_indy<3>(); break; }            
        case 4:
            ie_indy<4>();
            if(!page_crossing)
            {
                page_crossing = true;
                Accumulator = data;
                upd_negative_zero_flags(data);
            }
                
            break; 
        case 5:
            ie_indy<5>();
            Accumulator = data;
            upd_negative_zero_flags(data);
            break; 
    } 
}


void CPU::LDX_imm()
{
    data = read(PC);
    PC++;
    n_cycles++;
    X = data;  
    upd_negative_zero_flags(data);
}

void CPU::LDX_zp()
{
    switch(n_cycles)
    {
        case 1: { ie_zeropage<1>(); break; }
        case 2:
        {
            ie_zeropage<2>();
            X = data;
            upd_negative_zero_flags(data);
            break;
        }
    }
}

void CPU::LDX_zpy()
{
    switch(n_cycles)
    {
        case 1: { ie_zpxy<1>(Y); break; }
        case 2: { ie_zpxy<2>(Y); break; }
        case 3:
        {
            ie_zpxy<3>(Y);
            X = data;
            upd_negative_zero_flags(data);
            break; 
        }          
    }
}

void CPU::LDX_abs()
{
    switch(n_cycles)
    {
        case 1: { ie_abs<1>(); break; }
        case 2: { ie_abs<2>(); break; }
        case 3:
            ie_abs<3>();
            X = data;
            upd_negative_zero_flags(data);
            break;
    }
}

void CPU::LDX_absy()
{
    switch(n_cycles)
    {
        case 1: { ie_absxy<1>(Y); break; } 
        case 2: { ie_absxy<2>(Y); break; }                
        case 3:
            ie_absxy<3>(Y);

            // No page crossing, skip cycle 5
            if(!page_crossing)
            {
                page_crossing = true;
                X = data;
                upd_negative_zero_flags(data);
            }
            break;
            
        // Page crossed
        case 4:
            ie_absxy<4>(Y);
            X = data;
            upd_negative_zero_flags(data);
            break;
    }
}

void CPU::LDY_imm()
{
    data = read(PC);        // Fetch the immediate value from the program counter
    PC++;                   // Increment the program counter
    n_cycles++;             // Advance the cycle
    Y = data;               // Load the value into the Y register
    upd_negative_zero_flags(data); // Update flags based on the loaded value
}

void CPU::LDY_zp()
{
    switch(n_cycles)
    {
        case 1: 
            ie_zeropage<1>(); 
            break;
        case 2:
            ie_zeropage<2>();
            Y = data;                 // Load the value into the Y register
            upd_negative_zero_flags(data); // Update flags
            break;
    }
}

void CPU::LDY_zpx()
{
    switch(n_cycles)
    {
        case 1: 
            ie_zpxy<1>(X); 
            break;
        case 2: 
            ie_zpxy<2>(X); 
            break;
        case 3:
            ie_zpxy<3>(X);
            Y = data;                 // Load the value into the Y register
            upd_negative_zero_flags(data); // Update flags
            break; 
    }
}

void CPU::LDY_abs()
{
    switch(n_cycles)
    {
        case 1: 
            ie_abs<1>(); 
            break;
        case 2: 
            ie_abs<2>(); 
            break;
        case 3:
            ie_abs<3>();
            Y = data;                 // Load the value into the Y register
            upd_negative_zero_flags(data); // Update flags
            break;
    }
}

void CPU::LDY_absx()
{
    switch(n_cycles)
    {
        case 1: 
            ie_absxy<1>(X); 
            break;
        case 2: 
            ie_absxy<2>(X); 
            break;
        case 3:
            ie_absxy<3>(X);

            // If no page crossing, skip to the final step
            if(!page_crossing)
            {
                page_crossing = true;
                Y = data;             // Load the value into the Y register
                upd_negative_zero_flags(data); // Update flags
            }
            break;

        // Page crossing occurred, handle the extra cycle
        case 4:
            ie_absxy<4>(X);
            Y = data;                 // Load the value into the Y register
            upd_negative_zero_flags(data); // Update flags
            break;
    }
}

void CPU::STA_zp()
{
    switch(n_cycles)
    {
        case 1: 
            ie_zeropage<1>(); 
            break;
        case 2:
            write(effective_addr, Accumulator);
            n_cycles++;
            break;
    }
}

void CPU::STA_zpx()
{
    switch(n_cycles)
    {
        case 1: 
            ie_zpxy<1>(X); 
            break;
        case 2: 
            ie_zpxy<2>(X); 
            break;
        case 3:
            write((zero_page_addr + X) & 0xFF, Accumulator);
            n_cycles++;
            break;
    }
}

void CPU::STA_abs()
{
    switch(n_cycles)
    {
        case 1: 
            ie_abs<1>(); 
            break;
        case 2: 
            ie_abs<2>(); 
            break;
        case 3:
            write(effective_addr, Accumulator);
            n_cycles++;
            break;
    }
}

void CPU::STA_absx()
{
    switch(n_cycles)
    {
        case 1: 
            ie_absxy<1>(X); 
            break;
        case 2: 
            ie_absxy<2>(X); 
            break;
        case 3:
            n_cycles++;
            read(absolute_addr + X);
            break;
        case 4:
            effective_addr = absolute_addr + X;
            write(effective_addr, Accumulator);
            n_cycles++;
            break;
    }
}

void CPU::STA_absy()
{
    switch(n_cycles)
    {
        case 1: 
            ie_absxy<1>(Y); 
            break;
        case 2:
            ie_absxy<2>(Y); 
            break;
        case 3:
            effective_addr = absolute_addr + Y;
            if(!((effective_addr == 0x2007) && (Y == 0x17)))
                read(effective_addr);        
            n_cycles++;
            break;
        case 4:
            write(effective_addr, Accumulator);
            n_cycles++;
            break;
    }
}

void CPU::STA_indx()
{
    switch(n_cycles)
    {
        case 1: 
            ie_indx<1>(); 
            break;
        case 2: 
            ie_indx<2>(); 
            break;
        case 3: 
            ie_indx<3>(); 
            break;
        case 4: 
            ie_indx<4>(); 
            break;
        case 5:
            write(effective_addr, Accumulator);
            n_cycles++;
            break;
    }
}

void CPU::STA_indy()
{
    switch(n_cycles)
    {
        case 1: 
            ie_indy<1>(); 
            break;
        case 2: 
            ie_indy<2>(); 
            break;
        case 3: 
            ie_indy<3>(); 
            break;
        case 4:
            effective_addr = absolute_addr + Y;
            n_cycles++;
            break;
        case 5:
            write(effective_addr, Accumulator);
            n_cycles++;
            break;
    }
}

void CPU::STX_abs()
{
    switch(n_cycles)
    {
        case 1: 
            ie_abs<1>(); 
            break; 
        case 2: 
            ie_abs<2>(); 
            break; 
        case 3:
            write(effective_addr, X);
            n_cycles++;
            break;         
    }
}

void CPU::STX_zp()
{
    switch(n_cycles)
    {
        case 1: 
            ie_zeropage<1>(); 
            break;
        case 2:
            write(effective_addr, X);
            n_cycles++;
            break;
    }
}

void CPU::STX_zpy()
{
    switch(n_cycles)
    {
        case 1: 
            ie_zpxy<1>(Y); 
            break;
        case 2: 
            ie_zpxy<2>(Y); 
            break;
        case 3:
            write((zero_page_addr + Y) & 0xFF, X);
            n_cycles++;
            break;
    }
}

void CPU::STY_zp()
{
    switch(n_cycles)
    {
        case 1: 
            ie_zeropage<1>(); 
            break;
        case 2:
            write(effective_addr, Y);
            n_cycles++;
            break;
    } 
}

void CPU::STY_zpx()
{
    switch(n_cycles)
    {
        case 1: 
            ie_zpxy<1>(X); 
            break;
        case 2: 
            ie_zpxy<2>(X); 
            break;
        case 3:
            write((zero_page_addr + X) & 0xFF, Y);
            n_cycles++;
            break;           
    }  
}

void CPU::STY_abs()
{
    switch(n_cycles)
    {
        case 1: 
            ie_abs<1>(); 
            break;
        case 2: 
            ie_abs<2>(); 
            break;
        case 3:
            write(effective_addr, Y);
            n_cycles++;
            break;         
    }
}

void CPU::TAX()
{
    X = Accumulator;
    n_cycles++;
    upd_negative_zero_flags(Accumulator);
}
void CPU::TAY()
{
    Y = Accumulator;
    n_cycles++;
    upd_negative_zero_flags(Accumulator);
}
void CPU::TXA()
{
    Accumulator = X;
    n_cycles++;
    upd_negative_zero_flags(Accumulator);
}
void CPU::TYA()
{
    Accumulator = Y;
    n_cycles++;
    upd_negative_zero_flags(Y);
}
void CPU::TXS()
{
    SP = X;
    n_cycles++;
}
void CPU::TSX()
{
    X = SP;
    n_cycles++;
    upd_negative_zero_flags(SP);
}

//Stack instructions
void CPU::PHA()
{
    switch(n_cycles)
    {
        case 1: { n_cycles++; break;}
        case 2:
            write(0x100 + SP, Accumulator);
            SP--;
            n_cycles++;
            break;
    }
}
void CPU::PHP()
{
    switch(n_cycles)
    {
        case 1: { n_cycles++; break; }
        case 2:
            write(0x100 + SP, (P | 0x30));
            SP--;
            n_cycles++;
            break;
    }
}
void CPU::PLA()
{
    switch(n_cycles)
    {
        case 1: { n_cycles++; break; }
        case 2: { n_cycles++; break; }
        case 3:
            SP++;
            Accumulator = read(0x100 + SP);
            n_cycles++;
            break;
    }

    upd_negative_zero_flags(Accumulator);
}
void CPU::PLP()
{
    switch(n_cycles)
    {
        case 1: { n_cycles++; break; }
        case 2: { n_cycles++; break; }
        case 3:
            SP++;
            P = read(0x100 + SP);
            P |= 0x20;
            P &= 0xEF;
            n_cycles++;
            break;
    }
}

//Decrement and increment instructions
void CPU::DEC_abs()
{
    switch(n_cycles)
    {
        case 1: { ie_abs<1>(); break; }
        case 2: { ie_abs<2>(); break; }
        case 3: { data = read(effective_addr); n_cycles++; break; };
        case 4: { write(effective_addr, data); new_instruction = false; n_cycles++; break; };
        case 5:
            if(data == 0x00)
                data = 0xFF;
            else 
                data--;
            write(effective_addr, data);
            n_cycles++;
            upd_negative_zero_flags(data);
            break;
    }
}

void CPU::DEC_zp()
{
    switch(n_cycles)
    {
        case 1: 
            ie_zeropage<1>(); 
            break;
        case 2: 
            ie_zeropage<2>(); 
            break;
        case 3: 
            write(effective_addr, data); 
            new_instruction = false; 
            n_cycles++; 
            break;
        case 4:
            if(data == 0x00)
                data = 0xFF;
            else 
                data--;
            write(effective_addr, data);
            n_cycles++;
            upd_negative_zero_flags(data);
            break;
    }
}

void CPU::DEC_zpx()
{
    switch(n_cycles)
    {
        case 1: 
            ie_zpxy<1>(X); 
            break;
        case 2: 
            n_cycles++; 
            break;
        case 3: 
            ie_zpxy<3>(X); 
            break;
        case 4: 
            write(effective_addr, data);
            new_instruction = false; 
            n_cycles++; 
            break;
        case 5:
            if(data == 0x00)
                data = 0xFF;
            else 
                data--;
            write(effective_addr, data);
            n_cycles++;
            upd_negative_zero_flags(data);
            break;               
    }
}

void CPU::DEC_absx()
{
    switch(n_cycles)
    {
        case 1: 
            ie_absxy<1>(X); 
            break;
        case 2: 
            ie_absxy<2>(X); 
            break;
        case 3:
            n_cycles++;
            break;
        case 4:

            effective_addr = absolute_addr + X;
            data = read(effective_addr);
            n_cycles++;
            break;
        case 5: 
            write(effective_addr, data); 
            new_instruction = false; 
            n_cycles++; 
            break;
        case 6:
            if(data == 0x00)
                data = 0xFF;
            else 
                data--;
            write(effective_addr, data);
            n_cycles++;
            upd_negative_zero_flags(data);
            break;         
    }
}

void CPU::DEX()
{
    if(X == 0x00)
        X = 0xFF;
    else
        X--;
    n_cycles++;
    upd_negative_zero_flags(X);
}
void CPU::DEY()
{
    if(Y == 0x00)
        Y = 0xFF;
    else
        Y--;

    n_cycles++;
    upd_negative_zero_flags(Y);
}

void CPU::INC_abs()
{
    switch(n_cycles)
    {
        case 1: 
            ie_abs<1>(); 
            break;
        case 2: 
            ie_abs<2>(); 
            break;
        case 3: 
            data = read(effective_addr); 
            n_cycles++; 
            break;
        case 4: 
            write(effective_addr, data); 
            new_instruction = false; 
            n_cycles++; 
            break;
        case 5:
            if(data == 0xFF)
                data = 0x00;
            else 
                data++;
            write(effective_addr, data);
            n_cycles++;
            upd_negative_zero_flags(data);
            break;
    }
}

void CPU::INC_zp()
{
    switch(n_cycles)
    {
        case 1: 
            ie_zeropage<1>(); 
            break;
        case 2: 
            ie_zeropage<2>(); 
            break;
        case 3: 
            write(effective_addr, data); 
            new_instruction = false; 
            n_cycles++; 
            break;
        case 4:
            if(data == 0xFF)
                data = 0x00;
            else 
                data++;
            write(effective_addr, data);
            n_cycles++;
            upd_negative_zero_flags(data);
            break;
    }
}

void CPU::INC_zpx()
{
    switch(n_cycles)
    {
        case 1: 
            ie_zpxy<1>(X); 
            break;
        case 2: 
            n_cycles++; 
            break;
        case 3: 
            ie_zpxy<3>(X); 
            break;
        case 4: 
            write(effective_addr, data); 
            new_instruction = false; 
            n_cycles++; 
            break;
        case 5:
            if(data == 0xFF)
                data = 0x00;
            else 
                data++;
            write(effective_addr, data);
            n_cycles++;
            upd_negative_zero_flags(data);
            break;               
    }
}

void CPU::INC_absx()
{
    switch(n_cycles)
    {
        case 1: 
            ie_absxy<1>(X); 
            break;
        case 2: 
            ie_absxy<2>(X); 
            break;
        case 3:
            n_cycles++;
            break;
        case 4:
            effective_addr = absolute_addr + X;
            data = read(effective_addr);
            n_cycles++;
            break;
        case 5: 
            write(effective_addr, data); 
            new_instruction = false; 
            n_cycles++; 
            break;
        case 6:
            if(data == 0xFF)
                data = 0x00;
            else 
                data++;
            write(effective_addr, data);
            n_cycles++;
            upd_negative_zero_flags(data);
            break;          
    }
}

void CPU::INX()
{
    if(X == 0xFF)
        X = 0x00;
    else
        X++;
    n_cycles++;
    upd_negative_zero_flags(X); 
}
void CPU::INY()
{
    if(Y == 0xFF)
        Y = 0x00;

    else
        Y++;
    n_cycles++;
    upd_negative_zero_flags(Y);
}

//Arithmetic operations

void CPU::ADC_imm()
{
    data = read(PC);
    PC++;
    n_cycles++;
    ADC_calc();
    upd_negative_zero_flags(Accumulator);
}

void CPU::ADC_zp()
{
    switch(n_cycles)
    {
        case 1: 
            ie_zeropage<1>(); 
            break;
        case 2: 
            ie_zeropage<2>(); 
            ADC_calc(); 
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::ADC_zpx()
{
    switch(n_cycles)
    {
        case 1: 
            ie_zpxy<1>(X); 
            break;
        case 2: 
            ie_zpxy<2>(X); 
            break;
        case 3: 
            ie_zpxy<3>(X); 
            ADC_calc(); 
            upd_negative_zero_flags(Accumulator);
            break;           
    }
}

void CPU::ADC_abs()
{
    switch(n_cycles)
    {
        case 1: 
            ie_abs<1>(); 
            break;
        case 2: 
            ie_abs<2>(); 
            break;
        case 3: 
            ie_abs<3>(); 
            ADC_calc(); 
            upd_negative_zero_flags(Accumulator);
            break;        
    } 
}

void CPU::ADC_absx()
{
    switch(n_cycles)
    {
        case 1: 
            ie_absxy<1>(X); 
            break;
        case 2: 
            ie_absxy<2>(X); 
            break;
        case 3:
            ie_absxy<3>(X);
            if(!page_crossing)
            {
                ADC_calc();
                page_crossing = true;
                upd_negative_zero_flags(Accumulator);
            }
            break; 
        case 4:
            ie_absxy<4>(X);
            ADC_calc();
            upd_negative_zero_flags(Accumulator);
            break;      
    }        
}

void CPU::ADC_absy()
{
    switch(n_cycles)
    {
        case 1: 
            ie_absxy<1>(Y); 
            break;
        case 2: 
            ie_absxy<2>(Y); 
            break;
        case 3:
            ie_absxy<3>(Y);
            if(!page_crossing)
            {
                ADC_calc();
                page_crossing = true;
                upd_negative_zero_flags(Accumulator);
            }
            break; 
        case 4:
            ie_absxy<4>(Y);
            ADC_calc();
            upd_negative_zero_flags(Accumulator);
            break;      
    }
}

void CPU::ADC_indx()
{
    switch(n_cycles)
    {
        case 1: 
            ie_indx<1>(); 
            break;
        case 2: 
            ie_indx<2>(); 
            break;
        case 3: 
            ie_indx<3>(); 
            break;
        case 4: 
            ie_indx<4>(); 
            break;
        case 5:
            ie_indx<5>();
            ADC_calc();
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::ADC_indy()
{
    switch(n_cycles)
    {
        case 1: 
            ie_indy<1>(); 
            break;
        case 2: 
            ie_indy<2>(); 
            break;
        case 3: 
            ie_indy<3>(); 
            break;             
        case 4:
            ie_indy<4>();
            if(!page_crossing)
            {
                ADC_calc();
                page_crossing = true;
                upd_negative_zero_flags(Accumulator);
            }
            break; 
        case 5:
            ie_indy<5>();
            ADC_calc();
            upd_negative_zero_flags(Accumulator);
            break; 
    }       
}

void CPU::SBC_imm()
{
    data = read(PC);
    PC++;
    n_cycles++;
    data = ~data; // Two's complement
    ADC_calc();
    upd_negative_zero_flags(Accumulator);
}

void CPU::SBC_zp()
{
    switch (n_cycles)
    {
        case 1: 
            ie_zeropage<1>(); 
            break;
        case 2: 
            ie_zeropage<2>(); 
            data = ~data; // Two's complement
            ADC_calc(); 
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::SBC_zpx()
{
    switch (n_cycles)
    {
        case 1: 
            ie_zpxy<1>(X); 
            break;
        case 2: 
            ie_zpxy<2>(X); 
            break;
        case 3: 
            ie_zpxy<3>(X); 
            data = ~data; // Two's complement
            ADC_calc(); 
            upd_negative_zero_flags(Accumulator);
            break;           
    }
}

void CPU::SBC_abs()
{
    switch (n_cycles)
    {
        case 1: 
            ie_abs<1>(); 
            break;
        case 2: 
            ie_abs<2>(); 
            break;
        case 3: 
            ie_abs<3>(); 
            data = ~data; // Two's complement
            ADC_calc(); 
            upd_negative_zero_flags(Accumulator);
            break;        
    } 
}

void CPU::SBC_absx()
{
    switch (n_cycles)
    {
        case 1: 
            ie_absxy<1>(X); 
            break;
        case 2: 
            ie_absxy<2>(X); 
            break;
        case 3:
            ie_absxy<3>(X);
            if (!page_crossing)
            {
                data = ~data; // Two's complement
                ADC_calc();
                page_crossing = true;
                upd_negative_zero_flags(Accumulator);
            }
            break; 
        case 4:
            ie_absxy<4>(X);
            data = ~data; // Two's complement
            ADC_calc();
            upd_negative_zero_flags(Accumulator);
            break;      
    }        
}

void CPU::SBC_absy()
{
    switch (n_cycles)
    {
        case 1: 
            ie_absxy<1>(Y); 
            break;
        case 2: 
            ie_absxy<2>(Y); 
            break;
        case 3:
            ie_absxy<3>(Y);
            if (!page_crossing)
            {
                data = ~data; // Two's complement
                ADC_calc();
                page_crossing = true;
                upd_negative_zero_flags(Accumulator);
            }
            break; 
        case 4:
            ie_absxy<4>(Y);
            data = ~data; // Two's complement
            ADC_calc();
            upd_negative_zero_flags(Accumulator);
            break;      
    }
}

void CPU::SBC_indx()
{
    switch (n_cycles)
    {
        case 1: 
            ie_indx<1>(); 
            break;
        case 2: 
            ie_indx<2>(); 
            break;
        case 3: 
            ie_indx<3>(); 
            break;
        case 4: 
            ie_indx<4>(); 
            break;
        case 5:
            ie_indx<5>();
            data = ~data; // Two's complement
            ADC_calc();
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::SBC_indy()
{
    switch (n_cycles)
    {
        case 1: 
            ie_indy<1>(); 
            break;
        case 2: 
            ie_indy<2>(); 
            break;
        case 3: 
            ie_indy<3>(); 
            break;             
        case 4:
            ie_indy<4>();
            if (!page_crossing)
            {
                data = ~data; // Two's complement
                ADC_calc();
                page_crossing = true;
                upd_negative_zero_flags(Accumulator);
            }
            break; 
        case 5:
            ie_indy<5>();
            data = ~data; // Two's complement
            ADC_calc();
            upd_negative_zero_flags(Accumulator);
            break; 
    }       
}


//Logical operations

void CPU::AND_imm()
{
    data = read(PC);
    Accumulator &= data;
    PC++;
    n_cycles++;
    upd_negative_zero_flags(Accumulator);
}

void CPU::AND_zp()
{
    switch(n_cycles)
    {
        case 1:
            ie_zeropage<1>();
            break;
        case 2:
            ie_zeropage<2>();
            Accumulator &= data;
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::AND_zpx()
{
    switch(n_cycles)
    {
        case 1:
            ie_zpxy<1>(X);
            break;
        case 2:
            ie_zpxy<2>(X);
            break;
        case 3:
            ie_zpxy<3>(X);
            Accumulator &= data;
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::AND_abs()
{
    switch(n_cycles)
    {
        case 1:
            ie_abs<1>();
            break;
        case 2:
            ie_abs<2>();
            break;
        case 3:
            ie_abs<3>();
            Accumulator &= data;
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::AND_absx()
{
    switch(n_cycles)
    {
        case 1:
            ie_absxy<1>(X);
            break;
        case 2:
            ie_absxy<2>(X);
            break;
        case 3:
            ie_absxy<3>(X);
            if (!page_crossing)
            {
                page_crossing = true;
                Accumulator &= data;
                upd_negative_zero_flags(Accumulator);
            }
            break;
        case 4:
            ie_absxy<4>(X);
            Accumulator &= data;
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::AND_absy()
{
    switch(n_cycles)
    {
        case 1:
            ie_absxy<1>(Y);
            break;
        case 2:
            ie_absxy<2>(Y);
            break;
        case 3:
            ie_absxy<3>(Y);
            if (!page_crossing)
            {
                page_crossing = true;
                Accumulator &= data;
                upd_negative_zero_flags(Accumulator);
            }
            break;
        case 4:
            ie_absxy<4>(Y);
            Accumulator &= data;
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::AND_indx()
{
    switch(n_cycles)
    {
        case 1:
            ie_indx<1>();
            break;
        case 2:
            ie_indx<2>();
            break;
        case 3:
            ie_indx<3>();
            break;
        case 4:
            ie_indx<4>();
            break;
        case 5:
            ie_indx<5>();
            Accumulator &= data;
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::AND_indy()
{
    switch(n_cycles)
    {
        case 1:
            ie_indy<1>();
            break;
        case 2:
            ie_indy<2>();
            break;
        case 3:
            ie_indy<3>();
            break;
        case 4:
            ie_indy<4>();
            if (!page_crossing)
            {
                page_crossing = true;
                Accumulator &= data;
                upd_negative_zero_flags(Accumulator);
            }
            break;
        case 5:
            ie_indy<5>();
            Accumulator &= data;
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::EOR_imm()
{
    data = read(PC);
    Accumulator ^= data;
    PC++;
    n_cycles++;
    upd_negative_zero_flags(Accumulator);
}

void CPU::EOR_zp()
{
    switch(n_cycles)
    {
        case 1:
            ie_zeropage<1>();
            break;
        case 2:
            ie_zeropage<2>();
            Accumulator ^= data;
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::EOR_zpx()
{
    switch(n_cycles)
    {
        case 1:
            ie_zpxy<1>(X);
            break;
        case 2:
            ie_zpxy<2>(X);
            break;
        case 3:
            ie_zpxy<3>(X);
            Accumulator ^= data;
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::EOR_abs()
{
    switch(n_cycles)
    {
        case 1:
            ie_abs<1>();
            break;
        case 2:
            ie_abs<2>();
            break;
        case 3:
            ie_abs<3>();
            Accumulator ^= data;
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::EOR_absx()
{
    switch(n_cycles)
    {
        case 1:
            ie_absxy<1>(X);
            break;
        case 2:
            ie_absxy<2>(X);
            break;
        case 3:
            ie_absxy<3>(X);
            if (!page_crossing)
            {
                page_crossing = true;
                Accumulator ^= data;
                upd_negative_zero_flags(Accumulator);
            }
            break;
        case 4:
            ie_absxy<4>(X);
            Accumulator ^= data;
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::EOR_absy()
{
    switch(n_cycles)
    {
        case 1:
            ie_absxy<1>(Y);
            break;
        case 2:
            ie_absxy<2>(Y);
            break;
        case 3:
            ie_absxy<3>(Y);
            if (!page_crossing)
            {
                page_crossing = true;
                Accumulator ^= data;
                upd_negative_zero_flags(Accumulator);
            }
            break;
        case 4:
            ie_absxy<4>(Y);
            Accumulator ^= data;
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::EOR_indx()
{
    switch(n_cycles)
    {
        case 1:
            ie_indx<1>();
            break;
        case 2:
            ie_indx<2>();
            break;
        case 3:
            ie_indx<3>();
            break;
        case 4:
            ie_indx<4>();
            break;
        case 5:
            ie_indx<5>();
            Accumulator ^= data;
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::EOR_indy()
{
    switch(n_cycles)
    {
        case 1:
            ie_indy<1>();
            break;
        case 2:
            ie_indy<2>();
            break;
        case 3:
            ie_indy<3>();
            break;
        case 4:
            ie_indy<4>();
            if (!page_crossing)
            {
                page_crossing = true;
                Accumulator ^= data;
                upd_negative_zero_flags(Accumulator);
            }
            break;
        case 5:
            ie_indy<5>();
            Accumulator ^= data;
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::ORA_imm()
{
    data = read(PC);
    Accumulator |= data;
    PC++;
    n_cycles++;
    upd_negative_zero_flags(Accumulator);
}

void CPU::ORA_zp()
{
    switch(n_cycles)
    {
        case 1:
            ie_zeropage<1>();
            break;
        case 2:
            ie_zeropage<2>();
            Accumulator |= data;
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::ORA_zpx()
{
    switch(n_cycles)
    {
        case 1:
            ie_zpxy<1>(X);
            break;
        case 2:
            ie_zpxy<2>(X);
            break;
        case 3:
            ie_zpxy<3>(X);
            Accumulator |= data;
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::ORA_abs()
{
    switch(n_cycles)
    {
        case 1:
            ie_abs<1>();
            break;
        case 2:
            ie_abs<2>();
            break;
        case 3:
            ie_abs<3>();
            Accumulator |= data;
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::ORA_absx()
{
    switch(n_cycles)
    {
        case 1:
            ie_absxy<1>(X);
            break;
        case 2:
            ie_absxy<2>(X);
            break;
        case 3:
            ie_absxy<3>(X);
            if (!page_crossing)
            {
                Accumulator |= data;
                page_crossing = true;
                upd_negative_zero_flags(Accumulator);
            }
            break;
        case 4:
            ie_absxy<4>(X);
            Accumulator |= data;
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::ORA_absy()
{
    switch(n_cycles)
    {
        case 1:
            ie_absxy<1>(Y);
            break;
        case 2:
            ie_absxy<2>(Y);
            break;
        case 3:
            ie_absxy<3>(Y);
            if (!page_crossing)
            {
                Accumulator |= data;
                page_crossing = true;
                upd_negative_zero_flags(Accumulator);
            }
            break;
        case 4:
            ie_absxy<4>(Y);
            Accumulator |= data;
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::ORA_indx()
{
    switch(n_cycles)
    {
        case 1:
            ie_indx<1>();
            break;
        case 2:
            ie_indx<2>();
            break;
        case 3:
            ie_indx<3>();
            break;
        case 4:
            ie_indx<4>();
            break;
        case 5:
            ie_indx<5>();
            Accumulator |= data;
            upd_negative_zero_flags(Accumulator);
            break;
    }
}

void CPU::ORA_indy()
{
    switch(n_cycles)
    {
        case 1:
            ie_indy<1>();
            break;
        case 2:
            ie_indy<2>();
            break;
        case 3:
            ie_indy<3>();
            break;
        case 4:
            ie_indy<4>();
            if (!page_crossing)
            {
                Accumulator |= data;
                page_crossing = true;
                upd_negative_zero_flags(Accumulator);
            }
            break;
        case 5:
            ie_indy<5>();
            Accumulator |= data;
            upd_negative_zero_flags(Accumulator);
            break;
    }
}


//Shift &CPU:: Rotate Instructions

void CPU::ASL_imm()
{
    P = (P  & 0xFE) | ((Accumulator & 0x80) >> 7);
    Accumulator <<=1;
    Accumulator &= 0xFE;
    data = Accumulator;
    n_cycles++;
    upd_negative_zero_flags(data);
}

void CPU::ASL_zp()
{
    switch(n_cycles)
    {
        case 1: { ie_zeropage<1>(); break; }
        case 2: { ie_zeropage<2>(); break; }
        case 3: { write(effective_addr, data); new_instruction = false; n_cycles++; break; };
        case 4:
            P = (P  & 0xFE) | ((data & 0x80) >> 7);
            data <<= 1;
            data &= 0xFE;
            write(effective_addr, data);
            n_cycles++;
            upd_negative_zero_flags(data);
            break;
    }
}

void CPU::ASL_zpx()
{
    switch(n_cycles)
    {
        case 1: { ie_zpxy<1>(X); break; }
        case 2: { n_cycles++; break;}
        case 3: { ie_zpxy<3>(X); break; }
        case 4: { write(effective_addr, data); new_instruction = false; n_cycles++; break; };
        case 5:
            P = (P  & 0xFE) | ((data & 0x80) >> 7);
            data <<= 1;
            data &= 0xFE;
            write(effective_addr, data);
            n_cycles++;
            upd_negative_zero_flags(data);
            break;               
    }
}

void CPU::ASL_abs()
{
    switch(n_cycles)
    {
        case 1: { ie_abs<1>(); break;}
        case 2: { ie_abs<2>(); break;}
        case 3: { data = read(effective_addr); n_cycles++; break; };
        case 4: { write(effective_addr, data); new_instruction = false; n_cycles++; break; };
        case 5:
            P = (P  & 0xFE) | ((data & 0x80) >> 7);
            data <<= 1;
            data &= 0xFE;
            write(effective_addr, data);
            n_cycles++;
            upd_negative_zero_flags(data);
            break;
    }
}

void CPU::ASL_absx()
{
    switch(n_cycles)
    {
        case 1: { ie_absxy<1>(X); break; }
        case 2: { ie_absxy<2>(X); break; }
        case 3:
            n_cycles++;
            break;
        case 4:
            effective_addr = absolute_addr + X;
            data = read(effective_addr);
            n_cycles++;
            break;
        case 5: { write(effective_addr, data); new_instruction = false; n_cycles++; break; };
        case 6:
            P = (P  & 0xFE) | ((data & 0x80) >> 7);
            data <<= 1;
            data &= 0xFE;
            write(effective_addr, data);
            n_cycles++;
            upd_negative_zero_flags(data);
            break;           
    }
}


void CPU::LSR_imm()
{
    P = (P & 0xFE) | (Accumulator & 0x01);
    Accumulator >>= 1;
    Accumulator &= 0x7F;
    data = Accumulator;
    n_cycles++;
    upd_negative_zero_flags(data);
}

void CPU::LSR_zp()
{
    switch(n_cycles)
    {
        case 1: { ie_zeropage<1>(); break; }
        case 2: { ie_zeropage<2>(); break; }
        case 3: { data = read(effective_addr); n_cycles++; break; }
        case 4:
            P = (P & 0xFE) | (data & 0x01);
            data >>= 1;
            data &= 0x7F;
            write(effective_addr, data);
            n_cycles++;
            upd_negative_zero_flags(data);
            break;
    }
}

void CPU::LSR_zpx()
{
    switch(n_cycles)
    {
        case 1: { ie_zpxy<1>(X); break; }
        case 2: { n_cycles++; break; }
        case 3: { ie_zpxy<3>(X); break; }
        case 4: { data = read(effective_addr); n_cycles++; break; }
        case 5:
            P = (P & 0xFE) | (data & 0x01);
            data >>= 1;
            data &= 0x7F;
            write(effective_addr, data);
            n_cycles++;
            upd_negative_zero_flags(data);
            break;               
    }
}

void CPU::LSR_abs()
{
    switch(n_cycles)
    {
        case 1: { ie_abs<1>(); break; }
        case 2: { ie_abs<2>(); break; }
        case 3: { data = read(effective_addr); n_cycles++; break; }
        case 4: { write(effective_addr, data); new_instruction = false; n_cycles++; break; }
        case 5:
            P = (P  & 0xFE) | (data & 0x01);
            data >>= 1;
            data &= 0x7F;
            write(effective_addr, data);
            n_cycles++;
            upd_negative_zero_flags(data);
            break;
    }
}

void CPU::LSR_absx()
{
    switch(n_cycles)
    {
        case 1: { ie_absxy<1>(X); break; }
        case 2: { ie_absxy<2>(X); break; }
        case 3:
            n_cycles++;
            break;
        case 4:
            effective_addr = absolute_addr + X;
            data = read(effective_addr);
            n_cycles++;
            break;
        case 5: { write(effective_addr, data); new_instruction = false; n_cycles++; break; };
        case 6:
            data = read(effective_addr);
            P = (P  & 0xFE) | (data & 0x01);
            data >>= 1;
            data &= 0x7F;
            write(effective_addr, data);
            n_cycles++;
            upd_negative_zero_flags(data);
            break;           
    }
}

void CPU::ROL_imm()
{
    uint8_t aux = (Accumulator >> 7) & 0x01; // Get the carry bit
    Accumulator <<= 1; // Shift left
    Accumulator = (Accumulator & 0xFE) | (P & 0x01); // Insert carry from P
    P = (P & 0xFE) | aux; // Update the carry in P
    data = Accumulator;
    n_cycles++;
    upd_negative_zero_flags(data);
}

void CPU::ROL_zp()
{
    switch(n_cycles)
    {
        case 1: { ie_zeropage<1>(); break; }
        case 2: { ie_zeropage<2>(); break; }
        case 3: { data = read(effective_addr); n_cycles++; break; }
        case 4:
            {
                uint8_t aux = (data >> 7) & 0x01;
                data <<= 1;
                data = (data & 0xFE) | (P & 0x01);
                P = (P & 0xFE) | aux;
                write(effective_addr, data);
                n_cycles++;
                upd_negative_zero_flags(data);
            }
            break;
    }
}

void CPU::ROL_zpx()
{
    switch(n_cycles)
    {
        case 1: { ie_zpxy<1>(X); break; }
        case 2: { n_cycles++; break; }
        case 3: { ie_zpxy<3>(X); break; }
        case 4: { data = read(effective_addr); n_cycles++; break; }
        case 5:
            {
                uint8_t aux = (data >> 7) & 0x01;
                data <<= 1;
                data = (data & 0xFE) | (P & 0x01);
                P = (P & 0xFE) | aux;
                write(effective_addr, data);
                n_cycles++;
                upd_negative_zero_flags(data);
            }
            break;               
    }
}




void CPU::ROL_abs()
{
    uint8_t aux;
    switch(n_cycles)
    {
        case 1: { ie_abs<1>(); break; }
        case 2: { ie_abs<2>(); break; }
        case 3: { data = read(effective_addr); n_cycles++; break; };
        case 4: { write(effective_addr, data); new_instruction = false; n_cycles++; break; };
        case 5:
            aux = (data >> 7) & 0x01;
            data <<= 1;
            data = (data & 0xFE) | (P & 0x01);
            P = (P  & 0xFE) | (aux & 0x01);
            write(effective_addr, data);
            n_cycles++;
            upd_negative_zero_flags(data);
            break;
    }
}

void CPU::ROL_absx()
{
    uint8_t aux;
    switch(n_cycles)
    {
        case 1: { ie_absxy<1>(X); break; }
        case 2: { ie_absxy<2>(X); break; }
        case 3:
            n_cycles++;
            break;
        case 4:
            effective_addr = absolute_addr + X;
            data = read(effective_addr);
            n_cycles++;
            break;
        case 5: { write(effective_addr, data); new_instruction = false; n_cycles++; break; };
        case 6:
            data = read(effective_addr);
            aux = (data >> 7) & 0x01;
            data <<= 1;
            data = (data & 0xFE) | (P & 0x01);
            P = (P  & 0xFE) | (aux & 0x01);
            write(effective_addr, data);
            n_cycles++;
            upd_negative_zero_flags(data);
            break;             
        }
}

void CPU::ROR_imm()
{
    uint8_t aux = (Accumulator & 0x01); // Get the carry bit
    Accumulator >>= 1; // Shift right
    Accumulator = (Accumulator & 0x7F) | ((P & 0x01) << 7); // Insert carry from P
    P = (P & 0xFE) | aux; // Update the carry in P
    data = Accumulator;
    n_cycles++;
    upd_negative_zero_flags(data);
}

void CPU::ROR_zp()
{
    switch(n_cycles)
    {
        case 1: { ie_zeropage<1>(); break; }
        case 2: { ie_zeropage<2>(); break; }
        case 3: { data = read(effective_addr); n_cycles++; break; }
        case 4:
            {
                uint8_t aux = data & 0x01;
                data >>= 1;
                data = (data & 0x7F) | ((P & 0x01) << 7);
                P = (P & 0xFE) | aux;
                write(effective_addr, data);
                n_cycles++;
                upd_negative_zero_flags(data);
            }
            break;
    }
}

void CPU::ROR_zpx()
{
    switch(n_cycles)
    {
        case 1: { ie_zpxy<1>(X); break; }
        case 2: { n_cycles++; break; }
        case 3: { ie_zpxy<3>(X); break; }
        case 4: { data = read(effective_addr); n_cycles++; break; }
        case 5:
            {
                uint8_t aux = data & 0x01;
                data >>= 1;
                data = (data & 0x7F) | ((P & 0x01) << 7);
                P = (P & 0xFE) | aux;
                write(effective_addr, data);
                n_cycles++;
                upd_negative_zero_flags(data);
            }
            break;               
    }
}

void CPU::ROR_abs()
{
    uint8_t aux;
    switch(n_cycles)
    {
        case 1: { ie_abs<1>(); break; }
        case 2: { ie_abs<2>(); break; }
        case 3: { data = read(effective_addr); n_cycles++; break; };
        case 4: { write(effective_addr, data); new_instruction = false; n_cycles++; break; };
        case 5:
            aux = data & 0x01;
            data >>= 1;
            data = (data & 0x7F) | ((P & 0x01) << 7);
            data = data;
            P = (P & 0xFE) | (aux & 0x01);
            write(effective_addr, data);
            n_cycles++;
            upd_negative_zero_flags(data);
            break;
    }
}

void CPU::ROR_absx()
{
    uint8_t aux;
    switch(n_cycles)
    {
        case 1: { ie_absxy<1>(X); break; }
        case 2: { ie_absxy<2>(X); break; }
        case 3:
            n_cycles++;
            break;
        case 4:
            effective_addr = absolute_addr + X;
            data = read(effective_addr);
            n_cycles++;
            break;
        case 5: { write(effective_addr, data); new_instruction = false; n_cycles++; break; };
        case 6:
            data = read(effective_addr);
            aux = data & 0x01;
            data >>= 1;
            data = (data & 0x7F) | ((P & 0x01) << 7);
            data = data;
            P = (P & 0xFE) | (aux & 0x01);
            write(effective_addr, data);
            n_cycles++;
            upd_negative_zero_flags(data);
            break;            
    }
}


//Comparison instructions

void CPU::CMP_imm()
{
    data = read(PC);
    CMP_calc(Accumulator);
    PC++;
    n_cycles++;
}

void CPU::CMP_zp()
{
    switch(n_cycles)
    {
        case 1: { ie_zeropage<1>(); break; }
        case 2:
        {
            ie_zeropage<2>();
            CMP_calc(Accumulator);
            break;
        }
    }
}

void CPU::CMP_zpx()
{
    switch(n_cycles)
    {
        case 1: { ie_zpxy<1>(X); break; }
        case 2: { ie_zpxy<2>(X); break; }
        case 3:
        {
            ie_zpxy<3>(X);
            CMP_calc(Accumulator);
            break;  
        }
    }
}

void CPU::CMP_abs()
{
    switch(n_cycles)
    {
        case 1: { ie_abs<1>(); break; }
        case 2: { ie_abs<2>(); break; }
        case 3:
        {
            ie_abs<3>();
            CMP_calc(Accumulator);
            break; 
        }        
    }
}

void CPU::CMP_absx()
{
    switch(n_cycles)
    {
        case 1: { ie_absxy<1>(X); break; }
        case 2: { ie_absxy<2>(X); break; }
        case 3:
            ie_absxy<3>(X);
            // No page crossing, skips cycle 5
            if (!page_crossing)
            {
                page_crossing = true;
                CMP_calc(Accumulator);
            }
            break; 
        // Page crossed 
        case 4:
            ie_absxy<4>(X);
            CMP_calc(Accumulator);
            break;      
    }
}

void CPU::CMP_absy()
{
    switch(n_cycles)
    {
        case 1: { ie_absxy<1>(Y); break; }
        case 2: { ie_absxy<2>(Y); break; }
        case 3:
            ie_absxy<3>(Y);
            // No page crossing, skips cycle 5
            if (!page_crossing)
            {
                page_crossing = true;
                CMP_calc(Accumulator);
            }
            break; 
        // Page crossed 
        case 4:
            ie_absxy<4>(Y);
            CMP_calc(Accumulator);
            break;      
    }
}

void CPU::CMP_indx()
{
    switch(n_cycles)
    {
        case 1: { ie_indx<1>(); break; }
        case 2: { ie_indx<2>(); break; }
        case 3: { ie_indx<3>(); break; }
        case 4: { ie_indx<4>(); break; }
        case 5:
        {
            ie_indx<5>();
            CMP_calc(Accumulator);
            break;
        }
    }
}

void CPU::CMP_indy()
{
    switch(n_cycles)
    {
        case 1: { ie_indy<1>(); break; }
        case 2: { ie_indy<2>(); break; }
        case 3: { ie_indy<3>(); break; }             
        case 4:
        {
            ie_indy<4>();
            if (!page_crossing)
            {
                CMP_calc(Accumulator);
                page_crossing = true;
            }
            break; 
        }
        case 5:
        {
            ie_indy<5>();
            CMP_calc(Accumulator);
            break; 
        }
    }
}

void CPU::CPX_imm()
{
    data = read(PC);
    CMP_calc(X);
    PC++;
    n_cycles++;
}

void CPU::CPX_zp()
{
    switch(n_cycles)
    {
        case 1: { ie_zeropage<1>(); break; }
        case 2:
        {
            ie_zeropage<2>();
            CMP_calc(X);              
            break;
        }
    }
}

void CPU::CPX_abs()
{
    switch(n_cycles)
    {
        case 1: { ie_abs<1>(); break; }
        case 2: { ie_abs<2>(); break; }

        case 3:
        {
            ie_abs<3>();
            CMP_calc(X);
            break; 
        }        
    }
}

void CPU::CPY_imm()
{
    data = read(PC);
    CMP_calc(Y);
    PC++;
    n_cycles++;
}

void CPU::CPY_zp()
{
    switch(n_cycles)
    {
        case 1: { ie_zeropage<1>(); break; }
        case 2:
        {
            ie_zeropage<2>();
            CMP_calc(Y);              
            break;
        }
    }
}

void CPU::CPY_abs()
{
    switch(n_cycles)
    {
        case 1: { ie_abs<1>(); break; }
        case 2: { ie_abs<2>(); break; }

        case 3:
        {
            ie_abs<3>();
            CMP_calc(Y);
            break; 
        }        
    }
}


//Conditional Branch Instructions

void CPU::BCC()
{
    switch(n_cycles)
    {
        case 1:
            offset = read(PC);
            PC++;
            if((P & 0x01))
                n_cycles+=3;
            else
                n_cycles++;
            break;

        case 2:
            high_byte = ((0xFF00 & PC) >> 8);           
            PC += offset;
            if(high_byte == ((PC & 0xFF00) >> 8))
                n_cycles+=2;
            else
            {
                poll_interrupts();
                n_cycles++;
            }         
            break;
        case 3:         
            n_cycles++;
            break;
    }
}
void CPU::BCS()
{
    switch(n_cycles)
    {
        case 1:
            offset = read(PC);
            PC++;
            if(!(P & 0x01))
                n_cycles+=3;
            else
                n_cycles++;
            break;
        case 2:
            high_byte = ((0xFF00 & PC) >> 8);           
            PC += offset;
            if(high_byte == ((PC & 0xFF00) >> 8))
                n_cycles+=2;
            else
            {
                poll_interrupts();
                n_cycles++;
            }              
            break;
        case 3:         
            n_cycles++;
            break;
    }
}
void CPU::BEQ()
{
    switch(n_cycles)
    {
        case 1:
            offset = read(PC);
            PC++;
            if(!((P & 0x02) >> 1))
            {
                n_cycles+=3;
            }
            else
                n_cycles++;
            break;
        case 2:
            high_byte = ((0xFF00 & PC) >> 8);           
            PC += offset;
            if(high_byte == ((PC & 0xFF00) >> 8))
                n_cycles+=2;
            else
            {
                poll_interrupts();
                n_cycles++;
            }           
            break;
        case 3:         
            n_cycles++;
            break;
    }
}
void CPU::BMI()
{

    switch(n_cycles)
    {
        case 1:
            offset = read(PC);
            PC++;
            if(!(P & 0x80))
                n_cycles+=3;
            else
                n_cycles++;
            break;
        case 2:
            high_byte = ((0xFF00 & PC) >> 8);           
            PC += offset;
            if(high_byte == ((PC & 0xFF00) >> 8))
                n_cycles+=2;
            else
            {
                poll_interrupts();
                n_cycles++;
            }           
            break;
        case 3:         
            n_cycles++;
            break;
    }
}
void CPU::BNE()
{
    switch(n_cycles)
    {
        case 1:
            offset = read(PC);
            PC++;
            if(P & 0x02)
                n_cycles+=3;
            else
                n_cycles++;
            break;
        case 2:
            high_byte = ((0xFF00 & PC) >> 8);           
            PC += offset;
            if(high_byte == ((PC & 0xFF00) >> 8))
                n_cycles+=2;
            else
            {
                poll_interrupts();
                n_cycles++;
            }         
            break;
        case 3:         
            n_cycles++;
            break;
    }
}
void CPU::BPL()
{
    switch(n_cycles)
    {
        case 1:
            offset = read(PC);
            PC++;
            if(P & 0x80)
                n_cycles+=3;
            else
                n_cycles++;
            break;
        case 2:
            high_byte = ((0xFF00 & PC) >> 8);           
            PC += offset;
            if(high_byte == ((PC & 0xFF00) >> 8))
                n_cycles+=2;
            else
            {
                poll_interrupts();
                n_cycles++;
            }         
            break;
        case 3:         
            n_cycles++;
            break;
    }
}
void CPU::BVC()
{
    switch(n_cycles)
    {
        case 1:
            offset = read(PC);
            PC++;
            if(P & 0x40)
                n_cycles+=3;
            else
                n_cycles++;
            break;
        case 2:
            high_byte = ((0xFF00 & PC) >> 8);           
            PC += offset;
            if(high_byte == ((PC & 0xFF00) >> 8))
                n_cycles+=2;
            else
            {
                poll_interrupts();
                n_cycles++;
            }        
            break;
        case 3:         
            n_cycles++;
            break;
    }
}
void CPU::BVS()
{
    switch(n_cycles)
    {
        case 1:
            offset = read(PC);
            PC++;
            if(!(P & 0x40))
                n_cycles+=3;
            else
                n_cycles++;
            break;
        case 2:
            high_byte = ((0xFF00 & PC) >> 8);           
            PC += offset;
            if(high_byte == ((PC & 0xFF00) >> 8))
                n_cycles+=2;
            else
            {
                poll_interrupts();
                n_cycles++;
            }       
            break;
        case 3:         
            n_cycles++;
            break;
    }
}

//Jumps &CPU:: Subroutines

void CPU::JMP_abs()
{
    switch(n_cycles)
    {
        case 1: { ie_abs<1>(); break; }
        case 2:  
            ie_abs<2>();
            PC = effective_addr; 
            break; 
    }
}

void CPU::JMP_ind()
{
    switch(n_cycles)
    {
        case 1: { ie_abs<1>(); break; }
        case 2: { ie_abs<2>(); break; }
        case 3:
            jmp_address = 0x0000;
            jmp_address |= read(effective_addr);
            n_cycles++;
            break;
        case 4:
            jmp_address |= (read((effective_addr & 0xFF00) | (uint8_t)((effective_addr & 0x00FF) + 1)) << 8);
            PC = jmp_address;
            n_cycles++;
            break;            
    }
}

void CPU::JSR()
{
    switch(n_cycles)
    {
        case 1:
            subroutine_address = 0x0000;
            subroutine_address |= read(PC);
            PC++;
            n_cycles++;
            break;
        case 2:
            n_cycles++;
            break;
        case 3:
            write(0x100 + SP, (PC & 0xFF00) >> 8);
            SP--;
            n_cycles++;
            break;
        case 4:
            write(0x100 + SP, PC & 0x00FF);
            SP--;
            n_cycles++;
            break;
        case 5:
            subroutine_address |= ((read(PC) & 0x00FF) << 8);
            PC = subroutine_address;
            n_cycles++;
            break;
    }
}
void CPU::RTS()
{
    switch(n_cycles)
    {
        case 1:  
            n_cycles++; 
            subroutine_address = 0x0000; 
            break; 
        case 2: { n_cycles++; break; }
        case 3:
            SP++;
            subroutine_address |= read(0x100 + SP);
            n_cycles++;
            break;
        case 4:
            n_cycles++;
            SP++;
            subroutine_address |= ((read(0x100 + SP) & 0x00FF) << 8);
            break;
        case 5:
            PC = subroutine_address + 1;
            n_cycles++;
            break;

    }
}

//Interrupts

void CPU::BRK()
{
    switch (n_cycles)
    {
        case 1: 
            n_cycles++; 
            break; 
        case 2:
            write(0x100 + SP, (PC & 0xFF00) >> 8);
            SP--;
            n_cycles++;
            break;
        case 3:
            write(0x100 + SP, PC & 0x00FF);
            SP--;
            n_cycles++;
            break;
        case 4:
            write(0x100 + SP, P);
            P |= 0x04;
            SP--;
            n_cycles++;
            break;
        case 5:
            PC = 0x0000;
            if(NMI)
                PC |= read(0xFFFA);
            else
                PC |= read(0xFFFE);
            n_cycles++;
            break;
        case 6:
            if(NMI)
            {
                PC |= (read(0xFFFB) << 8);
                NMI = false;
            }

            else
                PC |= (read(0xFFFF) << 8);

  
            n_cycles++;
            break;
    } 
}

void CPU::RTI()
{
    switch(n_cycles)
    {
        case 1:
            subroutine_address = 0x0000;
            n_cycles++; 
            break; 
        case 2: { n_cycles++; break; }
        case 3:
            SP++;
            P = read(0x100 + SP);
            n_cycles++;
            break;
        case 4:
            SP++;
            subroutine_address |= read(0x100 + SP);
            n_cycles++;
            break;
        case 5:
            n_cycles++;
            SP++;
            subroutine_address |= ((read(0x100 + SP) & 0x00FF) << 8);
            PC = subroutine_address;
            break;
    } 
}

//Other

void CPU::BIT_zp()
{
    switch(n_cycles)
    {
        case 1: { ie_zeropage<1>(); break; }
        case 2:
        {
            ie_zeropage<2>();
            uint8_t aux = data & Accumulator;
            P = (aux == 0x00) ? (P | 0x02) : (P & 0xFD);
            P = (P & 0x7F) | (data & 0x80);
            P = (P & 0xBF) | (data & 0x40);
            break;
        }
    }
}

void CPU::BIT_abs()
{
    switch(n_cycles)
    {
        case 1: { ie_abs<1>(); break; }
        case 2: { ie_abs<2>(); break; }
        case 3:
        {
            ie_abs<3>();
            uint8_t aux = data & Accumulator;
            P = (aux == 0x00) ? (P | 0x02) : (P & 0xFD);
            P = (P & 0x7F) | (data & 0x80);
            P = (P & 0xBF) | (data & 0x40);
            break; 
        }       
    }
}

void CPU::NOP()
{
    n_cycles++;
}
void CPU::XXX()
{
    n_cycles++;
}

void CPU::reset()
{
    switch(n_cycles)
    {
        case 0:
            Accumulator = X = Y = SP = 0;
            P = 0x4;
            n_cycles++;
            break;
        case 1:
        case 2:
            n_cycles++;
            break;
        case 3:
        case 4:
        case 5:
            SP--;
            n_cycles++;
            break;
        case 6:
            PC = 0x0000;
            PC |= read(0xFFFC);
            n_cycles++;
            break;
        case 7:
            PC |= read(0xFFFD) << 8;
            n_cycles = 0;
            reset_flag = false;
            break;


    }
}

void CPU::soft_reset()
{
    // Reset cycle and logging-related variables
    cycles = 0;
    start_logging = false;

    // Reset opcode
    opcode = 0x00;

    // Reset CPU registers
    Accumulator = 0x00;
    X = 0x00;
    Y = 0x00;
    PC = 0x0000;
    SP = 0; // Stack pointer usually starts at 0xFF
    P = 0;  // Default status register with unused and interrupt disable bits set

    // Reset Direct Memory Access (DMA) related variables
    OAMDMA = 0x00;
    oamdma_flag = false;
    get_cycle = false;
    alignment_needed = false;
    dma_read = 0x00;
    dma_address = 0x0000;

    // Reset interrupt and control flags
    reset_flag = true;
    NMI = false;

    // Reset addressing and operation-related variables
    jmp_address = 0x0000;
    rel_address = 0x0000;
    subroutine_address = 0x0000;
    zero_page_addr = 0x0000;
    absolute_addr = 0x0000;
    effective_addr = 0x0000;
    page_crossing = false;

    // Reset cycle tracking variables
    n_cycles = 0;
    offset = 0x00;

    // Reset temporary data variables
    data = 0x00;
    high_byte = 0x00;
    low_byte = 0x00;
    h = 0x0000;
    l = 0x0000;

    // Reset internal memory without altering its size
    std::fill(std::begin(memory), std::end(memory), 0);
}