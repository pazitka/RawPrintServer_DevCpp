/*

This is a simple file made from stackoverflow code.
Here you have a link:
https://stackoverflow.com/questions/7411301/how-to-introduce-date-and-time-in-log-file

It's purpose is to get time and date and display it inside log.
That helps a lot with problem solving.
Original RawPrintServer was not always working in Windows 10.
It's probaply related to some restrictions and admin priviledges.
I'm working on this project to test is it possible to make RPS works with Zebra label printers with new updated Windows 10-11.

This file will also be properly modified in future or made from scratch but right now this code made its purpose.
That is why I leave it as it is.

*/

//-------------------------
// INCLUDE
//-------------------------

#include <time.h> // time_t, tm, time, localtime, strftime

/**
 * Long time for log purpose
 * @returns Returns the local date/time formatted as 2014-03-19 11:11:52
 * @see: https://stackoverflow.com/questions/7411301/how-to-introduce-date-and-time-in-log-file
 *       // oryginal name: getFormattedTime
 */
char* getCurrentTime(void) {

    time_t rawtime;
    struct tm* timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // Must be static, otherwise won't work
    static char _retval[12];
    strftime(_retval, sizeof(_retval), "%Y.%m.%d %H:%M:%S", timeinfo);

    return _retval;
}

/**
 * Short time for file name
 * @returns short time
 */
char* getShortTime(void) {

    time_t rawtime;
    struct tm* timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // Must be static, otherwise won't work
    static char _retval[8];
    strftime(_retval, sizeof(_retval), "%Y%m%d_%H%M%S", timeinfo);

    return _retval;
}
