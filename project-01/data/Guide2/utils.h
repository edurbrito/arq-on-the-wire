
typedef enum
{
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    SSTOP
} my_state_t;

typedef struct
{
    char flag;
    char a;
    char c;
    char bcc;
    my_state_t state;
} tram;

my_state_t startState(char input, tram *t)
{
    if (input == 0x7E)
    {
        t->flag = input;
        return FLAG_RCV;
    }
    return START;
}

my_state_t flagState(char input, tram *t)
{
    if (input == 0x03)
    {
        t->a = input;
        return A_RCV;
    }
    else if (input == 0x7E)
        return FLAG_RCV;
    return START;
}

my_state_t aState(char input, tram *t)
{
    if (input == 0x01)
    {
        t->c = input;
        return C_RCV;
    }
    else if (input == 0x7E)
        return FLAG_RCV;
    return START;
}

my_state_t cState(char input, tram *t)
{
    if (input == (t->a ^ t->c))
    {
        t->bcc = input;
        return BCC_OK;
    }
    else if (input == 0x7E)
        return FLAG_RCV;
    return START;
}

my_state_t bccState(char input, tram *t)
{
    if (input == 0x7E)
        return SSTOP;
    return START;
}

my_state_t getState(char input, tram *t)
{
    switch (t->state)
    {
    case START:
        return startState(input, t);
    case FLAG_RCV:
        return flagState(input, t);
    case A_RCV:
        return aState(input, t);
    case C_RCV:
        return cState(input, t);
    case BCC_OK:
        return bccState(input, t);
    case SSTOP:
        return SSTOP;
    default:
        return startState(input, t);
    }
}
