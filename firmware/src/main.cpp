#include <stdlib.h>
#include <wirish/wirish.h>
#include <servos.h>
#include <terminal.h>
#include <main.h>
#include <math.h>
#include <dxl.h>
#include <function.h>
#include <commands.h>
#include <rc.h>
#include "voltage.h"
#include "buzzer.h"
#include "config.h"
#include "motion.h"
#include "leds.h"
#include "mapping.h"
#include "imu.h"
#include "bt.h"

#define LIT     22

int x;
TERMINAL_COMMAND(mem, "mem")
{
    int y;
    terminal_io()->println(((int)&x)-0x20000000);
    terminal_io()->println(((int)&y)-0x20000000);
}

// Time
TERMINAL_PARAMETER_FLOAT(t, "Time", 0.0);

TERMINAL_COMMAND(version, "Getting firmware version")
{
    terminal_io()->print("version=");
    terminal_io()->println(METABOT_VERSION);
}

TERMINAL_COMMAND(started, "Is the robot started?")
{
    terminal_io()->print("started=");
    terminal_io()->println(started);
}

// Enabling/disabling move
TERMINAL_PARAMETER_BOOL(move, "Enable/Disable move", true);


TERMINAL_COMMAND(suicide, "Lit the fuse")
{
    digitalWrite(LIT, HIGH);
}

/**
 * Initializing
 */
void setup()
{
    digitalWrite(LIT, LOW);
    pinMode(LIT, OUTPUT);

    bt_init();

    motion_init();
    
    voltage_init();

    // Initializing the DXL bus
    delay(500);
    dxl_init();

    // Initializing config (see config.h)
    config_init();

    // Initializing the buzzer
    buzzer_init();
    buzzer_play(MELODY_BOOT);

    // Initializing the IMU
    imu_init();

    // Initializing positions to 0
    for (int i=0; i<12; i++) {
        dxl_set_position(i+1, 0.0);
    }
    for (int i=0; i<4; i++) {
        l1[i] = l2[i] = l3[i] = 0;
    }
}

/**
 * Computing the servo values
 */
void tick()
{
    if (!move || !started) {
        t = 0.0;
        return;
    }

    // Incrementing and normalizing t
    t += motion_get_f()*0.02;
    if (t > 1.0) {
        t -= 1.0;
        colorize();
    }
    if (t < 0.0) t += 1.0;

    motion_tick(t);

    // Sending order to servos
    dxl_set_position(mapping[0], l1[0]);
    dxl_set_position(mapping[3], l1[1]);
    dxl_set_position(mapping[6], l1[2]);
    dxl_set_position(mapping[9], l1[3]);

    dxl_set_position(mapping[1], l2[0]);
    dxl_set_position(mapping[4], l2[1]);
    dxl_set_position(mapping[7], l2[2]);
    dxl_set_position(mapping[10], l2[3]);

    dxl_set_position(mapping[2], l3[0]);
    dxl_set_position(mapping[5], l3[1]);
    dxl_set_position(mapping[8], l3[2]);
    dxl_set_position(mapping[11], l3[3]);
}

void loop()
{
    buzzer_tick();
    imu_tick();
    voltage_tick();
}
