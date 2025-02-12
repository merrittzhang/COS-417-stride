#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"
#include "test_helper.h"


int
main(int argc, char* argv[])
{
    struct pstat ps;
    int my_idx = find_my_stats_index(&ps);
    ASSERT(my_idx != -1, "Could not get process stats from pgetinfo");

    ASSERT(ps.inuse[my_idx], "My slot in the ptable is not in use!");

    ASSERT(ps.tickets[my_idx] == DEFAULT_TICKETS, "My ticekts (%d) does not match \
            the default number of tickets (%d)", ps.tickets[my_idx], DEFAULT_TICKETS);

    int new_tickets = 24;
    ASSERT(settickets(new_tickets) != -1, "settickets() failed");

    my_idx = find_my_stats_index(&ps);
    ASSERT(my_idx != -1, "Could not get process stats from pgetinfo");

    ASSERT(ps.tickets[my_idx] == new_tickets, "Tried setting tickets to \
            %d, but getpinfo returned %d", new_tickets, ps.tickets[my_idx]);

    test_passed();

    exit();
}
