#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#include "test_helper.h"


int
main(int argc, char* argv[])
{
    struct pstat *nullptr = 0;
    int my_idx = find_my_stats_index(nullptr);
    ASSERT(my_idx == -1, "Passing NULL ptr into getpinfo should have returned -1, \
            but it returned %d", my_idx);

    test_passed();

    exit();
}
