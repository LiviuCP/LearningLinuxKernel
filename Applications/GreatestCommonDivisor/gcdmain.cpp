#include "gcdapp.h"

/* How to use:
    - build the whole LearningLinuxKernel project
    - run the application by choosing either of the executables:
        sudo ./[Ioctl/Procfs/Sysfs]GreatestCommonDivisor [integer1] [integer2] # g.c.d. is retrieved
     (e.g. sudo ./IoctlGreatestCommonDivisor 6 10 # g.c.d. is 2)

    Notes:
    - the application requests loading of the required kernel module(s) so no action is required from user side other
   than running the app with "sudo" and providing the required arguments (divided and divider)
    - the kernel module(s) will be left in the same state that when the application got opened (loaded/unloaded) unless
   an expected error occured when loading/unloading one of the modules
    - if entering a single argument, the second is presumed 1
    - if entering no argument, the default values are 0 and 1
 */

int main(int argc, char** argv)
{
    return GCD::App::run(argc, argv);
}
