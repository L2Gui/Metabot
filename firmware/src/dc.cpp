#include <stdio.h>
#include <terminal.h>
#include <wirish.h>
#include "dc.h"

#define M1A 15  // 4 CH2
#define M1B 16  // 4 CH1
#define M2A 10  // 2 CH2
// XXX: In future fix, to avoid conflict with buzzer
// #define M2A 5  // 3 CH1
#define M2B 27  // 1 CH1
#define M3A 3   // 3 CH3
#define M3B 4   // 3 CH2

TERMINAL_PARAMETER_INT(kkk, "Debug", 0);

#define MOTORS 3

static int divider = 0;
bool dcFlag = false;

struct dc_motor {
    int a, b;
    float x, y;
};

/**
 
            ^
            | x

     2v /       \ 1v
                        -> y

            -
            <3


*/

static struct dc_motor motors[MOTORS] = {
    {M1A, M1B, -0.866, -0.5},
    {M2A, M2B, -0.866, 0.5},
    {M3A, M3B, 0, 1}
};

static void _dc_ovf()
{
    divider++;
    if (divider >= 480) {
        divider = 0;
        dcFlag = true;
    }
}

static void _init_timer(int number)
{
    HardwareTimer timer(number);

    // Configuring timer
    timer.pause();
    timer.setPrescaleFactor(1); 
    timer.setOverflow(3000); // 24Khz

    if (number == 3) {
        timer.attachCompare1Interrupt(_dc_ovf);
        timer.setCompare(TIMER_CH1, 1);
    }

    timer.refresh();
    timer.resume();
}

void dc_init()
{
    // XXX Init timers
    _init_timer(1);
    _init_timer(2);
    _init_timer(3);
    _init_timer(4);
    
    for (int k=0; k<3; k++) {
        pwmWrite(motors[k].a, 0);
        pwmWrite(motors[k].b, 0);
        pinMode(motors[k].a, PWM);
        pinMode(motors[k].b, PWM);
    }
}

int _min(int a, int b)
{
    return (a < b) ? a : b;
}

void dc_command(int m1, int m2, int m3)
{
    pwmWrite(M1A, m1>0 ? m1 : 0);
    pwmWrite(M1B, m1<0 ? -m1 : 0);

    pwmWrite(M2A, m2>0 ? m2 : 0);
    pwmWrite(M2B, m2<0 ? -m2 : 0);

    pwmWrite(M3A, m3>0 ? m3 : 0);
    pwmWrite(M3B, m3<0 ? -m3 : 0);
}

static int s(int m)
{
    return (m > 0) ? 1 : -1;
}

void dc_xyt(float x, float y, float t)
{
    int m1 = (x*motors[0].x+y*motors[0].y)*80;
    int m2 = (x*motors[1].x+y*motors[1].y)*80;
    int m3 = (x*motors[2].x+y*motors[2].y)*80;

    m1 += t*80;
    m2 -= t*80;
    m3 += t*80;

    /*
    if (m1 != 0) m1 += s(m1)*2000;
    if (m2 != 0) m2 += s(m2)*2000;
    if (m3 != 0) m3 += s(m3)*2000;
    */

    dc_command(m1, m2, m3);
}

TERMINAL_COMMAND(tst, "Test")
{
    pinMode(M3A, OUTPUT);
    digitalWrite(M3A, HIGH);
    delay(500);
    digitalWrite(M3A, LOW);
}

TERMINAL_COMMAND(dc, "DC test")
{
    if (argc == 3) {
        dc_xyt(atof(argv[0]), atof(argv[1]), atof(argv[2]));
    }
}

TERMINAL_COMMAND(dcr, "DC test")
{
    if (argc == 3) {
        dc_command(atoi(argv[0]), atoi(argv[1]), atoi(argv[2]));
    }
}

TERMINAL_COMMAND(em, "em")
{
    dc_command(0, 0, 0);
}

TERMINAL_COMMAND(test, "Test")
{
    digitalWrite(atoi(argv[0]), HIGH);
    delay(1000);
    digitalWrite(atoi(argv[0]), LOW);
}