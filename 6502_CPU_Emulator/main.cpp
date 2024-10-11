#include "Cpu.h"

#include <gtest/gtest.h>

class M6502Test1 : public testing::Test {
public:
	Mem mem;
	Cpu cpu;

	virtual void SetUp() {
		cpu.reset(mem);
	}

	virtual void TearDown() {

	}

};

TEST_F(M6502Test1, RunALittleInlineProgram)
{
	// start - inline a little program 
	mem[0xFFFC] = Cpu::INS_JSR;
	mem[0xFFFD] = 0x42;
	mem[0xFFFE] = 0x42;
	mem[0x4242] = Cpu::INS_LDA_IM;
	mem[0x4243] = 0x84;
	// start - inline a little program 
	cpu.execute(9, mem);
}

#if 0
int main()
{
	Mem mem;
	CPU cpu;
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
	printf("Running main() from %s \n", __FILE__);
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
#endif
