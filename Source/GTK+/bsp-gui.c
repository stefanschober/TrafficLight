/*****************************************************************************
 * Product: DPP example, Win32-GUI
 * Last updated for version 5.9.0
 * Last updated on  2017-04-15
 *
 *                    Q u a n t u m     L e a P s
 *                    ---------------------------
 *                    innovating embedded systems
 *
 * Copyright (C) Quantum Leaps, LLC. All rights reserved.
 *
 * This program is open source software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Alternatively, this program may be distributed and modified under the
 * terms of Quantum Leaps commercial licenses, which expressly supersede
 * the GNU General Public License and are specifically designed for
 * licensees interested in retaining the proprietary status of their code.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact information:
 * https://state-machine.com
 * mailto:info@state-machine.com
 *****************************************************************************/
#include "qpc.h"
#include "trafficlight.h"
#include "bsp-gui.h"
#include "bsp.h"

#include <gtk/gtk.h>

// Q_DEFINE_THIS_FILE

#define WINDOW_W    350
#define WINDOW_H    250

// LOCAL functions ----------------------------------------------------------
static void theApp (GtkApplication* app, gpointer user_data);
// called when button PEDESTRIAN is clicked
static void on_btn_ped_clicked(void);
static void on_label_draw(GtkWidget *widget, cairo_t *cr, gpointer data);
// timeout
static gboolean count_handler(GtkWidget *widget);
static gboolean event_handler(GtkWidget *widget);
static gboolean initTrafficLights(GtkWidget *widget);
static void     appThread(GTask *task, gpointer source_object, gpointer task_data, GCancellable *cancellable);
static void     appThreadReadyCallback (GObject *source_object, GAsyncResult *res, gpointer user_data);

// static struct termios l_tsav; /* structure with saved terminal attributes */
// static guint8 keyPressed = 0;
static GMutex myMutex;
static GRand  *myRnd;
static guint32 rndCountDown = 120;
GtkWidget       *trafficLights[MaxIdentity][NO_LIGHT][2], *ledWhite, *ledRed;
static struct {
        uint8_t changed;
        eTLlight_t light;
} setLight[MaxIdentity] = {
                           {0, NO_LIGHT},
                           {0, NO_LIGHT},
                           {0, NO_LIGHT}
};
static struct {
        uint8_t  changed;
        uint8_t  status;
} setLed = {0, 0};

typedef struct {
	int argc;
	char **argv;
} appArgs_t;
#define APP_ARGS(o) ((appArgs_t *)(o))

#ifdef Q_SPY
static uint8_t const l_button = 0U;
#endif


static void theApp (GtkApplication* app, gpointer user_data)
{
    GtkWidget       *winTrafficLight;
    GtkWidget       *vBox, *btnBox;
    GtkWidget       *lblCounter;
    GtkWidget       *gridTop;
    GtkWidget       *btnPedestrian;
    GtkWidget       *btnQuit;
    PangoAttrList   *lblAttrs;
    GTask           *tlThread;
    gchar           *fontName;

    (void)app;

    // initialize the random number generator
    myRnd = g_rand_new();

    // create the buttons
    btnPedestrian = GTK_WIDGET(gtk_button_new_with_mnemonic("_Pedestrian"));
    btnQuit       = GTK_WIDGET(gtk_button_new_with_mnemonic("_Quit"));

    // create the counting label
    lblCounter    = GTK_WIDGET(gtk_label_new("0"));
    gtk_widget_set_hexpand(lblCounter, TRUE);
    gtk_widget_set_halign(lblCounter, GTK_ALIGN_FILL);
    // arrange the font appearance for the counting label
    lblAttrs = pango_attr_list_new();
    if(g_getenv("OS") && g_str_has_prefix(g_getenv("OS"), "Windows"))
    {
        fontName = "Segoe Script Bold Oblique 32";
    }
    else
    {
        fontName = "Purisa Bold 32";
    }
    pango_attr_list_change(lblAttrs, pango_attr_font_desc_new(pango_font_description_from_string(fontName)));
    pango_attr_list_change(lblAttrs, pango_attr_foreground_new(0xfcfc, 0xe9e9, 0x4f4f));
    pango_attr_list_change(lblAttrs, pango_attr_background_new(0x2020, 0x4a4a, 0x8787));
    gtk_label_set_attributes(GTK_LABEL(lblCounter), lblAttrs);
    gtk_label_set_width_chars (GTK_LABEL(lblCounter), 7);
    gtk_label_set_justify(GTK_LABEL(lblCounter), GTK_JUSTIFY_CENTER);

    // the LED for the pedestrian notification
    ledRed      = GTK_WIDGET(gtk_image_new_from_file("./Res/LedRed.xpm"));
    ledWhite    = GTK_WIDGET(gtk_image_new_from_file("./Res/LedWhite.xpm"));
    gtk_widget_show(ledWhite);
    gtk_widget_hide(ledRed);

    // create the grid container
    gridTop = GTK_WIDGET(gtk_grid_new());
    gtk_grid_set_row_spacing(GTK_GRID(gridTop), 2);
    gtk_grid_set_column_spacing(GTK_GRID(gridTop), 2);

    // put all UI elements into the grid
    gtk_grid_attach(GTK_GRID(gridTop), btnPedestrian, 2, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(gridTop), lblCounter, 1, 0, 2, 3);

    gtk_grid_attach(GTK_GRID(gridTop), ledRed, 2, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(gridTop), ledWhite, 2, 4, 1, 1);

    initTrafficLights(gridTop);

    // Create the button box for the QUIT button
    btnBox = GTK_WIDGET(gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL));
    gtk_button_box_set_layout(GTK_BUTTON_BOX(btnBox), GTK_BUTTONBOX_END);

    // create the VBox to hold the grid and the button box
    vBox = GTK_WIDGET(gtk_box_new(GTK_ORIENTATION_VERTICAL, 2));

    // create the main window
    winTrafficLight = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request(winTrafficLight, WINDOW_W, WINDOW_H);
    gtk_window_set_title(GTK_WINDOW(winTrafficLight), "Traffic Light Control");
    gtk_container_set_border_width(GTK_CONTAINER(winTrafficLight), 10);
    gtk_window_set_position(GTK_WINDOW(winTrafficLight), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(winTrafficLight), FALSE);

    // create the layout container for the background image
    // layout = gtk_layout_new(NULL, NULL);

    // create the background image widget
    // bgImage = gtk_image_new_from_file("./Res/background.png");

    // put it all together
    gtk_container_add (GTK_CONTAINER(btnBox), btnQuit);
    gtk_box_pack_start(GTK_BOX(vBox), gridTop, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vBox), btnBox, TRUE, TRUE, 0);
    // gtk_layout_put(GTK_LAYOUT(layout), bgImage, 0, 0);
    // gtk_layout_put(GTK_LAYOUT(layout), vBox, 0, 0);
    gtk_container_add (GTK_CONTAINER(winTrafficLight), vBox /* layout */);

    // ... and bring it to the screen
    gtk_widget_show_all(winTrafficLight);

    // connect the signals
    g_signal_connect(btnPedestrian, "clicked", G_CALLBACK (on_btn_ped_clicked), NULL);
    g_signal_connect(lblCounter, "draw", G_CALLBACK (on_label_draw), NULL);

    g_signal_connect_swapped(btnQuit, "clicked", G_CALLBACK (gtk_main_quit), G_OBJECT(btnQuit));
    g_signal_connect_swapped (winTrafficLight, "destroy", G_CALLBACK (gtk_main_quit), G_OBJECT(winTrafficLight));

    // kick off the statemachine thread
    tlThread = g_task_new(NULL, NULL, appThreadReadyCallback, NULL);
    if ((GTask *)(0) != tlThread)
    {
		g_task_set_task_data(tlThread, user_data, NULL);
        g_task_set_return_on_cancel (tlThread, TRUE);
        g_task_run_in_thread (tlThread, appThread);
    }

    // start the timer...
    g_timeout_add(1000, (GSourceFunc)count_handler, (gpointer)lblCounter);
    g_timeout_add(10, (GSourceFunc)event_handler, (gpointer)NULL);

    // and run all of it
    gtk_main();
}

int startGui(int argc, char *argv[])
{
	static appArgs_t appArgs;
    GtkApplication *app;
    int status;

	appArgs.argc = argc;
	appArgs.argv = argv;
	
    app = gtk_application_new ("com.webasto.trafficlight", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (theApp), &appArgs);
    status = g_application_run (G_APPLICATION (app), 1, argv);
    g_object_unref (app);

    return status;
}

static gboolean initTrafficLights(GtkWidget *widget)
{
    eTLidentity_t   id;
    eTLlight_t      lidx;

    for (id = TrafficLightA; id < MaxIdentity; id++)
    {
        guint32 row, col;

        switch(id)
        {
            case TrafficLightA:
                row = 0;
                col = 0;
                break;
            case TrafficLightB:
                row = 0;
                col = 3;
                break;
            case PedestrianLight:
                row = 3;
                col = 1;
                break;
            default:
                break;
        }
        for (lidx = RED; lidx < NO_LIGHT; lidx++)
        {
            switch(lidx)
            {
                case RED:
                    trafficLights[id][lidx][0] = GTK_WIDGET(gtk_image_new_from_file("./Res/LightOff.xpm"));
                    trafficLights[id][lidx][1] = GTK_WIDGET(gtk_image_new_from_file("./Res/LightRed.xpm"));
                    break;
                case YELLOW:
                    if (id != PedestrianLight)
                    {
                        trafficLights[id][lidx][0] = GTK_WIDGET(gtk_image_new_from_file("./Res/LightOff.xpm"));
                        trafficLights[id][lidx][1] = GTK_WIDGET(gtk_image_new_from_file("./Res/LightYellow.xpm"));
                    }
                    else
                    {
                        trafficLights[id][lidx][0] = GTK_WIDGET(0);
                        trafficLights[id][lidx][1] = GTK_WIDGET(0);
                    }
                    break;
                case GREEN:
                    trafficLights[id][lidx][0] = GTK_WIDGET(gtk_image_new_from_file("./Res/LightOff.xpm"));
                    trafficLights[id][lidx][1] = GTK_WIDGET(gtk_image_new_from_file("./Res/LightGreen.xpm"));
                    break;
                default:
                    trafficLights[id][lidx][0] = GTK_WIDGET(0);
                    trafficLights[id][lidx][1] = GTK_WIDGET(0);
                    break;
            }
            if ((GTK_WIDGET(0) != GTK_WIDGET(trafficLights[id][lidx][0])) && (GTK_WIDGET(0) != GTK_WIDGET(trafficLights[id][lidx][1])))
            {
                gtk_grid_attach(GTK_GRID(widget), trafficLights[id][lidx][0], col, row, 1, 1);
                gtk_grid_attach(GTK_GRID(widget), trafficLights[id][lidx][1], col, row, 1, 1);

                // gtk_misc_set_alignment(GTK_MISC(trafficLights[id][lidx][0]), 1, 0.5);
                // gtk_misc_set_alignment(GTK_MISC(trafficLights[id][lidx][1]), 1, 0.5);

                gtk_widget_hide(trafficLights[id][lidx][0]);
                gtk_widget_show(trafficLights[id][lidx][1]);

                row++;
            }
        }
    }

    return TRUE;
}

static void appThreadReadyCallback(GObject *source_object, GAsyncResult *res, gpointer user_data)
{
    (void)source_object;
    (void)res;
    (void)user_data;
}

static void appThread(GTask *task, gpointer source_object, gpointer task_data, GCancellable *cancellable)
{
    guint32 result;

    (void)source_object;
    (void)task_data;
    (void)cancellable;

    result = (guint32)main_gui(APP_ARGS(task_data)->argc, APP_ARGS(task_data)->argv); /* run the QF application */

    g_task_return_int(task, result);
}

static void on_btn_ped_clicked(void)
{
    g_mutex_lock(&myMutex);
    rndCountDown = 120;
    BSP_publishBtnEvt();
    g_mutex_unlock(&myMutex);
}

static void on_label_draw(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    GdkRGBA color;
    (void)data;

    gdk_rgba_parse(&color, "#20204a4a8787");
    cairo_set_source_rgb (cr, color.red, color.green, color.blue);
    cairo_rectangle (cr, 0, 0, gtk_widget_get_allocated_width(widget), gtk_widget_get_allocated_height(widget));
    cairo_fill(cr);
}

static gboolean event_handler(GtkWidget *widget)
{
    eTLlight_t light, lidx;
    eTLidentity_t id;
    guint8    led;
    (void)widget;

    if (g_mutex_trylock(&myMutex))
    {
        for (id = TrafficLightA; id < MaxIdentity; id++)
        {
            if (setLight[id].changed)
            {
                light = setLight[id].light;
                for (lidx = RED; lidx < NO_LIGHT; lidx++)
                {
                    if ((GTK_WIDGET(0) != trafficLights[id][lidx][0]) && (GTK_WIDGET(0) != trafficLights[id][lidx][1]))
                    {
                        gtk_widget_hide(trafficLights[id][lidx][lidx == light ? 0 : 1]);
                        gtk_widget_show(trafficLights[id][lidx][lidx == light ? 1 : 0]);
                    }
                }
                setLight[id].changed = 0;
            }
        }

        if (setLed.changed)
        {
            led = setLed.status;
            gtk_widget_hide(led ? ledWhite : ledRed);
            gtk_widget_show(led ? ledRed : ledWhite);

            setLed.changed = 0;
        }
        g_mutex_unlock(&myMutex);
    }

    return TRUE;
}

static gboolean count_handler(GtkWidget *widget)
{
	static guint32 countVal = 0;
    static gchar buf[16];

    //if (widget->window == NULL) return FALSE;

    countVal++;
    if (countVal >= 1000000ul)
        countVal = 0;
    g_snprintf(buf, Q_DIM(buf) - 1, "%u", countVal);
    gtk_label_set_text((GtkLabel *)widget, buf);
    gtk_widget_queue_draw(widget);

    g_mutex_lock(&myMutex);
    if (0 < rndCountDown)
    {
        rndCountDown--;
    }
    else
    {
        rndCountDown = g_rand_int_range(myRnd, 30, 120);
        BSP_publishBtnEvt();
    }
    g_mutex_unlock(&myMutex);

    return TRUE;
}

/*..........................................................................*/
void guiSetLight(eTLidentity_t id, eTLlight_t light)
{
    g_mutex_lock(&myMutex);
    setLight[id].light = light;
    setLight[id].changed = 1;
    g_mutex_unlock(&myMutex);
}
/*..........................................................................*/
void guiSetPedLed(uint16_t status)
{
    g_mutex_lock(&myMutex);
    setLed.status = status;
    setLed.changed = 1;
    g_mutex_unlock(&myMutex);
}
/*..........................................................................*/
guint16 guiGetButton(void)
{
    guint16 retCode = 0;
#if 0
    g_mutex_lock(&myMutex);
    retCode = keyPressed;
    keyPressed = 0;
    g_mutex_unlock(&myMutex);
#endif
    return retCode;
}
/*--------------------------------------------------------------------------*/

