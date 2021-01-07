/**
 * Represents the alarm.
 */
class Alarm 
{
public:
    Alarm();

    void startAlarm();
    void stopAlarm();

private:
    static void runAlarm(void*);

    bool alarmAlreadyLunched = false;
    TaskHandle_t xTask2Handle = NULL;
}; 
