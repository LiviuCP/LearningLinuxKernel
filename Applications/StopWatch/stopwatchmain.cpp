// #include <iostream>

#include "stopwatchapp.h"
// #include "stopwatchparser.h"

int main(int argc, char** argv)
{
    // if (argc > 1)
    // {
    //     const auto result{StopWatch::Parser::parse(argc, argv)};

    //     if (!result)
    //     {
    //         std::cerr << result.error() << "\n";
    //         return -1;
    //     }

    //     std::cout << "Seconds count: " << result->seconds << "\n";
    //     std::cout << "Countdown: " << (result->reverse ? "yes" : "no") << "\n";
    //     std::cout << "Auto-reset: " << (result->autoReset ? "yes" : "no") << "\n";

    //     return 0;
    // }

    return StopWatch::App::run(argc, argv);
}
