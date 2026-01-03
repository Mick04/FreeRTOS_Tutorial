#include "TimeService.h"
#include <WiFi.h>
#include <sys/time.h>

// -------------------- Globals --------------------
static TimeState_t timeState = TIME_DISCONNECTED;
static SemaphoreHandle_t timeMutex = NULL;

static const char* ntpServer = "pool.ntp.org";
static const long gmtOffset_sec = 0;   // UTC
static const int daylightOffset_sec = 0; // Adjusted dynamically
static bool dstActive = false;

// -------------------- DST Helper --------------------
bool isDST(struct tm *timeinfo)
{
    // Simple EU/UK DST: last Sunday of March to last Sunday of October
    int month = timeinfo->tm_mon + 1;
    int day = timeinfo->tm_mday;
    int wday = timeinfo->tm_wday; // Sunday = 0
    int lastSunday;

    if (month < 3 || month > 10) return false;
    if (month > 3 && month < 10) return true;

    // Find last Sunday
    lastSunday = day - wday;
    if (month == 3) return day >= lastSunday; // DST starts
    if (month == 10) return day < lastSunday; // DST ends
    return false;
}

// -------------------- Task --------------------
void TimeService_task(void* pvParameters)
{
    Serial.println("⏰ Time Service Task started");

    for (;;)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            if (xSemaphoreTake(timeMutex, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                timeState = TIME_DISCONNECTED;
                xSemaphoreGive(timeMutex);
            }
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        if (xSemaphoreTake(timeMutex, pdMS_TO_TICKS(100)) == pdTRUE)
        {
            timeState = TIME_SYNCING;
            xSemaphoreGive(timeMutex);
        }

        // Initialize NTP
        configTime(gmtOffset_sec, 0, ntpServer);
        struct tm timeinfo;
        if (getLocalTime(&timeinfo, 10000))
        {
            // Apply DST
            dstActive = isDST(&timeinfo);
            if (dstActive) timeinfo.tm_hour += 1;

            if (xSemaphoreTake(timeMutex, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                timeState = TIME_READY;
                xSemaphoreGive(timeMutex);
            }

            char buffer[64];
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
            Serial.printf("✅ Current Time: %s (DST=%d)\n", buffer, dstActive);
        }
        else
        {
            Serial.println("❌ Failed to get NTP time, retrying...");
        }

        vTaskDelay(pdMS_TO_TICKS(60000)); // update every 60s
    }
}

// -------------------- Public API --------------------
void TimeService_init()
{
    timeMutex = xSemaphoreCreateMutex();
    if (!timeMutex)
    {
        Serial.println("❌ Failed to create Time mutex!");
        return;
    }

    xTaskCreatePinnedToCore(
        TimeService_task,
        "Time Task",
        4096,
        NULL,
        1,
        NULL,
        1
    );

    Serial.println("✅ Time task created");
}

TimeState_t TimeService_getState()
{
    TimeState_t state;
    if (xSemaphoreTake(timeMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        state = timeState;
        xSemaphoreGive(timeMutex);
    }
    else
    {
        state = TIME_DISCONNECTED;
    }
    return state;
}

struct tm TimeService_getLocalTime()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        memset(&timeinfo, 0, sizeof(timeinfo));
    }
    if (dstActive) timeinfo.tm_hour += 1;
    return timeinfo;
}

String TimeService_getIsoTimestamp()
{
    struct tm timeinfo = TimeService_getLocalTime();
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &timeinfo);
    return String(buffer);
}

uint32_t TimeService_getEpoch()
{
    time_t now;
    time(&now);
    return (uint32_t)now;
}

String TimeService_getFormattedDateTime()
{
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo); // DST-aware

    char buffer[25];
    snprintf(buffer, sizeof(buffer),
             "%02d/%02d/%04d %02d:%02d:%02d",
             timeinfo.tm_mday,
             timeinfo.tm_mon + 1,
             timeinfo.tm_year + 1900,
             timeinfo.tm_hour,
             timeinfo.tm_min,
             timeinfo.tm_sec);

    return String(buffer);
}