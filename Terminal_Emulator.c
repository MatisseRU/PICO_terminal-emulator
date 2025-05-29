#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"

// Terminal Emulator
// commands are sequences of numbers made of characters inserted, commands can't be longer than 2 chars

uint8_t DBG_CONSOLE_FLAG;
int RUN_CONSOLE;
uint8_t POWER_ON;

uint32_t expo_reccur(int nbr, int exp)
{
    if ( exp == 0 )
    {
        return 1;
    }

    if ( nbr == 0 )
    {
        return 0;
    }

    return nbr*expo_reccur(nbr, exp-1);
}

// commands

int cmd_echo(int *inBuff, int buffSize)// code: ec
{
    if ( buffSize < 3 )
    {
        return 0;
    }
    int k = 0;

    for(k=3; k!=buffSize; k++)
    {
        putchar_raw(inBuff[k]);
    }
    printf("\n");

    return 0;
}



int what_command(int *inBuff, int buffSize)
{
    int curr_c = 0;
    uint8_t k = 0;
    int command_code = 0;
    

    // find the command
    for(k=0; k < 2; k++)
    {
        curr_c = inBuff[k];
        // DBG
        //printf("k=%d and curr_c=%d  =>  %d\n", k, curr_c, curr_c*expo_reccur(10, k*3));
        
        command_code += curr_c*expo_reccur(10, k*3);
    }
    // DBG CONSOLE
    if ( DBG_CONSOLE_FLAG )
    {
        printf("\nYOU ENTERED CODE: %d\n", command_code);
    }

    switch (command_code)
    {
        case 111112:
            RUN_CONSOLE = 0;
            POWER_ON = 0;
            break;

        case 120101:
            RUN_CONSOLE = 0;
            break;

        case 99101:
            cmd_echo(inBuff, buffSize);
            break;

        default:
            return -1;
            break;
    }


    return 0;
}

int set_cmd_line(void)
{
    printf("root@picoWH# ");

    return 0;
}

int main()
{
    POWER_ON = 1;
    int input_string[256];
    int c = 0;
    uint32_t console_timeout = 2000000;
    int i;
    int j;

    stdio_init_all();

    while ( POWER_ON ) // STARTUP CONSOLE
    {
        DBG_CONSOLE_FLAG = 0;
        RUN_CONSOLE = 0;

        busy_wait_ms(500);

        // conn challenge
        while( !RUN_CONSOLE )
        {
            while( c != 13 )// Waits until [ENTER] is pressed
            {
                c = getchar();
                if ( c == 97 )
                {
                    c = getchar();
                    if ( c == 122 )
                    {
                        c = getchar();
                        if ( c == 101 )
                        {
                            c = getchar();
                            if ( c == 114 )
                            {
                                c = getchar();
                                if ( c == 116 )
                                {
                                    c = getchar();
                                    if ( c == 121 )
                                    {
                                        RUN_CONSOLE = 1;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            c = 0;
        }
        
        printf("Welcome to Pico WH !\n");
        printf("This is a terminal emulator loaded in.\n");
        printf("\nType \"he\" to get the full list of available commands\n\n");

        set_cmd_line();

        i = 0;
        while( RUN_CONSOLE )// EXIT shuts down the console
        {

            // read input console
            c = getchar_timeout_us(console_timeout);
            if ( c != PICO_ERROR_TIMEOUT )
            {
                // if something was typed, print it
                // first, test if buff is full
                if ( i == sizeof(input_string)-1 ) // if buffer is full, flush it
                {

                    input_string[i] = 0;

                    printf("BUFFER FULL, COMMAND TOO LONG\n");

                    set_cmd_line();

                    i=0;

                }

                if ( c == 13 ) // if [ENTER] was pressed, flush buffer and find the command
                {

                    input_string[i] = 0;
                    printf("\n");

                    if ( i > 1 )
                    {
                        if ( what_command(input_string, i) < 0 )
                        {
                            printf("Command not found...\n");
                        }
                    }

                    set_cmd_line();

                    i=0;

                }else if ( c == 4 ) // if [CTRL+D] was pressed, enable debugging
                {

                    printf("\nCONSOLE DEBUG TOGGLED\n");
                    DBG_CONSOLE_FLAG = !DBG_CONSOLE_FLAG;
                    i=0;

                }else if ( c == 8 && i != 0)// [BACKSPACE] handling
                {
                    printf("\n");
                    i--;
                    set_cmd_line();
                    for(j=0; j != i; j++)
                    {
                        putchar_raw(input_string[j]);
                    }
                }
                else
                {
                    // DBG CONSOLE
                    if ( DBG_CONSOLE_FLAG )
                    {
                        printf("%d", c);
                        input_string[i] = c;
                        i++;
                    }else
                    {
                        // NORMAL OPERATION
                        putchar_raw(c);
                        input_string[i] = c;
                        i++;
                    }
                }

            }else
            {
                // else, print stuff and keep doing things
                continue;
            }
        }
        // console EXIT
        printf("Goodbye !\n");


    }

    return 0;
}
