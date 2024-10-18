#include "Cpu.h"
#include <gtest/gtest.h>

class M6502Test1 : public testing::Test
{
public:
	Mem mem;
	Cpu cpu;

	virtual void SetUp()
	{
		cpu.reset(mem);
	}

	virtual void TearDown()
	{

	}
};

static void verifyUnmodifiedFlagsFromLDA(const Cpu& cpu, const Cpu& cpuCopy)
{
	EXPECT_EQ(cpu.C, cpuCopy.C);
	EXPECT_EQ(cpu.I, cpuCopy.I);
	EXPECT_EQ(cpu.D, cpuCopy.D);
	EXPECT_EQ(cpu.B, cpuCopy.B);
	EXPECT_EQ(cpu.V, cpuCopy.V);
}

TEST_F(M6502Test1, CpuDoesNothingWhenWeExecuteZeroCycles)
{
	// given:
	constexpr int32_t numCycles = 0;

	// when:
	int32_t cyclesUsed = cpu.execute(numCycles, mem);

	// then:
	EXPECT_EQ(cyclesUsed, 0);
}

TEST_F(M6502Test1, CpuCanExecuteMoreCyclesThanRequestedIfRequiredByTheInstruction)
{
	// given:
	mem[0xFFFC] = Cpu::INS_LDA_IM;
	mem[0xFFFD] = 0x84;
	Cpu cpuCopy = cpu;
	constexpr int32_t numCycles = 1;
	
	// when:
	int32_t cyclesUsed = cpu.execute(numCycles, mem);

	// then:
	EXPECT_EQ(cyclesUsed, 2);
}

TEST_F(M6502Test1, ExecutingABadInstructionDoesNotPutUsInAnInfiniteLoop)
{
	// given:
	mem[0xFFFC] = 0x0; // invalid instruction
	mem[0xFFFD] = 0x0;
	Cpu cpuCopy = cpu;
	constexpr int32_t numCycles = 1;

	// when:
	int32_t cyclesUsed = cpu.execute(numCycles, mem);

	// then:
	EXPECT_EQ(cyclesUsed, numCycles);
}

TEST_F(M6502Test1, LDAImmediateCanLoadAValueIntoRegister)
{
	// given:
	mem[0xFFFC] = Cpu::INS_LDA_IM;
	mem[0xFFFD] = 0x84;
	Cpu cpuCopy = cpu;

	// when:
	int32_t cyclesUsed = cpu.execute(2, mem);

	// then:
	EXPECT_EQ(cpu.A, 0x84); // checks if the value equals
	EXPECT_EQ(cyclesUsed, 2);
	EXPECT_FALSE(cpu.Z);
	EXPECT_TRUE(cpu.N);
	verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

TEST_F(M6502Test1, LDAImmediateCanAffectTheZeroFlag)
{
	// given:
	cpu.A = 0x44;
	mem[0xFFFC] = Cpu::INS_LDA_IM;
	mem[0xFFFD] = 0x00;
	Cpu cpuCopy = cpu;

	// when:
	cpu.execute(2, mem);

	// then:
	EXPECT_TRUE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}


TEST_F(M6502Test1, LDAZeroPageCanLoadAValueIntoRegister)
{
	// given:
	mem[0xFFFC] = Cpu::INS_LDA_ZP;
	mem[0xFFFD] = 0x42;
	mem[0x0042] = 0x37;

	// when:
	Cpu cpuCopy = cpu;
	int32_t cyclesUsed = cpu.execute(3, mem);

	// then:
	EXPECT_EQ(cpu.A, 0x37); // checks if the value equals
	EXPECT_EQ(cyclesUsed, 3);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

TEST_F(M6502Test1, LDAZeroPageXCanLoadAValueIntoRegister)
{
	// given:
	cpu.X = 5;
	mem[0xFFFC] = Cpu::INS_LDA_ZPX;
	mem[0xFFFD] = 0x42;
	// We add the current value to new value and get to that address 
	mem[0x0047] = 0x37;

	// when:
	Cpu cpuCopy = cpu;
	int32_t cyclesUsed = cpu.execute(4, mem);

	// then:
	EXPECT_EQ(cpu.A, 0x37); // checks if the value equals
	EXPECT_EQ(cyclesUsed, 4);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

TEST_F(M6502Test1, LDAZeroPageXCanLoadAValueIntoRegisterWhenItWraps)
{
	// given:
	cpu.X = 0xFF;
	mem[0xFFFC] = Cpu::INS_LDA_ZPX;
	mem[0xFFFD] = 0x80;
	// We add the current value to new value and get to that address 
	mem[0x007F] = 0x37;

	// when:
	Cpu cpuCopy = cpu;
	int32_t cyclesUsed = cpu.execute(4, mem);

	// then:
	EXPECT_EQ(cpu.A, 0x37); // checks if the value equals
	EXPECT_EQ(cyclesUsed, 4);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

TEST_F(M6502Test1, LDAAbsoluteCanLoadValueIntoTheARegister)
{
	// given:
	mem[0xFFFC] = Cpu::INS_LDA_ABS;
	mem[0xFFFD] = 0x80;
	mem[0xFFFE] = 0x44; // 0x4480
	mem[0x4480] = 0x37;
	constexpr int32_t expectedCycles = 4;
	Cpu cpuCopy = cpu;

	// when:
	int32_t cyclesUsed = cpu.execute(expectedCycles, mem);

	// then:
	EXPECT_EQ(cpu.A, 0x37); // checks if the value equals
	EXPECT_EQ(cyclesUsed, expectedCycles);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

TEST_F(M6502Test1, LDAAbsoluteXCanLoadValueIntoTheARegister)
{
	// given:
	cpu.X = 0X1;
	mem[0xFFFC] = Cpu::INS_LDA_ABSX;
	mem[0xFFFD] = 0x80;
	mem[0xFFFE] = 0x44; // 0x4480
	mem[0x4480] = 0x37; 
	constexpr int32_t expectedCycles = 4;
	Cpu cpuCopy = cpu;

	// when:
	int32_t cyclesUsed = cpu.execute(expectedCycles, mem);

	// then:
	EXPECT_EQ(cpu.A, 0x37); // checks if the value equals
	EXPECT_EQ(cyclesUsed, expectedCycles);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

TEST_F(M6502Test1, LDAAbsoluteXCanLoadValueIntoTheARegisterWhenItCrossesAPageBoundary)
{
	// given:
	cpu.X = 0XFF;
	mem[0xFFFC] = Cpu::INS_LDA_ABSX;
	mem[0xFFFD] = 0x02;
	mem[0xFFFE] = 0x44; // 0x4402
	mem[0x4501] = 0x37; // 0x4402 + 0xFF cross page boundary
	constexpr int32_t expectedCycles = 5;
	Cpu cpuCopy = cpu;

	// when:
	int32_t cyclesUsed = cpu.execute(expectedCycles, mem);

	// then:
	EXPECT_EQ(cpu.A, 0x37); // checks if the value equals
	EXPECT_EQ(cyclesUsed, expectedCycles);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

TEST_F(M6502Test1, LDAAbsoluteYCanLoadValueIntoTheARegister)
{
	// given:
	cpu.Y = 0X1;
	mem[0xFFFC] = Cpu::INS_LDA_ABSY;
	mem[0xFFFD] = 0x80;
	mem[0xFFFE] = 0x44; // 0x4480
	mem[0x4480] = 0x37;
	constexpr int32_t expectedCycles = 4;
	Cpu cpuCopy = cpu;

	// when:
	int32_t cyclesUsed = cpu.execute(expectedCycles, mem);

	// then:
	EXPECT_EQ(cpu.A, 0x37); // checks if the value equals
	EXPECT_EQ(cyclesUsed, expectedCycles);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

TEST_F(M6502Test1, LDAAbsoluteYCanLoadValueIntoTheARegisterWhenItCrossesAPageBoundary)
{
	// given:
	cpu.Y = 0XFF;
	mem[0xFFFC] = Cpu::INS_LDA_ABSY;
	mem[0xFFFD] = 0x02;
	mem[0xFFFE] = 0x44; // 0x4402
	mem[0x4501] = 0x37; // 0x4402 + 0xFF cross page boundary
	constexpr int32_t expectedCycles = 5;
	Cpu cpuCopy = cpu;

	// when:
	int32_t cyclesUsed = cpu.execute(expectedCycles, mem);

	// then:
	EXPECT_EQ(cpu.A, 0x37); // checks if the value equals
	EXPECT_EQ(cyclesUsed, expectedCycles);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

TEST_F(M6502Test1, LDAIndirectXCanLoadAValueIntoTheARegister)
{
	// given:
	cpu.X = 0X04;
	mem[0xFFFC] = Cpu::INS_LDA_INDX;
	mem[0xFFFD] = 0x02;
	mem[0x0006] = 0x00; // 0x2 + 0x4
	mem[0x0007] = 0x80; 
	mem[0x8000] = 0x37;
	constexpr int32_t expectedCycles = 6;
	Cpu cpuCopy = cpu;

	// when:
	int32_t cyclesUsed = cpu.execute(expectedCycles, mem);

	// then:
	EXPECT_EQ(cpu.A, 0x37); // checks if the value equals
	EXPECT_EQ(cyclesUsed, expectedCycles);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

TEST_F(M6502Test1, LDAIndirectYCanLoadAValueIntoTheARegister)
{
	// given:
	cpu.Y = 0X04;
	mem[0xFFFC] = Cpu::INS_LDA_INDY;
	mem[0xFFFD] = 0x02;
	mem[0x0002] = 0x00; 
	mem[0x0003] = 0x80;
	mem[0x8004] = 0x37; // 0x8000 + 0x0004
	constexpr int32_t expectedCycles = 5;
	Cpu cpuCopy = cpu;

	// when:
	int32_t cyclesUsed = cpu.execute(expectedCycles, mem);

	// then:
	EXPECT_EQ(cpu.A, 0x37); // checks if the value equals
	EXPECT_EQ(cyclesUsed, expectedCycles);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}

TEST_F(M6502Test1, LDAIndirectYCanLoadAValueIntoTheARegisterWhenItCrossesAPage)
{
	// given:
	cpu.Y = 0XFF;
	mem[0xFFFC] = Cpu::INS_LDA_INDY;
	mem[0xFFFD] = 0x02;
	mem[0x0002] = 0x02;
	mem[0x0003] = 0x80;
	mem[0x8101] = 0x37; // 0x8002 + 0xFF
	constexpr int32_t expectedCycles = 6;
	Cpu cpuCopy = cpu;

	// when:
	int32_t cyclesUsed = cpu.execute(expectedCycles, mem);

	// then:
	EXPECT_EQ(cpu.A, 0x37); // checks if the value equals
	EXPECT_EQ(cyclesUsed, expectedCycles);
	EXPECT_FALSE(cpu.Z);
	EXPECT_FALSE(cpu.N);
	verifyUnmodifiedFlagsFromLDA(cpu, cpuCopy);
}


#if 0
int main()
{
	Mem mem;
	Cpu cpu;
	cpu.reset(mem);
	// start - inline a little program 
	mem[0xFFFC] = Cpu::INS_JSR;
	mem[0xFFFD] = 0x42;
	mem[0xFFFE] = 0x42;
	mem[0x4242] = Cpu::INS_LDA_IM;
	mem[0x4243] = 0x84;
	// start - inline a little program 
	cpu.execute(9, mem);
	return 0;
}
#endif

#if GTEST_OS_ESP8266 || GTEST_OS_ESP32
#if GTEST_OS_ESP8266
extern "C" {
#endif
	void setup() {
		testing::InitGoogleTest();
	}

	void loop() { RUN_ALL_TESTS(); }

#if GTEST_OS_ESP8266
}
#endif

#else

GTEST_API_ int main(int argc, char** argv) {
	printf("Running main() from %s\n", __FILE__);
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
#endif