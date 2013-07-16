#include "OS.h"
#include "../PDP11/regs.h"
#include <string.h>

using namespace UnixV6;

void OS::sighandler2(int sig) {
    uint16_t r[8];
    memcpy(r, cpu.r, sizeof (r));
    bool Z = cpu.Z, N = cpu.N, C = cpu.C, V = cpu.V;
    cpu.write16((cpu.SP -= 2), cpu.PC);
    cpu.PC = sighandlers[sig];
    while (!cpu.hasExited && !(cpu.PC == PC && cpu.SP == SP)) {
        cpu.run1();
    }
    if (!cpu.hasExited) {
        memcpy(cpu.r, r, sizeof (r));
        cpu.Z = Z;
        cpu.N = N;
        cpu.C = C;
        cpu.V = V;
    }
}
