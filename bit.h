#define BIT_SET(target, position) target |= 1 << position
#define BIT_CLR(target, position) target &= ~(1 << position)
#define BIT_FLP(target, position) target ^= 1 << position
#define BIT_GET(target, position) (target & (1 << position)) >> position
#define BOOL2STRING(input) (input ? "true" : "false")