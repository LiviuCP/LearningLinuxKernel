#include "stopwatchapp.h"

/* How to use:
    - enter sudo ./StopWatch
    - following optional arguments can be entered after the command:
        1) a time interval expressed in seconds (if not entered a default value will be used)
        2) --reverse: the stopwatch will run in reverse (decrease time) starting with the given/default interval value
        3) --autoreset: once the stopwatch reaches the reset value it will auto-reset and resume counting from the start

   Notes:
    - when no arguments are entered the stopwatch counts from 0 to the predefined interval value then stops
    - when --reverse is not entered the stopwatch counts from 0 to the given/default interval value
    - when --autoreset is not entered the stopwatch stops once finishing the counting process
    - the order of the arguments can be arbitrary
    - duplicate arguments (including entering two times) are not allowed; an error is triggered
    - any invalid arguments prevent the application from running; an error is triggered
*/

int main(int argc, char** argv)
{
    return StopWatch::App::run(argc, argv);
}
