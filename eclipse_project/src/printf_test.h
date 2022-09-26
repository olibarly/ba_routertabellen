#ifndef PRINTF_TEST
#define PRINTF_TEST

#include "rodos.h"

class PRINTF_Test : StaticThread<> {
public:
	PRINTF_Test(const char* name);

private:
	void init();
	void run();
};


#endif
