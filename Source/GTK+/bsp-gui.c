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
#include <string.h>
#include <stdlib.h>

// Q_DEFINE_THIS_FILE

#define WINDOW_W    350
#define WINDOW_H    250

// LOCAL functions ----------------------------------------------------------
static void theApp (GtkApplication* app, gpointer user_data);
// called when button PEDESTRIAN is clicked
static void on_btn_ped_clicked(void);
static void on_btn_emergency_clicked(GtkButton *button, gpointer user_data);
// static void on_label_draw(GtkWidget *widget, cairo_t *cr, gpointer data);
// timeout
static gboolean count_handler(GtkWidget *widget);
static gboolean event_handler(GtkWidget *widget);
// static void getResName(gchar *buf, gchar *res);
static void     appThread(GTask *task, gpointer source_object, gpointer task_data, GCancellable *cancellable);
static void     appThreadReadyCallback (GObject *source_object, GAsyncResult *res, gpointer user_data);

// static struct termios l_tsav; /* structure with saved terminal attributes */
// static guint8 keyPressed = 0;
static GMutex myMutex;
static GRand  *myRnd;
static guint32 rndCountDown = 120;
static gchar *appName = NULL, *dirName = NULL;

GtkWidget    *trafficLights[MaxIdentity][NO_LIGHT][2], *ledWhite, *ledRed;
GtkWidget    *digits[3][10];

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

static void theApp (GtkApplication* app, gpointer user_data)
{
	GtkBuilder      *builder;
	GtkWidget       *winTrafficLight;
    // GtkWidget       *lblCounter;
    GTask           *tlThread;
    uint16_t        i, j;
    gchar           digitId[10];

    (void)app;

    // initialize the random number generator
    myRnd = g_rand_new();

    // load the ressources
    builder = gtk_builder_new_from_resource("/com/webasto/trafficLight/trafficLight.xml");

    // connect the widgets to the application
    winTrafficLight = GTK_WIDGET(gtk_builder_get_object(builder, "winTrafficLight"));
    // lblCounter      = GTK_WIDGET(gtk_builder_get_object(builder, "lblCounter"));

    trafficLights[TrafficLightA][RED][0]     = GTK_WIDGET(gtk_builder_get_object(builder, "tlaRedOff"));
    trafficLights[TrafficLightA][YELLOW][0]  = GTK_WIDGET(gtk_builder_get_object(builder, "tlaYellowOff"));
    trafficLights[TrafficLightA][GREEN][0]   = GTK_WIDGET(gtk_builder_get_object(builder, "tlaGreenOff"));

    trafficLights[TrafficLightA][RED][1]     = GTK_WIDGET(gtk_builder_get_object(builder, "tlaRedOn"));
    trafficLights[TrafficLightA][YELLOW][1]  = GTK_WIDGET(gtk_builder_get_object(builder, "tlaYellowOn"));
    trafficLights[TrafficLightA][GREEN][1]   = GTK_WIDGET(gtk_builder_get_object(builder, "tlaGreenOn"));

    trafficLights[TrafficLightB][RED][0]     = GTK_WIDGET(gtk_builder_get_object(builder, "tlbRedOff"));
    trafficLights[TrafficLightB][YELLOW][0]  = GTK_WIDGET(gtk_builder_get_object(builder, "tlbYellowOff"));
    trafficLights[TrafficLightB][GREEN][0]   = GTK_WIDGET(gtk_builder_get_object(builder, "tlbGreenOff"));

    trafficLights[TrafficLightB][RED][1]     = GTK_WIDGET(gtk_builder_get_object(builder, "tlbRedOn"));
    trafficLights[TrafficLightB][YELLOW][1]  = GTK_WIDGET(gtk_builder_get_object(builder, "tlbYellowOn"));
    trafficLights[TrafficLightB][GREEN][1]   = GTK_WIDGET(gtk_builder_get_object(builder, "tlbGreenOn"));

    trafficLights[PedestrianLight][RED][0]   = GTK_WIDGET(gtk_builder_get_object(builder, "pedRedOff"));
    trafficLights[PedestrianLight][GREEN][0] = GTK_WIDGET(gtk_builder_get_object(builder, "pedGreenOff"));

    trafficLights[PedestrianLight][RED][1]   = GTK_WIDGET(gtk_builder_get_object(builder, "pedRedOn"));
    trafficLights[PedestrianLight][GREEN][1] = GTK_WIDGET(gtk_builder_get_object(builder, "pedGreenOn"));

    for (i = 0; i < 3; i++)
    {
    	for (j = 0; j < 10; j++)
    	{
    		switch (i)
    		{
				case 2:
		    		sprintf(digitId, "cntxx%1d", j);
					break;
				case 1:
		    		sprintf(digitId, "cntx%1dx", j);
					break;
				case 0:
				default:
		    		sprintf(digitId, "cnt%1dxx", j);
					break;
    		}
    		digits[i][j] = GTK_WIDGET(gtk_builder_get_object(builder, digitId));
    		if (0 == j)
    		{
    			gtk_widget_show(digits[i][j]);
    		}
    		else
    		{
    			gtk_widget_hide(digits[i][j]);
    		}
    	}
    }

    ledWhite = GTK_WIDGET(gtk_builder_get_object(builder, "LedWhite"));
    ledRed   = GTK_WIDGET(gtk_builder_get_object(builder, "LedRed"));

    // connect the signals
    gtk_builder_add_callback_symbol(builder, "on_btn_ped_clicked", G_CALLBACK(on_btn_ped_clicked));
    gtk_builder_add_callback_symbol(builder, "on_btn_emergency_clicked", G_CALLBACK(on_btn_emergency_clicked));
    gtk_builder_add_callback_symbol(builder, "on_btn_quit_clicked", G_CALLBACK(gtk_main_quit));
    gtk_builder_add_callback_symbol(builder, "on_winTrafficLight_destroy", G_CALLBACK(gtk_main_quit));
    // gtk_builder_add_callback_symbol(builder, "on_label_draw", G_CALLBACK(on_label_draw));
    gtk_builder_connect_signals(builder, NULL);
    g_object_unref(builder);

    // ... and bring it to the screen
    gtk_widget_show(winTrafficLight);

    // kick off the statemachine thread
    tlThread = g_task_new(NULL, NULL, appThreadReadyCallback, NULL);
    if ((GTask *)(0) != tlThread)
    {
		g_task_set_task_data(tlThread, user_data, NULL);
        g_task_set_return_on_cancel (tlThread, TRUE);
        g_task_run_in_thread (tlThread, appThread);
    }

    // start the timer...
    g_timeout_add(1000, (GSourceFunc)count_handler, NULL /* (gpointer)lblCounter */);
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
	guint16 i = 0, j = 0;
	
	appName = strdup(argv[0]);
	dirName = appName;
	for (i = 0; '\0' != appName[i]; i++)
	{
		if (appName[i] == '\\') appName[i] = '/';
		if (appName[i] == '/')  j = i;
	}
	if (j > 0)
	{
		appName[j] = '\0';
		appName = &appName[j+1];
	}
	else
	{
		dirName = NULL;
	}
    app = gtk_application_new ("com.webasto.trafficlight", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (theApp), &appArgs);
    status = g_application_run (G_APPLICATION (app), 1, argv);
    g_object_unref (app);

    return status;
}

#if 0
static void getResName(gchar *buf, gchar *res)
{
	if (NULL != dirName)
	{
		sprintf(buf, "%s/Res/%s", dirName, res);
	}
	else
	{
		sprintf(buf, "./Res/%s", res);
	}
}
#endif

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

static void on_btn_emergency_clicked(GtkButton *button, gpointer user_data)
{
	static enum {
		SetEm = 0,
		RelEm
	} state = SetEm;
	static gchar *btnLabel = NULL;
	static GtkWidget *btnImage = NULL;

	g_mutex_lock(&myMutex);
	BSP_publishEmergencyEvt(); /* publish to all subscribers */
#if 0
	switch (state)
	{
		case RelEm:
			state = SetEm;
			if (NULL != btnLabel)
			{
				gtk_button_set_label(button, btnLabel);
				free(btnLabel);
				btnLabel = NULL;
			}
			if (NULL != btnImage)
			{
				gtk_button_set_image(button, btnImage);
				btnImage = NULL;
			}
			break;
		case SetEm:
			state = RelEm;
			btnImage = gtk_button_get_image(button);
			btnLabel = strdup(gtk_button_get_label(button));
			gtk_button_set_label(button, "Release");
			if (NULL != btnImage)
			{
				gtk_button_set_image(button, btnImage);
			}

			break;
		default:
			break;
	}
#endif
    g_mutex_unlock(&myMutex);
}

#if 0
static void on_label_draw(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    GdkRGBA color;
    (void)data;

    gdk_rgba_parse(&color, "#20204a4a8787");
    cairo_set_source_rgb (cr, color.red, color.green, color.blue);
    cairo_rectangle (cr, 0, 0, gtk_widget_get_allocated_width(widget), gtk_widget_get_allocated_height(widget));
    cairo_fill(cr);
}
#endif

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

static void drawDigits(guint32 cnt, guint flag)
{
	guint  i;

	i = cnt / 100;
	if (flag)
	{
		gtk_widget_show(digits[0][i]);
	}
	else
	{
		gtk_widget_hide(digits[0][i]);
	}
	gtk_widget_queue_draw(digits[0][i]);

	cnt %= 100;
	i = cnt / 10;
	if (flag)
	{
		gtk_widget_show(digits[1][i]);
	}
	else
	{
		gtk_widget_hide(digits[1][i]);
	}
	gtk_widget_queue_draw(digits[1][i]);

	i = cnt % 10;
	if (flag)
	{
		gtk_widget_show(digits[2][i]);
	}
	else
	{
		gtk_widget_hide(digits[2][i]);
	}
	gtk_widget_queue_draw(digits[2][i]);
}

static gboolean count_handler(GtkWidget *widget)
{
	static guint32 countVal = 0;
    // static gchar buf[16];

    //if (widget->window == NULL) return FALSE;

    drawDigits(countVal, FALSE);
    countVal++;
    if (countVal >= 1000ul)
        countVal = 0;
    drawDigits(countVal, TRUE);
    // g_snprintf(buf, Q_DIM(buf) - 1, "%u", countVal);
    // gtk_label_set_text((GtkLabel *)widget, buf);
    // gtk_widget_queue_draw(widget);

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

