#include <csignal>
#include <iostream>

void ctrlCInterruptHandler(int signal)
{
    // backspaces are needed for getting rid of the printed ^C when hitting CTRL+C
    std::cout << "\b\bInterrupt signal " << signal << " arrived! Goodbye!\n\a";
    exit(signal);
}

int main()
{
    std::signal(SIGINT, ctrlCInterruptHandler);

    std::cout << "Waiting for interrupt (press CTRL+C)...\n" << std::flush;

    for (;;)
        ;

    return 0;
}
