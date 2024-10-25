// =======================================================================
// project: TraffickLight
// main module -- app entry point
// =======================================================================
#include "qpc.h"
#include "trafficlight.h"
#include "bsp.h"
#include "bsp-gui.h"

#if defined main
#undef main
#endif
int main (int argc, char *argv[])
{
#if defined QWIN_GUI
    return startGui(argc, argv);
#else
    BSP_HW_init();
    QF_init();
    BSP_init(argc, argv);

#if defined Q_UTEST
    // pause execution of the test and wait for the test script to continue
    QS_TEST_PAUSE();
#endif
    
    return tlMain(); 
#endif
}
