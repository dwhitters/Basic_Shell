/******************************************************************
 * @authors Jonah Bukowsky, and David Whitters
 * @date 1/24/18
 *
 * CIS 452
 * Dr. Dulimarta
 *****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/resource.h>

/** The maximum number of characters that will be analyzed. */
#define MAX_NUM_INPUT_CHARS 100u
/** The delimiter character for the user-input lines. */
#define DELIMITER_CHAR " "

/**
    Splits the string into an arguments list.

    @param input
        Pointer to the user-input command string.
    @param args
        All strings stored in this array.
*/
void SplitInputString(char * input, char * args[])
{
    /* Get the arguments until there are no more space delimited strings. */
    int i = 0u;
    for(i = 0u; (args[i] = strsep(&input, DELIMITER_CHAR)) != NULL; ++i)
    {
        /* Don't consider extra spaces to be arguments. */
        if(strcmp(args[i], "") == 0u)
        {
            --i; /* Overwrite the empty string. */
        }
    }
}

int main(int argc, char * argv[])
{
    /* Holds user input. */
    char input[MAX_NUM_INPUT_CHARS] = {0};
    char ** args;

    /* Allocate memory for an array of character pointers. */
    args = (char **)malloc(MAX_NUM_INPUT_CHARS);

    /* Process the input if the command isn't "quit". */
    while(1u)
    {
        /* Prompt user for a command. */
        printf("Enter Command: ");
        /* Read the user input. */
        fgets(input, MAX_NUM_INPUT_CHARS, stdin);
        /* Remove newline character. */
        input[strcspn(input, "\n")] = 0;

        /* Make sure the user input a string. */
        if(strlen(input) > 0u)
        {
            /* Split the string into its args. */
            SplitInputString(input, args);

            /* Process command if the input isn't "quit". */
            if(strcmp(args[0], "quit") != 0u)
            {
                /*
                    Variables for resource usage statistics about
                    each executed process.
                */
                struct rusage r;
                long previous_sec = 0;
                long previous_usec = 0;
                long previous_switch = 0;

                /* Holds ID of the parent process. */
                pid_t pid;
                int status;

                /* Exit on failure. */
                if((pid = fork()) < 0)
                {
                    perror("FORK FAILURE");
                    exit(EXIT_FAILURE);
                }
                else if(pid == 0)
                {
                    /* Child process execution. */
                    if(execvp(args[0], args) < 0)
                    {
                        perror("EXEC FAILURE");
                        exit(EXIT_FAILURE);
                    }
                    else
                    {
                        exit(EXIT_SUCCESS);
                    }
                }
                else
                {
                    /* Parent process execution. */
                    (void)wait(&status);

                    if (getrusage(RUSAGE_CHILDREN, &r) < 0)
                    {
                        printf("Could not get stats on process!\n");
                    }
                    else
                    {
                        /*
                            Output the user CPU time used for each
                            child process spawned by the shell.
                        */
                        printf("\nUser cpu time:\t%ld seconds, %ld microseconds\n",
                                r.ru_utime.tv_sec - previous_sec,
                                r.ru_utime.tv_usec - previous_usec);

                        /*
                            Output the number of involuntary context
                            switches experienced by each child process
                            spawned by the shell.
                        */
                        printf("involuntary context switches:\t%ld\n\n",
                                r.ru_nivcsw - previous_switch);
                        previous_sec = r.ru_utime.tv_sec;
                        previous_usec = r.ru_utime.tv_usec;
                        previous_switch = r.ru_nivcsw;
                    }
                }
            }
            else
            {
                break;
            }
        }
    }

    /* Free the allocated memory. */
    free(args);

    return 0;
}
