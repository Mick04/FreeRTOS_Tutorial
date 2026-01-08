// ScheduleService.cpp
#include "ScheduleService.h"
#include "TimeService.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static HeaterSchedule currentSchedule =
{};

static SemaphoreHandle_t scheduleMutex = NULL;  // ✅ Initialize to NULL

// Add this helper function at the top:
int timeStringToMinutes(const String& timeStr)
{
    int colonIndex = timeStr.indexOf(':');
    if (colonIndex > 0)
    {
        int hour = timeStr.substring(0, colonIndex).toInt();
        int minute = timeStr.substring(colonIndex + 1).toInt();
        return hour * 60 + minute;
    }
    return 0;  // Invalid time format
}

void ScheduleService_init()
{
     if (scheduleMutex == NULL)  // ✅ Prevent double-init
    {
        scheduleMutex = xSemaphoreCreateMutex();
        if (scheduleMutex == NULL)
        {
            Serial.println("❌ Failed to create schedule mutex!");
        }
        else
        {
            Serial.println("✅ ScheduleService initialized");
        }
    }
}

bool ScheduleService_isInitialized()  // ✅ Add this helper
{
    return (scheduleMutex != NULL);
}

void ScheduleService_setSchedule(const HeaterSchedule &schedule)
{
    if (scheduleMutex == NULL)
    {
        Serial.println("⚠️ ScheduleService not initialized, cannot set schedule");
        return;
    }
    
    if (xSemaphoreTake(scheduleMutex, portMAX_DELAY))
    {
        currentSchedule = schedule;
        
        Serial.println("\n📅 Schedule Updated:");
        Serial.printf("  AM: %s → %.1f°C\n", 
                      schedule.amTargetTime.c_str(), schedule.amTarget);  // ✅ Print string directly
        Serial.printf("  PM: %s → %.1f°C\n", 
                      schedule.pmTargetTime.c_str(), schedule.pmTarget);  // ✅ Print string directly
        
        xSemaphoreGive(scheduleMutex);
    }
}


HeaterSchedule ScheduleService_getSchedule()
{
    HeaterSchedule copy = {};
    
    if (scheduleMutex == NULL)  // ✅ Safety check
    {
        return copy;  // Return empty schedule
    }
    
    if (xSemaphoreTake(scheduleMutex, portMAX_DELAY))
    {
        copy = currentSchedule;
        xSemaphoreGive(scheduleMutex);
    }
    return copy;
}

PeriodOfDay ScheduleService_getCurrentPeriod()
{
    struct tm now = TimeService_getLocalTime();
    HeaterSchedule s = ScheduleService_getSchedule();

    int nowMinutes = now.tm_hour * 60 + now.tm_min;
    int amMinutes = timeStringToMinutes(s.amTargetTime);  // ✅ Convert "07:00" → 420
    int pmMinutes = timeStringToMinutes(s.pmTargetTime);  // ✅ Convert "19:00" → 1140

    if (amMinutes < pmMinutes)
    {
        // Normal day schedule (AM before PM)
        return (nowMinutes >= amMinutes && nowMinutes < pmMinutes)
               ? PERIOD_AM
               : PERIOD_PM;
    }
    else
    {
        // Overnight schedule (PM before AM next day)
        return (nowMinutes >= amMinutes || nowMinutes < pmMinutes)
               ? PERIOD_AM
               : PERIOD_PM;
    }
}

float ScheduleService_getCurrentTarget()
{
    HeaterSchedule s = ScheduleService_getSchedule();
    return (ScheduleService_getCurrentPeriod() == PERIOD_AM)
           ? s.amTarget
           : s.pmTarget;
}