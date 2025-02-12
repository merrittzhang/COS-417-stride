#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#include "test_helper.h"
#include "param.h"

int
main(int argc, char* argv[])
{
    struct pstat ps;
    int my_idx = find_my_stats_index(&ps);
    ASSERT(my_idx != -1, "Could not get process stats from pgetinfo");

    int count_in_use = 0;
    for (int i = 0; i < NPROC; i++) 
        if (ps.inuse[i])
            count_in_use++;

    // Current process + init + shell
    int expected_in_use = 3;
    ASSERT(count_in_use == expected_in_use, "Expected %d process in use initially,\
            but got %d in use", expected_in_use, count_in_use);

    int p = fork();
    if (p == 0) {
        my_idx = find_my_stats_index(&ps);
        count_in_use = 0;
        for (int i = 0; i < NPROC; i++) 
            if (ps.inuse[i])
                count_in_use++;

        // Child should be added to the in use list
        ASSERT(count_in_use == expected_in_use + 1, "Expected %d process in use \
                after creatin a child, but got %d in use",
                expected_in_use + 1, count_in_use);
            exit();
    }
       
    wait();

    test_passed();

    exit();
}
