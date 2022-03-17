#include "qpc.h"
#include "trafficlight.h"
#include "bsp.h"

#ifdef Q_SPY
#include "qs_port.h"
#endif

StdSenderId_t const l_SysTick_Handler = {1},
                    l_Button_Handler  = {2};
