#pragma once 

#include <stdio.h>
#include <stdlib.h>
#include <cstdint>

// http://www.6502.org/users/obelisk/6502/index.html

// The processor is little endian
// Zero page : Fancy way of saying first 256 bytes of memory
// The Second Page of memory is reserved for stack memory ( 256 bytes )

// The Registers 
// -----------------------------------------------------------------------
// Program Counter : Its' the pointer to the address that cpu is executing the code for. Whatever address is in there next instruction will be that.
// Stack Pointer
// Processor Registers : A quickly accessible location available to processor ( For our case Accumulator, X, Y).
// Processor Status :   Bunch of bits that have flags in it. When certain instructions happen they get set.

typedef uint8_t Byte;
typedef uint16_t Word;

struct Mem
{
	static constexpr uint32_t MAX_MEM = 1024 * 64;
	Byte Data[MAX_MEM];

	void init()
	{
		for (uint32_t i = 0; i < MAX_MEM; i++)
		{
			Data[i] = 0;
		}
	}

	Byte operator[](uint32_t address) const
	{
		// assert here address is < MAX_MEM
		return Data[address];
	}

	Byte& operator[](uint32_t address)
	{
		// assert here address is < MAX_MEM
		return Data[address];
	}

	void writeWord(Word value, uint32_t address, uint32_t& cycles)
	{
		Data[address] = value & 0xFF;
		Data[address + 1] = (value >> 8);
		cycles -= 2;
	}

};

struct Cpu
{
	Word PC;		// Program Counter
	Byte SP;		// Stack Pointer

	Byte A, X, Y;	// Processor Registers

	// Processor Status Flags
	Byte C : 1;		// ( Carry Flag )
	Byte Z : 1;		// ( Zero Flag )
	Byte I : 1;		// ( Interrupt Disable )
	Byte D : 1;		// ( Decimal Mode )
	Byte B : 1;		// ( Break Command )
	Byte V : 1;		// ( Overflow Flag )
	Byte N : 1;		// ( Negative Flag )

	void reset(Mem& memory)
	{
		// Cpu needs to reset. We are kinda faking it not doing the whole process 
		// https://www.c64-wiki.com/wiki/Reset_(Process) We give the values depending on here 

		PC = 0xFFFC;
		SP = 0x0100;

		C = 0;
		Z = 0;
		I = 0;
		D = 0;
		B = 0;
		V = 0;
		N = 0;

		A = 0;
		X = 0;
		Y = 0;

		memory.init();
	}

	Byte fetchByte(uint32_t& cycles, Mem& memory)
	{
		Byte data = memory[PC];
		PC++;
		cycles--;
		return data;
	}

	Word fetchWord(uint32_t& cycles, Mem& memory)
	{
		// 6502 is little endian
		Word data = memory[PC];
		PC++;

		data |= (memory[PC] << 8);
		PC++;

		cycles += 2;

		// if you want to handle endianness
		// you would have to swap bytes
		// if (PLATFORM_BIG_ENDIAN)
		// SwapBytesInWord(data)

		return data;
	}


	Byte readByte(uint32_t& cycles, Byte address, Mem& memory)
	{
		Byte data = memory[address];
		cycles--;
		return data;
	}

	// opcodes
	static constexpr Byte INS_LDA_IM = 0xA9;
	static constexpr Byte INS_LDA_ZP = 0xA5;
	static constexpr Byte INS_LDA_ZPX = 0xB5;
	static constexpr Byte INS_JSR = 0x20;

	void ldaSetStatus()
	{
		Z = (A == 0);
		N = (A & 0b10000000) > 0;
	}

	void execute(uint32_t cycles, Mem& memory)
	{
		while (cycles > 0)
		{
			Byte instruction = fetchByte(cycles, memory);

			switch (instruction)
			{
			case INS_LDA_IM:
			{
				Byte value = fetchByte(cycles, memory);
				A = value;
				ldaSetStatus();
			}
			break;
			case INS_LDA_ZP:
			{
				Byte zeroPageAddress = fetchByte(cycles, memory);
				A = readByte(cycles, zeroPageAddress, memory);
				ldaSetStatus();
			}
			break;
			case INS_LDA_ZPX:
			{
				// Todo will be handling overflow case later
				Byte zeroPageAddress = fetchByte(cycles, memory);
				zeroPageAddress += X;
				cycles--;
				A = readByte(cycles, zeroPageAddress, memory);
				ldaSetStatus();
			}
			break;
			case INS_JSR:
			{
				Word subAddr = fetchWord(cycles, memory);
				memory.writeWord(PC - 1, SP, cycles);
				PC = subAddr;
				cycles--;
			}
			default:
				printf("Instruction not handled %d \n", instruction);
				break;
			}
		}
	}
};