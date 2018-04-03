/*$file${./Source::bsp.h} ##################################################*/
/*
* Model: TrafficLight.qm
* File:  /home/stenny/Projects/TrafficLight/Source/bsp.h
*
* This code has been generated by QM tool (https://state-machine.com/qm).
* DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*/
/*$endhead${./Source::bsp.h} ###############################################*/
#ifndef bsp_gui_h
#define bsp_gui_h

void guiSetLight(eTLidentity_t id, eTLlight_t light);
void guiSetPedLed(guint16 status);
guint16 guiGetButton(void);

int startGui(int argc, char *argv[]);
int main_gui(int argc, char *argv[]);

#endif /* bsp_gui_h */
