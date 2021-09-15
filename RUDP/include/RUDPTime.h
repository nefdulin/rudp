#pragma once

#define RUDP_TIME_OVERFLOW 86400000

#define RUDP_TIME_LESS(a, b) ((a) - (b) >= RUDP_TIME_OVERFLOW)
#define RUDP_TIME_GREATER(a, b) ((b) - (a) >= RUDP_TIME_OVERFLOW)
#define RUDP_TIME_LESS_EQUAL(a, b) (! RUDP_TIME_GREATER (a, b))
#define RUDP_TIME_GREATER_EQUAL(a, b) (! RUDP_TIME_LESS (a, b))

#define RUDP_TIME_DIFFERENCE(a, b) ((a) - (b) >= RUDP_TIME_OVERFLOW ? (b) - (a) : (a) - (b))