// =======================================================================
// project: TrafficLight
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
    return tlMain(argc, argv);
#endif
}
