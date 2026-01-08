// ScheduleService.h
#pragma once
#include <Arduino.h>

enum PeriodOfDay
{
    PERIOD_AM,
    PERIOD_PM
};

struct HeaterSchedule
{
    float amTarget;
    float pmTarget;
    String amTargetTime;
    String pmTargetTime;

};

extern HeaterSchedule heaterSchedule;

void ScheduleService_init();
bool ScheduleService_isInitialized();

void ScheduleService_setSchedule(const HeaterSchedule &schedule);
HeaterSchedule ScheduleService_getSchedule();

PeriodOfDay ScheduleService_getCurrentPeriod();
float ScheduleService_getCurrentTarget();