#include "PID.h"
#include "ResolverToDigital.h"
#include <xc.h>

void main(void)
{
    PIDInit();
    calculatePID(SetAngle, CurrentAngle);
}