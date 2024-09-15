#include "CPU.h"
#include "Bus.h"

//Testeados ORA, ADC, SBC, CMP, CPX, CPY, EOR, AND, LDA, LDX, LDY, BIT, ASL, LSR, ROL, ROR, DEC, INC, STA, STX, STY, CLC
//CLD, CLV, CLI, SEC, SED, SEI, DEX, DEY, INX, INY, NOP, TAX, TAY, TSX, TXA, TXS, TYA, BCC, BCS, BEQ, BMI, BNE, BPL, BVC, BVS
//PHA, PLA, PLP, PHP, JSR, RTS, RTI

//Falta BRK, JMP

CPU::CPU()
{
    //Falla 0x4E
    Instr = 
    {
        { 0x00, &CPU::BRK, 7 }, { 0x01, &CPU::ORA, 6 }, { 0x02, &CPU::XXX, 2 }, { 0x03, &CPU::XXX, 8 },
        { 0x04, &CPU::NOP, 3 }, { 0x05, &CPU::ORA, 3 }, { 0x06, &CPU::ASL, 5 }, { 0x07, &CPU::XXX, 5 },
        { 0x08, &CPU::PHP, 3 }, { 0x09, &CPU::ORA, 2 }, { 0x0A, &CPU::ASL, 2 }, { 0x0B, &CPU::XXX, 2 },
        { 0x0C, &CPU::NOP, 4 }, { 0x0D, &CPU::ORA, 4 }, { 0x0E, &CPU::ASL, 6 }, { 0x0F, &CPU::XXX, 6 },
        { 0x10, &CPU::BPL, 4 }, { 0x11, &CPU::ORA, 6 }, { 0x12, &CPU::XXX, 2 }, { 0x13, &CPU::XXX, 8 },
        { 0x14, &CPU::NOP, 4 }, { 0x15, &CPU::ORA, 4 }, { 0x16, &CPU::ASL, 6 }, { 0x17, &CPU::XXX, 6 },
        { 0x18, &CPU::CLC, 2 }, { 0x19, &CPU::ORA, 5 }, { 0x1A, &CPU::NOP, 2 }, { 0x1B, &CPU::XXX, 7 },
        { 0x1C, &CPU::XXX, 4 }, { 0x1D, &CPU::ORA, 5 }, { 0x1E, &CPU::ASL, 7 }, { 0x1F, &CPU::XXX, 7 },
        { 0x20, &CPU::JSR, 6 }, { 0x21, &CPU::AND, 6 }, { 0x22, &CPU::XXX, 2 }, { 0x23, &CPU::XXX, 8 },
        { 0x24, &CPU::BIT, 3 }, { 0x25, &CPU::AND, 3 }, { 0x26, &CPU::ROL, 5 }, { 0x27, &CPU::XXX, 5 },
        { 0x28, &CPU::PLP, 4 }, { 0x29, &CPU::AND, 2 }, { 0x2A, &CPU::ROL, 2 }, { 0x2B, &CPU::XXX, 2 },
        { 0x2C, &CPU::BIT, 4 }, { 0x2D, &CPU::AND, 4 }, { 0x2E, &CPU::ROL, 6 }, { 0x2F, &CPU::XXX, 6 },
        { 0x30, &CPU::BMI, 4 }, { 0x31, &CPU::AND, 6 }, { 0x32, &CPU::XXX, 2 }, { 0x33, &CPU::XXX, 8 },
        { 0x34, &CPU::XXX, 4 }, { 0x35, &CPU::AND, 4 }, { 0x36, &CPU::ROL, 6 }, { 0x37, &CPU::XXX, 6 },
        { 0x38, &CPU::SEC, 2 }, { 0x39, &CPU::AND, 5 }, { 0x3A, &CPU::NOP, 2 }, { 0x3B, &CPU::XXX, 7 },
        { 0x3C, &CPU::XXX, 4 }, { 0x3D, &CPU::AND, 5 }, { 0x3E, &CPU::ROL, 7 }, { 0x3F, &CPU::XXX, 7 },
        { 0x40, &CPU::RTI, 6 }, { 0x41, &CPU::EOR, 6 }, { 0x42, &CPU::XXX, 2 }, { 0x43, &CPU::XXX, 8 },
        { 0x44, &CPU::NOP, 3 }, { 0x45, &CPU::EOR, 3 }, { 0x46, &CPU::LSR, 5 }, { 0x47, &CPU::XXX, 5 },
        { 0x48, &CPU::PHA, 3 }, { 0x49, &CPU::EOR, 2 }, { 0x4A, &CPU::LSR, 2 }, { 0x4B, &CPU::XXX, 2 },
        { 0x4C, &CPU::JMP, 3 }, { 0x4D, &CPU::EOR, 4 }, { 0x4E, &CPU::LSR, 6 }, { 0x4F, &CPU::XXX, 6 },
        { 0x50, &CPU::BVC, 4 }, { 0x51, &CPU::EOR, 6 }, { 0x52, &CPU::XXX, 2 }, { 0x53, &CPU::XXX, 8 },
        { 0x54, &CPU::XXX, 4 }, { 0x55, &CPU::EOR, 4 }, { 0x56, &CPU::LSR, 6 }, { 0x57, &CPU::XXX, 6 },
        { 0x58, &CPU::CLI, 2 }, { 0x59, &CPU::EOR, 5 }, { 0x5A, &CPU::NOP, 2 }, { 0x5B, &CPU::XXX, 7 },
        { 0x5C, &CPU::XXX, 4 }, { 0x5D, &CPU::EOR, 5 }, { 0x5E, &CPU::LSR, 7 }, { 0x5F, &CPU::XXX, 7 },
        { 0x60, &CPU::RTS, 6 }, { 0x61, &CPU::ADC, 6 }, { 0x62, &CPU::XXX, 2 }, { 0x63, &CPU::XXX, 8 },
        { 0x64, &CPU::XXX, 3 }, { 0x65, &CPU::ADC, 3 }, { 0x66, &CPU::ROR, 5 }, { 0x67, &CPU::XXX, 5 },
        { 0x68, &CPU::PLA, 4 }, { 0x69, &CPU::ADC, 2 }, { 0x6A, &CPU::ROR, 2 }, { 0x6B, &CPU::XXX, 2 },
        { 0x6C, &CPU::JMP, 5 }, { 0x6D, &CPU::ADC, 4 }, { 0x6E, &CPU::ROR, 6 }, { 0x6F, &CPU::XXX, 6 },
        { 0x70, &CPU::BVS, 4 }, { 0x71, &CPU::ADC, 6 }, { 0x72, &CPU::XXX, 2 }, { 0x73, &CPU::XXX, 8 },
        { 0x74, &CPU::XXX, 4 }, { 0x75, &CPU::ADC, 4 }, { 0x76, &CPU::ROR, 6 }, { 0x77, &CPU::XXX, 6 },
        { 0x78, &CPU::SEI, 2 }, { 0x79, &CPU::ADC, 5 }, { 0x7A, &CPU::NOP, 2 }, { 0x7B, &CPU::XXX, 7 },
        { 0x7C, &CPU::XXX, 4 }, { 0x7D, &CPU::ADC, 5 }, { 0x7E, &CPU::ROR, 7 }, { 0x7F, &CPU::XXX, 7 },
        { 0x80, &CPU::XXX, 2 }, { 0x81, &CPU::STA, 6 }, { 0x82, &CPU::XXX, 2 }, { 0x83, &CPU::XXX, 6 },
        { 0x84, &CPU::STY, 3 }, { 0x85, &CPU::STA, 3 }, { 0x86, &CPU::STX, 3 }, { 0x87, &CPU::XXX, 3 },
        { 0x88, &CPU::DEY, 2 }, { 0x89, &CPU::XXX, 2 }, { 0x8A, &CPU::TXA, 2 }, { 0x8B, &CPU::XXX, 2 },
        { 0x8C, &CPU::STY, 4 }, { 0x8D, &CPU::STA, 4 }, { 0x8E, &CPU::STX, 4 }, { 0x8F, &CPU::XXX, 4 },
        { 0x90, &CPU::BCC, 4 }, { 0x91, &CPU::STA, 6 }, { 0x92, &CPU::XXX, 2 }, { 0x93, &CPU::XXX, 6 },
        { 0x94, &CPU::STY, 4 }, { 0x95, &CPU::STA, 4 }, { 0x96, &CPU::STX, 4 }, { 0x97, &CPU::XXX, 4 },
        { 0x98, &CPU::TYA, 2 }, { 0x99, &CPU::STA, 5 }, { 0x9A, &CPU::TXS, 2 }, { 0x9B, &CPU::XXX, 5 },
        { 0x9C, &CPU::XXX, 5 }, { 0x9D, &CPU::STA, 5 }, { 0x9E, &CPU::XXX, 5 }, { 0x9F, &CPU::XXX, 5 },
        { 0xA0, &CPU::LDY, 2 }, { 0xA1, &CPU::LDA, 6 }, { 0xA2, &CPU::LDX, 2 }, { 0xA3, &CPU::XXX, 6 },
        { 0xA4, &CPU::LDY, 3 }, { 0xA5, &CPU::LDA, 3 }, { 0xA6, &CPU::LDX, 3 }, { 0xA7, &CPU::XXX, 3 },
        { 0xA8, &CPU::TAY, 2 }, { 0xA9, &CPU::LDA, 2 }, { 0xAA, &CPU::TAX, 2 }, { 0xAB, &CPU::XXX, 2 },
        { 0xAC, &CPU::LDY, 4 }, { 0xAD, &CPU::LDA, 4 }, { 0xAE, &CPU::LDX, 4 }, { 0xAF, &CPU::XXX, 4 },
        { 0xB0, &CPU::BCS, 4 }, { 0xB1, &CPU::LDA, 6 }, { 0xB2, &CPU::XXX, 2 }, { 0xB3, &CPU::XXX, 5 },
        { 0xB4, &CPU::LDY, 4 }, { 0xB5, &CPU::LDA, 4 }, { 0xB6, &CPU::LDX, 4 }, { 0xB7, &CPU::XXX, 4 },
        { 0xB8, &CPU::CLV, 2 }, { 0xB9, &CPU::LDA, 5 }, { 0xBA, &CPU::TSX, 2 }, { 0xBB, &CPU::XXX, 4 },
        { 0xBC, &CPU::LDY, 5 }, { 0xBD, &CPU::LDA, 5 }, { 0xBE, &CPU::LDX, 5 }, { 0xBF, &CPU::XXX, 4 },
        { 0xC0, &CPU::CPY, 2 }, { 0xC1, &CPU::CMP, 6 }, { 0xC2, &CPU::XXX, 2 }, { 0xC3, &CPU::XXX, 8 },
        { 0xC4, &CPU::CPY, 3 }, { 0xC5, &CPU::CMP, 3 }, { 0xC6, &CPU::DEC, 5 }, { 0xC7, &CPU::XXX, 5 },
        { 0xC8, &CPU::INY, 2 }, { 0xC9, &CPU::CMP, 2 }, { 0xCA, &CPU::DEX, 2 }, { 0xCB, &CPU::XXX, 2 },
        { 0xCC, &CPU::CPY, 4 }, { 0xCD, &CPU::CMP, 4 }, { 0xCE, &CPU::DEC, 6 }, { 0xCF, &CPU::XXX, 6 },
        { 0xD0, &CPU::BNE, 4 }, { 0xD1, &CPU::CMP, 6 }, { 0xD2, &CPU::XXX, 2 }, { 0xD3, &CPU::XXX, 8 },
        { 0xD4, &CPU::XXX, 4 }, { 0xD5, &CPU::CMP, 4 }, { 0xD6, &CPU::DEC, 6 }, { 0xD7, &CPU::XXX, 6 },
        { 0xD8, &CPU::CLD, 2 }, { 0xD9, &CPU::CMP, 5 }, { 0xDA, &CPU::NOP, 2 }, { 0xDB, &CPU::XXX, 7 },
        { 0xDC, &CPU::XXX, 4 }, { 0xDD, &CPU::CMP, 5 }, { 0xDE, &CPU::DEC, 7 }, { 0xDF, &CPU::XXX, 7 },
        { 0xE0, &CPU::CPX, 2 }, { 0xE1, &CPU::SBC, 6 }, { 0xE2, &CPU::XXX, 2 }, { 0xE3, &CPU::XXX, 8 },
        { 0xE4, &CPU::CPX, 3 }, { 0xE5, &CPU::SBC, 3 }, { 0xE6, &CPU::INC, 5 }, { 0xE7, &CPU::XXX, 5 },
        { 0xE8, &CPU::INX, 2 }, { 0xE9, &CPU::SBC, 2 }, { 0xEA, &CPU::NOP, 2 }, { 0xEB, &CPU::XXX, 2 },
        { 0xEC, &CPU::CPX, 4 }, { 0xED, &CPU::SBC, 4 }, { 0xEE, &CPU::INC, 6 }, { 0xEF, &CPU::XXX, 6 },
        { 0xF0, &CPU::BEQ, 4 }, { 0xF1, &CPU::SBC, 6 }, { 0xF2, &CPU::XXX, 2 }, { 0xF3, &CPU::XXX, 8 },
        { 0xF4, &CPU::XXX, 4 }, { 0xF5, &CPU::SBC, 4 }, { 0xF6, &CPU::INC, 6 }, { 0xF7, &CPU::XXX, 6 },
        { 0xF8, &CPU::SED, 2 }, { 0xF9, &CPU::SBC, 5 }, { 0xFA, &CPU::NOP, 2 }, { 0xFB, &CPU::XXX, 7 },
        { 0xFC, &CPU::XXX, 4 }, { 0xFD, &CPU::SBC, 5 }, { 0xFE, &CPU::INC, 7 }, { 0xFF, &CPU::XXX, 7 },
    };

    odd_cycle = false;
    cycles_dma = 0;
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
        if(odd_cycle)
            alignment_needed = true;
        else
            alignment_needed = false;
        dma_address = 0x0000 | (OAMDMA << 8);
    }  
    else
        bus->cpu_writes(address, value);
    
}

uint8_t CPU::read(uint16_t address)
{
    uint8_t value;
    if(address  < 0x2000)
    {
        value = memory[address & 0x7FFF];
    }
    else
    {
        value = bus->cpu_reads(address);
    }
    return value; 
}

void CPU::connect_bus(std::shared_ptr<Bus> bus)
{
    this->bus = bus;
}

void CPU::transfer_oam_bytes()
{
    switch(odd_cycle)
    {
        case 0:
        {
            if(cycles_dma == 0);
            else
                dma_read = read(dma_address);
            break;
        }
        case 1:
        {
            if(cycles_dma == 0);
            else if(alignment_needed)
                alignment_needed = false;
            else
            {
                write(0x2004, dma_read);
                dma_address++;
            }
            break;
        }
    }
    cycles_dma++; 
    if(((dma_address & 0x00FF) == 0x00) && (cycles_dma > 3))
    {
        cycles_dma = 0;
        oamdma_flag = false;
    } 
}

void CPU::tick()
{
    odd_cycle = !odd_cycle;

    if(oamdma_flag)
        transfer_oam_bytes();

    else
    {
        if (n_cycles == 0)
        {
            fetch();
            finish = false;
        }
        else if(n_cycles < Instr[opcode].cycles)
            (this->*Instr[opcode].function)();
        
        if(n_cycles == Instr[opcode].cycles)
        {
            n_cycles = 0;
            finish = true;
        }
    }
}

void CPU::fetch()
{
    
    if(bus->get_nmi())
        opcode =  0x00; //BRK instruction. Handles NMI
    else
    {
        opcode = read(PC);
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
        //std::cout << "Address: " << std::hex << (int)effective_addr << "Data: " << (int)data<<std::endl;
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

void CPU::LDA()
{
    //Immediate
    if(opcode == 0xA9)
    {
        data = read(PC);
        Accumulator = data;
        PC++;
        n_cycles++;       
    }

    //Zero page
    if(opcode == 0xA5)
    {
        switch(n_cycles)
        {
            case 1: { ie_zeropage<1>(); break; }
            case 2:
            {
                ie_zeropage<2>();
                Accumulator = data;
                break;
            }
        }       
    }

    //Zero page, X
    else if(opcode == 0xB5)
    {
        switch(n_cycles)
        {
            case 1: { ie_zpxy<1>(X); break; }
            case 2: { ie_zpxy<2>(X); break; }
            case 3:
            {
                ie_zpxy<3>(X);
                Accumulator = data;
                break; 
            }          
        }        
    }

    //Absolute
    else if(opcode == 0xAD)
    {
        switch(n_cycles)
        {
            case 1: { ie_abs<1>(); break; }
            case 2: { ie_abs<2>(); break; }

            case 3:
                ie_abs<3>();
                Accumulator = data;
                break;         
        } 
    }

    //Absolute, X
    else if(opcode == 0xBD)
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
                }              
                break; 
            //Page crossed 
            case 4:
                ie_absxy<4>(X);            
                Accumulator = data;
                break;       
        }        
    }

    //Absolute, Y
    else if(opcode == 0xB9)
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
                }
                                   
                break; 
            //Page crossed 
            case 4:
                ie_absxy<4>(Y);
                Accumulator = data;
                break;       
        }          
    }

    //Indirect, X
    else if(opcode == 0xA1)
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
                break;
        }
    }

    //Indirect, Y
    else if(opcode == 0xB1)
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
                }
                    
                break; 
            case 5:
                ie_indy<5>();
                Accumulator = data;
                break; 
        }       
    }

    if(Instr[opcode].cycles == n_cycles)
        upd_negative_zero_flags(data);
}

void CPU::LDX()
{
    //Immediate
    if(opcode == 0xA2)
    {
        data = read(PC);
        PC++;
        n_cycles++;
        X = data;       
    }

    //Zero page
    if(opcode == 0xA6)
    {
        switch(n_cycles)
        {
            case 1: { ie_zeropage<1>(); break; }
            case 2:
            {
                ie_zeropage<2>();
                X = data;
                break;
            }
        }      
    }

    //Zero page, Y
    else if(opcode == 0xB6)
    {
        switch(n_cycles)
        {
            case 1: { ie_zpxy<1>(Y); break; }
            case 2: { ie_zpxy<2>(Y); break; }
            case 3:
            {
                ie_zpxy<3>(Y);
                X = data;
                break; 
            }          
        }        
    }

    //Absolute
    else if(opcode == 0xAE)
    {
        switch(n_cycles)
        {
            case 1: { ie_abs<1>(); break; }
            case 2: { ie_abs<2>(); break; }

            case 3:
                ie_abs<3>();
                X = data;
                break;         
        } 
    }

    //Absolute, Y
    else if(opcode == 0xBE)
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
                    X = data;
                }
                                   
                break; 
            //Page crossed 
            case 4:
                ie_absxy<4>(Y);
                X = data;
                break;             
        }         
    }
    if(Instr[opcode].cycles == n_cycles)
        upd_negative_zero_flags(data);
}

void CPU::LDY()
{
    //Immediate
    if(opcode == 0xA0)
    {

        data = read(PC);
        PC++;
        n_cycles++;
        Y = data;

    }

    //Zero page
    if(opcode == 0xA4)
    {
        switch(n_cycles)
        {
            case 1: { ie_zeropage<1>(); break; }
            case 2:
            {
                ie_zeropage<2>();
                Y = data;
                break;
            }
        }        
    }

    //Zero page, X
    else if(opcode == 0xB4)
    {
        switch(n_cycles)
        {
            case 1: { ie_zpxy<1>(X); break; }
            case 2: { ie_zpxy<2>(X); break; }
            case 3:
            {
                ie_zpxy<3>(X);
                Y = data;
                break; 
            }          
        }        
    }

    //Absolute
    else if(opcode == 0xAC)
    {
        switch(n_cycles)
        {
            case 1: { ie_abs<1>(); break; }
            case 2: { ie_abs<2>(); break; }

            case 3:
                ie_abs<3>();
                Y = data;
                break;         
        } 
    }

    //Absolute, X
    else if(opcode == 0xBC)
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
                    Y = data;
                }
                                   
                break; 
            //Page crossed 
            case 4:
                ie_absxy<4>(X);
                Y = data;
                break;       
        }         
    }
    if(Instr[opcode].cycles == n_cycles)
        upd_negative_zero_flags(data);
}
void CPU::STA()
{

    //Zero page
    if(opcode == 0x85)
    {
        switch(n_cycles)
        {
            case 1: { ie_zeropage<1>(); break;}
            case 2:
            {
                write(effective_addr, Accumulator);
                n_cycles++;
                break;
            }
        }
    }
    //Zero page, X
    else if(opcode == 0x95)
    {
        switch(n_cycles)
        {
            case 1: { ie_zpxy<1>(X); break; }
            case 2: { ie_zpxy<2>(X); break; }
            case 3:
            {
                n_cycles++;
                write((zero_page_addr + X) & 0xFF, Accumulator);
                break; 
            }          
        }       
    }
    
    //Absolute
    else if(opcode == 0x8D)
    {
        switch(n_cycles)
        {
            case 1: { ie_abs<1>(); break; }
            case 2: { ie_abs<2>(); break; }
            case 3:
            {
                write(effective_addr, Accumulator);
                n_cycles++;
                break;  
            }       
        }       
    }

    //Absolute, X
    else if(opcode == 0x9D)
    {
        switch(n_cycles)
        {
            case 1: { ie_absxy<1>(X); break; }
            case 2: { ie_absxy<2>(X); break; }
            case 3: { n_cycles++; break; }
            case 4:
            {
                high_byte = ((0xFF00 & absolute_addr) >> 8);
                low_byte = 0x00FF & absolute_addr;
                h = ((high_byte ) & 0xff) << 8;
                l = (low_byte ) & 0xff;
                effective_addr = (h | l) + X;
                write(effective_addr, Accumulator);
                n_cycles++;
                break; 
            }      
        }       
    }
    //Absolute, Y
    else if(opcode == 0x99)
    {
        switch(n_cycles)
        {
            case 1: { ie_absxy<1>(Y); break; }
            case 2: { ie_absxy<2>(Y); break; }
            case 3: { n_cycles++; break; }
            case 4:
            {
                high_byte = ((0xFF00 & absolute_addr) >> 8);
                low_byte = 0x00FF & absolute_addr;
                h = ((high_byte ) & 0xff) << 8;
                l = (low_byte ) & 0xff;
                effective_addr = (h | l) + Y;
                write(effective_addr, Accumulator);
                n_cycles++;
                break; 
            }  
        }     
    }
    //Indirect, X
    else if(opcode == 0x81)
    {
        switch(n_cycles)
        {
            case 1: { ie_indx<1>(); break; }
            case 2: { ie_indx<2>(); break; }
            case 3: { ie_indx<3>(); break; }
            case 4: { ie_indx<4>(); break; }
            case 5:
                n_cycles++;
                write(effective_addr, Accumulator);
                break;       
        }    
    }

    //Indirect, Y
    else if(opcode == 0x91)
    {
        switch(n_cycles)
        {
            case 1: { ie_indy<1>(); break; }
            case 2: { ie_indy<2>(); break; }
            case 3: { ie_indy<3>(); break; }            
            case 4:
                high_byte = ((0xFF00 & absolute_addr) >> 8);
                low_byte = 0x00FF & absolute_addr;
                h = ((high_byte ) & 0xff) << 8;
                l = (low_byte ) & 0xff;
                effective_addr = (h | l) + Y;
                n_cycles++;
                break; 
            case 5:
                write(effective_addr, Accumulator);
                n_cycles++;
                break; 
        } 
    }
}

void CPU::STX()
{
    //Absolute
    if(opcode == 0x8E)
    {
        switch(n_cycles)
        {
            case 1: { ie_abs<1>(); break; } 
            case 2: { ie_abs<2>(); break; } 
            case 3:
                write(effective_addr, X);
                n_cycles++;
                break;         
        }        
    }

    //Zero page
    else if(opcode == 0x86)
    {
        switch(n_cycles)
        {
            case 1: { ie_zeropage<1>(); break; }
            case 2:
            {
                write(effective_addr, X);
                n_cycles++;
                break;
            }
        }        
    }

    //Zero page, Y
    else if (opcode == 0x96)
    {
        switch(n_cycles)
        {
            case 1: { ie_zpxy<1>(Y); break; }
            case 2: { ie_zpxy<2>(Y); break; }
            case 3:
                write((zero_page_addr + Y) & 0xFF, X);
                n_cycles++;
                break;
            
        } 
    }
}
void CPU::STY()
{
    //Zero page
    if(opcode == 0x84)
    {
        switch(n_cycles)
        {
            case 1: { ie_zeropage<1>(); break; }
            case 2:
                write(effective_addr, Y);
                n_cycles++;
                break;
        } 
    }

    //Zero page, X
    else if(opcode == 0x94)
    {
        switch(n_cycles)
        {
            case 1: { ie_zpxy<1>(X); break; }
            case 2: { ie_zpxy<2>(X); break; }
            case 3:
                write((zero_page_addr + X) & 0xFF, Y);
                n_cycles++;
                break;           
        }  
    }

    //Absolute 
    else if(opcode == 0x8C)
    {
        switch(n_cycles)
        {
            case 1: { ie_abs<1>(); break; }
            case 2: { ie_abs<2>(); break; }
            case 3:
                write(effective_addr, Y);
                n_cycles++;
                break;         
        }
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
void CPU::DEC()
{
    if(opcode == 0xCE)
    {
        //Absolute
        switch(n_cycles)
        {
            case 1: { ie_abs<1>(); break; }
            case 2: { ie_abs<2>(); break; }
            case 3: { n_cycles++; break; }
            case 4: { n_cycles++; break; }
            case 5:
                data = read(effective_addr);
                if(data == 0x00)
                    data = 0xFF;
                else 
                    data--;
                write(effective_addr, data);
                n_cycles++;
                break;
        }
    }
    //Zero page
    else if(opcode == 0xC6)
    {
        switch(n_cycles)
        {
            case 1: { ie_zeropage<1>(); break; }
            case 2: { ie_zeropage<2>(); break; }
            case 3: { n_cycles++; break; }
            case 4:
                if(data == 0x00)
                    data = 0xFF;
                else 
                    data--;
                write(effective_addr, data);
                n_cycles++;
                break;
        }
    }

    //Zero page, X

    else if(opcode == 0xD6)
    {
        switch(n_cycles)
        {
            case 1: { ie_zpxy<1>(X); break; }
            case 2: { n_cycles++; break;}
            case 3: { ie_zpxy<3>(X); break; }
            case 4: { n_cycles++; break; }
            case 5:
                if(data == 0x00)
                    data = 0xFF;
                else 
                    data--;
                write(effective_addr, data);
                n_cycles++;
                break;               
        }
    }

    //Absolute, X

    else if(opcode == 0xDE)
    {
        switch(n_cycles)
        {
            case 1: { ie_absxy<1>(X); break; }
            case 2: { ie_absxy<2>(X); break; }
            case 3:
                n_cycles++;
                break;
            case 4:
                high_byte = ((0xFF00 & absolute_addr) >> 8);
                low_byte = 0x00FF & absolute_addr;
                h = ((high_byte ) & 0xff) << 8;
                l = (low_byte ) & 0xff;
                effective_addr = (h | l) + X;
                data = read(effective_addr);
                n_cycles++;
                break;
            case 5:
                n_cycles++;
                break;
            case 6:
                if(data == 0x00)
                    data = 0xFF;
                else 
                    data--;
                write(effective_addr, data);
                n_cycles++;
                break;         
        }
    }
    if(Instr[opcode].cycles == n_cycles)
        upd_negative_zero_flags(data);
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

void CPU::INC()
{
    if(opcode == 0xEE)
    {
        //Absolute
        switch(n_cycles)
        {
            case 1: { ie_abs<1>(); break; }
            case 2: { ie_abs<2>(); break; }
            case 3: { n_cycles++; break; }
            case 4: { n_cycles++; break; }
            case 5:
                data = read(effective_addr);
                if(data == 0xFF)
                    data = 0x00;
                else 
                    data++;
                write(effective_addr, data);
                n_cycles++;
                break;
        }
    }
    //Zero page
    else if(opcode == 0xE6)
    {
        switch(n_cycles)
        {
            case 1: { ie_zeropage<1>(); break; }
            case 2: { ie_zeropage<2>(); break; }
            case 3: { n_cycles++; break; }
            case 4:
                if(data == 0xFF)
                    data = 0x00;
                else 
                    data++;
                write(effective_addr, data);
                n_cycles++;
                break;
        }
    }

    //Zero page, X

    else if(opcode == 0xF6)
    {
        switch(n_cycles)
        {
            case 1: { ie_zpxy<1>(X); break; }
            case 2: { n_cycles++; break;}
            case 3: { ie_zpxy<3>(X); break; }
            case 4: { n_cycles++; break; }
            case 5:
                if(data == 0xFF)
                    data = 0x00;
                else 
                    data++;
                write(effective_addr, data);
                n_cycles++;
                break;               
        }
    }

    //Absolute, X

    else if(opcode == 0xFE)
    {
        switch(n_cycles)
        {
            case 1: { ie_absxy<1>(X); break; }
            case 2: { ie_absxy<2>(X); break; }
            case 3:
                n_cycles++;
                break;
            case 4:
                high_byte = ((0xFF00 & absolute_addr) >> 8);
                low_byte = 0x00FF & absolute_addr;
                h = ((high_byte ) & 0xff) << 8;
                l = (low_byte ) & 0xff;
                effective_addr = (h | l) + X;
                data = read(effective_addr);
                n_cycles++;
                break;
            case 5:
                n_cycles++;
                break;
            case 6:
                if(data == 0xFF)
                    data = 0x00;
                else 
                    data++;
                write(effective_addr, data);
                n_cycles++;
                break;          
        }
    }
    upd_negative_zero_flags(data);
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

void CPU::ADC()
{
    //Immediate
    if(opcode == 0x69)
    {
        data = read(PC);
        PC++;
        n_cycles++;
        ADC_calc();            
    }

    //Zero page
    if(opcode == 0x65)
    {
        switch(n_cycles)
        {
            case 1: { ie_zeropage<1>(); break; }
            case 2:
            {
                ie_zeropage<2>();
                ADC_calc();
                break;
            }       
        }
    }

    //Zero page, X
    else if(opcode == 0x75)
    {
        switch(n_cycles)
        {
            case 1: { ie_zpxy<1>(X); break; }
            case 2: { ie_zpxy<2>(X); break; }
            case 3:
                ie_zpxy<3>(X);
                ADC_calc();
                break;           
        }        
    }

    //Absolute
    else if(opcode == 0x6D)
    {
        switch(n_cycles)
        {
            case 1: { ie_abs<1>(); break; }
            case 2: { ie_abs<2>(); break; }
            case 3:
                ie_abs<3>();
                ADC_calc();
                break;        
        } 
    }

    //Absolute, X
    else if(opcode == 0x7D)
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
                    ADC_calc();
                    page_crossing = true;
                }
                break; 
            //Page crossed 
            case 4:
                ie_absxy<4>(X);
                ADC_calc();
                break;      
        }        
    }

    //Absolute, Y
    else if(opcode == 0x79)
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
                    ADC_calc();
                    page_crossing = true;
                }
                break; 
            //Page crossed 
            case 4:
                ie_absxy<4>(Y);
                ADC_calc();
                break;      
        }
    }

    //Indirect, X
    else if(opcode == 0x61)
    {
        switch(n_cycles)
        {
            case 1: { ie_indx<1>(); break; }
            case 2: { ie_indx<2>(); break; }
            case 3: { ie_indx<3>(); break; }
            case 4: { ie_indx<4>(); break; }
            case 5:
                ie_indx<5>();
                ADC_calc();
                break;
        }
    }

    //Indirect, Y
    else if(opcode == 0x71)
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
                    ADC_calc();
                    page_crossing = true;
                }
                break; 
            case 5:
                ie_indy<5>();
                ADC_calc();
                break; 
        }       
    }

    if(Instr[opcode].cycles == n_cycles)
        upd_negative_zero_flags(Accumulator);
}
void CPU::SBC()
{
    if(opcode == 0xE9)
    {
        data = read(PC);
        data = ~data;
        n_cycles++;
        PC++;
        ADC_calc();            
    }

    //Zero page
    if(opcode == 0xE5)
    {
        switch(n_cycles)
        {
            case 1: { ie_zeropage<1>(); break; }
            case 2:
            {
                ie_zeropage<2>();
                data = ~data;
                ADC_calc();
                break;
            }       
        }
    }

    //Zero page, X
    else if(opcode == 0xF5)
    {
        switch(n_cycles)
        {
            case 1: { ie_zpxy<1>(X); break; }
            case 2: { ie_zpxy<2>(X); break; }
            case 3:
                ie_zpxy<3>(X);
                data = ~data;
                ADC_calc();
                break;           
        }        
    }

    //Absolute
    else if(opcode == 0xED)
    {
        switch(n_cycles)
        {
            case 1: { ie_abs<1>(); break; }
            case 2: { ie_abs<2>(); break; }
            case 3:
                ie_abs<3>();
                data = ~data;
                ADC_calc();
                break;        
        } 
    }

    //Absolute, X
    else if(opcode == 0xFD)
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
                    data = ~data;
                    ADC_calc();
                    page_crossing = true;
                }
                break; 
            //Page crossed 
            case 4:
                ie_absxy<4>(X);
                data = ~data;
                ADC_calc();
                break;      
        }        
    }

    //Absolute, Y
    else if(opcode == 0xF9)
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
                    data = ~data;
                    ADC_calc();
                    page_crossing = true;
                }
                break; 
            //Page crossed 
            case 4:
                ie_absxy<4>(Y);
                data = ~data;
                ADC_calc();
                break;      
        }
    }

    //Indirect, X
    else if(opcode == 0xE1)
    {
        switch(n_cycles)
        {
            case 1: { ie_indx<1>(); break; }
            case 2: { ie_indx<2>(); break; }
            case 3: { ie_indx<3>(); break; }
            case 4: { ie_indx<4>(); break; }
            case 5:
                ie_indx<5>();
                data = ~data;
                ADC_calc();
                break;
        }
    }

    //Indirect, Y
    else if(opcode == 0xF1)
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
                    data = ~data;
                    ADC_calc();
                    page_crossing = true;
                }
                break; 
            case 5:
                ie_indy<5>();
                data = ~data;
                ADC_calc();
                break; 
        }       
    }

    if(Instr[opcode].cycles == n_cycles)
        upd_negative_zero_flags(Accumulator);
}

//Logical operations

void CPU::AND()
{
    //Immediate
    if(opcode == 0x29)
    {
        data = read(PC);
        Accumulator &= data;
        PC++;
        n_cycles++;
    }

    //Zero page
    if(opcode == 0x25)
    {
        switch(n_cycles)
        {
            case 1: { ie_zeropage<1>(); break;}
            case 2:
            {
                ie_zeropage<2>();
                Accumulator &= data;
                break;
            }
        }       
    }

    //Zero page, X
    else if(opcode == 0x35)
    {
        switch(n_cycles)
        {
            case 1: { ie_zpxy<1>(X); break; }
            case 2: { ie_zpxy<2>(X); break; }
            case 3:
            {
                ie_zpxy<3>(X);
                Accumulator &= data;
                break; 
            }          
        }        
    }

    //Absolute
    else if(opcode == 0x2D)
    {
        switch(n_cycles)
        {
            case 1: { ie_abs<1>(); break; }
            case 2: { ie_abs<2>(); break; }
            case 3:
            {
                ie_abs<3>();
                Accumulator &= data;
                break;  
            }       
        } 
    }

    //Absolute, X
    else if(opcode == 0x3D)
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
                    Accumulator &= data;
                }
                break; 
            //Page crossed 
            case 4:
                ie_absxy<4>(X);
                Accumulator &= data;
                break;      
        }       
    }

    //Absolute, Y
    else if(opcode == 0x39)
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
                    Accumulator &= data;
                }
                break; 
            //Page crossed 
            case 4:
                ie_absxy<4>(Y);
                Accumulator &= data;
                break;      
        }        
    }

    //Indirect, X
    else if(opcode == 0x21)
    {
        switch(n_cycles)
        {
            case 1: { ie_indx<1>(); break;}
            case 2: { ie_indx<2>(); break;}
            case 3: { ie_indx<3>(); break;}
            case 4: { ie_indx<4>(); break;}
            case 5:
            {
                ie_indx<5>();
                Accumulator &= data;
                break;
            }
        }
    }

    //Indirect, Y
    else if(opcode == 0x31)
    {
        switch(n_cycles)
        {
            case 1: { ie_indy<1>(); break; }
            case 2: { ie_indy<2>(); break; }
            case 3: { ie_indy<3>(); break; }             
            case 4:
            {
                ie_indy<4>();
                if(!page_crossing)
                {
                    page_crossing = true;
                    Accumulator &= data;
                }
                break; 
            }
            case 5:
            {
                ie_indy<5>();
                Accumulator &= data;
                break; 
            }
        }      
    }

    if(Instr[opcode].cycles == n_cycles)
        upd_negative_zero_flags(Accumulator);    
}
void CPU::EOR()
{
    //Immediate
    if(opcode == 0x49)
    {
        data = read(PC);
        Accumulator ^= data;
        PC++;
        n_cycles++;
    }

    //Zero page
    if(opcode == 0x45)
    {
        switch(n_cycles)
        {
            case 1: { ie_zeropage<1>(); break;}
            case 2:
            {
                ie_zeropage<2>();
                Accumulator ^= data;
                break;
            }
        }       
    }

    //Zero page, X
    else if(opcode == 0x55)
    {
        switch(n_cycles)
        {
            case 1: { ie_zpxy<1>(X); break; }
            case 2: { ie_zpxy<2>(X); break; }
            case 3:
            {
                ie_zpxy<3>(X);
                Accumulator ^= data;
                break; 
            }          
        }        
    }

    //Absolute
    else if(opcode == 0x4D)
    {
        switch(n_cycles)
        {
            case 1: { ie_abs<1>(); break; }
            case 2: { ie_abs<2>(); break; }
            case 3:
            {
                ie_abs<3>();
                Accumulator ^= data;
                break;  
            }       
        } 
    }

    //Absolute, X
    else if(opcode == 0x5D)
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
                    Accumulator ^= data;
                }
                break; 
            //Page crossed 
            case 4:
                ie_absxy<4>(X);
                Accumulator ^= data;
                break;      
        }       
    }

    //Absolute, Y
    else if(opcode == 0x59)
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
                    Accumulator ^= data;
                }
                break; 
            //Page crossed 
            case 4:
                ie_absxy<4>(Y);
                Accumulator ^= data;
                break;      
        }        
    }

    //Indirect, X
    else if(opcode == 0x41)
    {
        switch(n_cycles)
        {
            case 1: { ie_indx<1>(); break;}
            case 2: { ie_indx<2>(); break;}
            case 3: { ie_indx<3>(); break;}
            case 4: { ie_indx<4>(); break;}
            case 5:
            {
                ie_indx<5>();
                Accumulator ^= data;
                break;
            }
        }
    }

    //Indirect, Y
    else if(opcode == 0x51)
    {
        switch(n_cycles)
        {
            case 1: { ie_indy<1>(); break; }
            case 2: { ie_indy<2>(); break; }
            case 3: { ie_indy<3>(); break; }             
            case 4:
            {
                ie_indy<4>();
                if(!page_crossing)
                {
                    page_crossing = true;
                    Accumulator ^= data;
                }
                break; 
            }
            case 5:
            {
                ie_indy<5>();
                Accumulator ^= data;
                break; 
            }
        }      
    }

    if(Instr[opcode].cycles == n_cycles)
        upd_negative_zero_flags(Accumulator);    
}

void CPU::ORA()
{
    //Immediate
    if(opcode == 0x09)
    {
        data = read(PC);
        Accumulator |= data;
        n_cycles++;
        PC++;
    }

    //Zero page
    if(opcode == 0x05)
    {
        switch(n_cycles)
        {
            case 1: { ie_zeropage<1>(); break;}
            case 2:
            {
                ie_zeropage<2>();
                Accumulator |= data;
                break;
            }
        }       
    }

    //Zero page, X
    else if(opcode == 0x15)
    {
        switch(n_cycles)
        {
            case 1: { ie_zpxy<1>(X); break; }
            case 2: { ie_zpxy<2>(X); break; }
            case 3:
            {
                ie_zpxy<3>(X);
                Accumulator |= data;
                break; 
            }          
        }        
    }

    //Absolute
    else if(opcode == 0x0D)
    {
        switch(n_cycles)
        {
            case 1: { ie_abs<1>(); break; }
            case 2: { ie_abs<2>(); break; }
            case 3:
            {
                ie_abs<3>();
                Accumulator |= data;
                break;  
            }       
        } 
    }

    //Absolute, X
    else if(opcode == 0x1D)
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
                    Accumulator |= data;
                    page_crossing = true;
                }
                break; 
            //Page crossed 
            case 4:

                ie_absxy<4>(X);
                Accumulator |= data;
                break;      
        }       
    }

    //Absolute, Y
    else if(opcode == 0x19)
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
                    Accumulator |= data;
                    page_crossing = true;
                }
                break; 
            //Page crossed 
            case 4:
                ie_absxy<4>(Y);
                Accumulator |= data;
                break;      
        }        
    }

    //Indirect, X
    else if(opcode == 0x01)
    {
        switch(n_cycles)
        {
            case 1: { ie_indx<1>(); break;}
            case 2: { ie_indx<2>(); break;}
            case 3: { ie_indx<3>(); break;}
            case 4: { ie_indx<4>(); break;}
            case 5:
            {
                ie_indx<5>();
                Accumulator |= data;
                break;
            }
        }
    }

    //Indirect, Y
    else if(opcode == 0x11)
    {
        switch(n_cycles)
        {
            case 1: { ie_indy<1>(); break; }
            case 2: { ie_indy<2>(); break; }
            case 3: { ie_indy<3>(); break; }             
            case 4:
            {
                ie_indy<4>();
                if(!page_crossing)
                {
                    Accumulator |= data;
                    page_crossing = true;
                }
                break; 
            }
            case 5:
            {
                ie_indy<5>();
                Accumulator |= data;
                break; 
            }
        }      
    }

    if(n_cycles == Instr[opcode].cycles)
    {
        upd_negative_zero_flags(Accumulator);
    }
}

//Shift &CPU:: Rotate Instructions

void CPU::ASL()
{
    if(opcode == 0x0A)
    {
        P = (P  & 0xFE) | ((Accumulator & 0x80) >> 7);
        Accumulator <<=1;
        Accumulator &= 0xFE;
        data = Accumulator;
        n_cycles++;
    }

    
    if(opcode == 0x0E)
    {
        //Absolute
        switch(n_cycles)
        {
            case 1: { ie_abs<1>(); break;}
            case 2: { ie_abs<2>(); break;}
            case 3: { n_cycles++; break; };
            case 4: { n_cycles++; break; };
            case 5:
                data = read(effective_addr);
                P = (P  & 0xFE) | ((data & 0x80) >> 7);
                data <<= 1;
                data &= 0xFE;
                write(effective_addr, data);
                n_cycles++;
                break;
        }
    }
    //Zero page
    else if(opcode == 0x06)
    {
        switch(n_cycles)
        {
            case 1: { ie_zeropage<1>(); break; }
            case 2: { ie_zeropage<2>(); break; }
            case 3: { n_cycles++; break; };
            case 4:
                P = (P  & 0xFE) | ((data & 0x80) >> 7);
                data <<= 1;
                data &= 0xFE;
                write(effective_addr, data);
                n_cycles++;
                break;
        }
    }

    //Zero page, X

    else if(opcode == 0x16)
    {
        switch(n_cycles)
        {
            case 1: { ie_zpxy<1>(X); break; }
            case 2: { n_cycles++; break;}
            case 3: { ie_zpxy<3>(X); break; }
            case 4: { n_cycles++; break; }
            case 5:
                P = (P  & 0xFE) | ((data & 0x80) >> 7);
                data <<= 1;
                data &= 0xFE;
                write(effective_addr, data);
                n_cycles++;
                break;               
        }
    }

    //Absolute, X

    else if(opcode == 0x1E)
    {
        switch(n_cycles)
        {
            case 1: { ie_absxy<1>(X); break; }
            case 2: { ie_absxy<2>(X); break; }
            case 3:
                n_cycles++;
                break;
            case 4:
                high_byte = ((0xFF00 & absolute_addr) >> 8);
                low_byte = 0x00FF & absolute_addr;
                h = ((high_byte ) & 0xff) << 8;
                l = (low_byte ) & 0xff;
                effective_addr = (h | l) + X;
                data = read(effective_addr);
                n_cycles++;
                break;
            case 5:
                n_cycles++;
                break;
            case 6:
                P = (P  & 0xFE) | ((data & 0x80) >> 7);
                data <<= 1;
                data &= 0xFE;
                write(effective_addr, data);
                n_cycles++;
                break;           
        }
    }
    if(Instr[opcode].cycles == n_cycles)
        upd_negative_zero_flags(data);

}
void CPU::LSR()
{
    uint8_t data = 0x00;

    if(opcode == 0x4A)
    {
        P = (P  & 0xFE) | (Accumulator & 0x01);
        Accumulator >>=1;
        Accumulator &= 0x7F;
        data = Accumulator;
        n_cycles++;
    }

    //Absolute   
    if(opcode == 0x4E)
    {
        switch(n_cycles)
        {
            case 1: { ie_abs<1>(); break; }
            case 2: { ie_abs<2>(); break; }
            case 3: { n_cycles++; break; }
            case 4: { n_cycles++; break; }
            case 5:
                data = read(effective_addr);
                P = (P  & 0xFE) | (data & 0x01);
                data >>= 1;
                data &= 0x7F;
                write(effective_addr, data);
                n_cycles++;
                break;
        }
    }
    //Zero page
    else if(opcode == 0x46)
    {
        switch(n_cycles)
        {
            case 1: { ie_zeropage<1>(); break; }
            case 2: { ie_zeropage<2>(); break; }
            case 3: { n_cycles++; break; }
            case 4:
                data = read(effective_addr);
                P = (P  & 0xFE) | (data & 0x01);
                data >>= 1;
                data &= 0x7F;
                write(effective_addr, data);
                n_cycles++;
                break;
        }
    }

    //Zero page, X
    else if(opcode == 0x56)
    {
        switch(n_cycles)
        {
            case 1: { ie_zpxy<1>(X); break; }
            case 2: { n_cycles++; break; }
            case 3: { ie_zpxy<3>(X); break; }
            case 4: {n_cycles++; break;}
            case 5:
                data = read(effective_addr);
                P = (P  & 0xFE) | (data & 0x01);
                data >>= 1;
                data &= 0x7F;
                write(effective_addr, data);
                n_cycles++;
                break;               
        }
    }

    //Absolute, X
    else if(opcode == 0x5E)
    {
        switch(n_cycles)
        {
            case 1: { ie_absxy<1>(X); break; }
            case 2: { ie_absxy<2>(X); break; }
            case 3:
                n_cycles++;
                break;
            case 4:
                high_byte = ((0xFF00 & absolute_addr) >> 8);
                low_byte = 0x00FF & absolute_addr;
                h = ((high_byte ) & 0xff) << 8;
                l = (low_byte ) & 0xff;
                effective_addr = (h | l) + X;
                data = read(effective_addr);
                n_cycles++;
                break;
            case 5:
                n_cycles++;
                break;
            case 6:
                data = read(effective_addr);
                P = (P  & 0xFE) | (data & 0x01);
                data >>= 1;
                data &= 0x7F;
                write(effective_addr, data);
                n_cycles++;
                break;           
        }
    }   
    if(Instr[opcode].cycles == n_cycles)
        upd_negative_zero_flags(data);
}

void CPU::ROL()
{
    uint8_t aux;
    uint8_t data = 0x00;

    if(opcode == 0x2A)
    {
        aux = (Accumulator >> 7) & 0x01;
        Accumulator <<= 1;
        Accumulator = (Accumulator & 0xFE) | (P & 0x01);
        P = (P  & 0xFE) | (aux & 0x01);
        data = Accumulator;
        n_cycles++;
    }
    //Absolute
    if(opcode == 0x2E)
    {
        switch(n_cycles)
        {
            case 1: { ie_abs<1>(); break; }
            case 2: { ie_abs<2>(); break; }
            case 3: { n_cycles++; break; }
            case 4: { n_cycles++; break; }
            case 5:
                data = read(effective_addr);
                aux = (data >> 7) & 0x01;
                data <<= 1;
                data = (data & 0xFE) | (P & 0x01);
                P = (P  & 0xFE) | (aux & 0x01);
                write(effective_addr, data);
                n_cycles++;
                break;
        }
    }
    //Zero page
    else if(opcode == 0x26)
    {
        switch(n_cycles)
        {
            case 1: { ie_zeropage<1>(); break; }
            case 2: { ie_zeropage<2>(); break; }
            case 3: { n_cycles++; break; }
            case 4:
                data = read(effective_addr);
                aux = (data >> 7) & 0x01;
                data <<= 1;
                data = (data & 0xFE) | (P & 0x01);
                P = (P  & 0xFE) | (aux & 0x01);
                write(effective_addr, data);
                n_cycles++;
                break;
        }
    }

    //Zero page, X
    else if(opcode == 0x36)
    {
        switch(n_cycles)
        {
            case 1: { ie_zpxy<1>(X); break; }
            case 2: { n_cycles++; break; }
            case 3: { ie_zpxy<3>(X); break; }
            case 4: {n_cycles++; break;}
            case 5:
                data = read(effective_addr);
                aux = (data >> 7) & 0x01;
                data <<= 1;
                data = (data & 0xFE) | (P & 0x01);
                P = (P  & 0xFE) | (aux & 0x01);
                write(effective_addr, data);
                n_cycles++;
                break;               
        }
    }

    //Absolute, X

    else if(opcode == 0x3E)
    {
        switch(n_cycles)
        {
            case 1: { ie_absxy<1>(X); break; }
            case 2: { ie_absxy<2>(X); break; }
            case 3:
                n_cycles++;
                break;
            case 4:
                high_byte = ((0xFF00 & absolute_addr) >> 8);
                low_byte = 0x00FF & absolute_addr;
                h = ((high_byte ) & 0xff) << 8;
                l = (low_byte ) & 0xff;
                effective_addr = (h | l) + X;
                data = read(effective_addr);
                n_cycles++;
                break;
            case 5:
                n_cycles++;
                break;
            case 6:
                data = read(effective_addr);
                aux = (data >> 7) & 0x01;
                data <<= 1;
                data = (data & 0xFE) | (P & 0x01);
                P = (P  & 0xFE) | (aux & 0x01);
                write(effective_addr, data);
                n_cycles++;
                break;             
        }
    }
    if(Instr[opcode].cycles == n_cycles)
        upd_negative_zero_flags(data);
}

void CPU::ROR()
{
    uint8_t aux = false;
    uint8_t data = 0x00;

    if(opcode == 0x6A)
    {
        aux = Accumulator & 0x01;
        Accumulator >>= 1;
        Accumulator = (Accumulator & 0x7F) | ((P & 0x01) << 7);
        data = Accumulator;
        P = (P & 0xFE) | (aux & 0x01);
        n_cycles++;
    }

    if(opcode == 0x6E)
    {
        switch(n_cycles)
        {
            case 1: { ie_abs<1>(); break; }
            case 2: { ie_abs<2>(); break; }
            case 3: { n_cycles++; break; }
            case 4: { n_cycles++; break; }
            case 5:
                data = read(effective_addr);
                aux = data & 0x01;
                data >>= 1;
                data = (data & 0x7F) | ((P & 0x01) << 7);
                data = data;
                P = (P & 0xFE) | (aux & 0x01);
                write(effective_addr, data);
                n_cycles++;
                break;
        }
    }

    //Zero page
    else if(opcode == 0x66)
    {
        switch(n_cycles)
        {
            case 1: { ie_zeropage<1>(); break; }
            case 2: { ie_zeropage<2>(); break; }
            case 3: { n_cycles++; break; }
            case 4:
                data = read(effective_addr);
                aux = data & 0x01;
                data >>= 1;
                data = (data & 0x7F) | ((P & 0x01) << 7);
                data = data;
                P = (P & 0xFE) | (aux & 0x01);
                write(effective_addr, data);
                n_cycles++;
                break;
        }
    }

    //Zero page, X
    else if(opcode == 0x76)
    {
        switch(n_cycles)
        {
            case 1: { ie_zpxy<1>(X); break; }
            case 2: { n_cycles++; break; }
            case 3: { ie_zpxy<3>(X); break; }
            case 4: {n_cycles++; break;}
            case 5:
                data = read(effective_addr);
                aux = data & 0x01;
                data >>= 1;
                data = (data & 0x7F) | ((P & 0x01) << 7);
                data = data;
                P = (P & 0xFE) | (aux & 0x01);
                write(effective_addr, data);
                n_cycles++;
                break;               
        }
    }

    //Absolute, X

    else if(opcode == 0x7E)
    {
        switch(n_cycles)
        {
            case 1: { ie_absxy<1>(X); break; }
            case 2: { ie_absxy<2>(X); break; }
            case 3:
                n_cycles++;
                break;
            case 4:
                high_byte = ((0xFF00 & absolute_addr) >> 8);
                low_byte = 0x00FF & absolute_addr;
                h = ((high_byte ) & 0xff) << 8;
                l = (low_byte ) & 0xff;
                effective_addr = (h | l) + X;
                data = read(effective_addr);
                n_cycles++;
                break;
            case 5:
                n_cycles++;
                break;
            case 6:
                data = read(effective_addr);
                aux = data & 0x01;
                data >>= 1;
                data = (data & 0x7F) | ((P & 0x01) << 7);
                data = data;
                P = (P & 0xFE) | (aux & 0x01);
                write(effective_addr, data);
                n_cycles++;
                break;            
        }
    }
    upd_negative_zero_flags(data);
}

//Comparison instructions

void CPU::CMP()
{
    //Immediate
    if(opcode == 0xC9)
    {
        data = read(PC);
        CMP_calc(Accumulator);
        PC++;
        n_cycles++;      
    }

    //Zero page
    if(opcode == 0xC5)
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

    //Zero page, X
    else if(opcode == 0xD5)
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

    //Absolute
    else if(opcode == 0xCD)
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

    //Absolute, X
    else if(opcode == 0xDD)
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
                    CMP_calc(Accumulator);
                }
                break; 
            //Page crossed 
            case 4:
                ie_absxy<4>(X);
                CMP_calc(Accumulator);
                break;      
        }       
    }

    //Absolute, Y
    else if(opcode == 0xD9)
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
                    CMP_calc(Accumulator);
                }
                break; 
            //Page crossed 
            case 4:
                ie_absxy<4>(Y);
                CMP_calc(Accumulator);
                break;      
        }        
    }

    //Indirect, X
    else if(opcode == 0xC1)
    {
        switch(n_cycles)
        {
            case 1: { ie_indx<1>(); break;}
            case 2: { ie_indx<2>(); break;}
            case 3: { ie_indx<3>(); break;}
            case 4: { ie_indx<4>(); break;}
            case 5:
            {
                ie_indx<5>();
                CMP_calc(Accumulator);
                break;
            }
        }
    }

    //Indirect, Y
    else if(opcode == 0xD1)
    {
        switch(n_cycles)
        {
            case 1: { ie_indy<1>(); break; }
            case 2: { ie_indy<2>(); break; }
            case 3: { ie_indy<3>(); break; }             
            case 4:
            {
                ie_indy<4>();
                if(!page_crossing)
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
}
void CPU::CPX()
{
    //Immediate
    if(opcode == 0xE0)
    {
        data = read(PC);
        CMP_calc(X);
        PC++;
        n_cycles++;   
    }

    //Zero page
    if(opcode == 0xE4)
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

    //Absolute
    else if(opcode == 0xEC)
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
}
void CPU::CPY()
{
     //Immediate
    if(opcode == 0xC0)
    {
        data = read(PC);
        CMP_calc(Y);
        PC++;
        n_cycles++;   
    }

    //Zero page
    if(opcode == 0xC4)
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

    //Absolute
    else if(opcode == 0xCC)
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
                n_cycles++;           
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
                n_cycles++;           
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
                n_cycles++;           
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
                n_cycles++;           
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
                n_cycles++;           
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
                n_cycles++;           
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
                n_cycles++;           
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
                n_cycles++;           
            break;
        case 3:         
            n_cycles++;
            break;
    }
}

//Jumps &CPU:: Subroutines


void CPU::JMP()
{

    //Absolute
    if(opcode == 0x4C)
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

    else
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

    if(bus->get_nmi() || (P & 0x4) == 0)
    {
        switch (n_cycles)
        {
            case 1: { 
                n_cycles++; 
                break; }
            case 2:
                if(!bus->get_nmi())
                    PC++;

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
                write(0x100 + SP, P | 0x10);
                P |= 0x04;
                SP--;
                n_cycles++;
                break;
            case 5:
                PC = 0x0000;
                if(bus->get_nmi())
                    PC |= read(0xFFFA);
                else
                    PC |= read(0xFFFE);
                n_cycles++;
                break;
            case 6:
                if(bus->get_nmi())
                {
                    PC |= (read(0xFFFB) << 8);
                    bus->set_nmi(false);
                }
                else
                {
                    PC |= (read(0xFFFF) << 8);
                }
                n_cycles++;
                break;
        }
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
            P |= 0x20;
            P &= 0xEF;
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

void CPU::BIT()
{
    uint8_t aux;

    //Zero Page
    if(opcode == 0x24)
    {
        switch(n_cycles)
        {
            case 1: { ie_zeropage<1>(); break; }
            case 2:
            {
                ie_zeropage<2>();
                aux = data & Accumulator;
                if (aux == 0x00)
                    P = P | 0x02;
                else
                    P =  P  & 0xFD;
                P = (P  & 0x7F) | (data & 0x80);
                P = (P  & 0xBF) | (data & 0x40);
                break;
            }
        }       
    }
    //Absolute
    else if(opcode == 0x2C)
    {
        switch(n_cycles)
        {
            case 1: { ie_abs<1>(); break; }
            case 2: { ie_abs<2>(); break; }
            case 3:
            {
                ie_abs<3>();
                aux = data & Accumulator;
                if (aux == 0x00)
                    P = P | 0x02;
                else
                    P =  P  & 0xFD;
                P = (P  & 0x7F) | (data & 0x80);
                P = (P  & 0xBF) | (data & 0x40);
                break; 
            }       
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

bool CPU::open_file(std::string name)
{
    std::ifstream f(name);
    bool ok = true;

    if(!f.is_open())
    {
        ok = false;
        std::cerr<< "Hubo un error";
    }

    f >> json_data;
    f.close();

    return ok;  
}

void CPU::load(int line)
{
    PC = json_data[line]["initial"]["pc"];
    Accumulator = json_data[line]["initial"]["a"];
    X = json_data[line]["initial"]["x"];
    Y = json_data[line]["initial"]["y"];
    SP = json_data[line]["initial"]["s"];
    P = json_data[line]["initial"]["p"];

    for(int i = 0; i < (int)json_data[line]["initial"]["ram"].size(); i++)       
        write((uint16_t)json_data[line]["initial"]["ram"][i][0], (uint8_t)json_data[line]["initial"]["ram"][i][1]);    
}

bool CPU::finished()
{
    bool aux = finish;
    finish = false;
    return aux;
}
bool CPU::compare(int line)
{
    bool ok = true;
   

    ok &= ((int)PC == json_data[line]["final"]["pc"]);
    ok &= ((int)Accumulator == json_data[line]["final"]["a"]);
    ok &= ((int)X == json_data[line]["final"]["x"]);
    ok &= ((int)Y == json_data[line]["final"]["y"]);
    ok &= ((int)SP == json_data[line]["final"]["s"]);
    ok &= ((int)P == json_data[line]["final"]["p"]);

    for(int i = 0; i < (int)json_data[line]["final"]["ram"].size(); i++)
        ok &= read((uint16_t)json_data[line]["final"]["ram"][i][0]) == (uint8_t)json_data[line]["final"]["ram"][i][1];

    if (!ok)
    {
        std::cout << "PC: "<< (int)PC << std::endl;
        std::cout << "Acc: "<< (int)Accumulator<< std::endl;
        std::cout << "X: "<< (int)X<< std::endl;
        std::cout << "Y: "<< (int)Y<< std::endl;
        std::cout << "SP: "<< (int)SP<< std::endl;
        std::cout << "P: "<< (int)P<< std::endl;
    }

    return ok;
}

void CPU::reset()
{
    Accumulator = X = Y = 0;
    PC = (read(0xFFFC) | (read(0xFFFD) << 8));
    SP = 0xFD;
    P = 0x04;
}