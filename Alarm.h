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

    // do, do#, ré, ré#, mi, fa, fa#, sol, sol#, la, la#, si
    const static float note[12];
    static const int nombreDeNotes = 32;
    static const int tempo = 150;
    static const int melodie[][3];
}; 
