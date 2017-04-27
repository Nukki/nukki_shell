/**
 file: nukki_shell.c
 author: Nikki Jack
 date created: 04/18/2017
 course: CSCI 340

 nsh is a simple shell program that has following features:
 - can run a process in background if there is "&" at the end
 - has "history" feature that shows last 10 commands entered by user
 - to run last command enter "!!"
 - has a shortcut way to call a command from history by entering "!<number_in_the_list>"

 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>


#define MAX_LINE    80 /* 80 chars per line, per command */

char *trim_line(char *line) {
    char *end;

    /* trim leading space */
    while(isspace((unsigned char)*line))
      line++;

    if(*line == 0)  /* all spaces? */
      return line;

    /* trim trailing space */
    end = line + strlen(line) - 1;
    while(end > line && isspace((unsigned char)*end))
      end--;

    /* write new null terminator */
    *(end+1) = 0;
    return line;
}



/* parses input and returns index of ampersand is there is one
   or -1 if there is no ampersand among the args */
int  parse_input(char *line, char **args) {

    char *copy = trim_line(line); /* get rid of white space */
    int i = 0;                    /* args array index counter */
    int result = -1;

    /* separate the words in a line */
    while (copy != '\0'){                /* until the end of line */
        char delim = ' ';                /* use space as delimeter */
        args[i] = strsep(&copy, &delim); /* put a separated token into args array */

        /* its an ampersand */
        if (strcmp(args[i], "&") == 0){
            args[i] = NULL;         /* don't put it into args array */
            result = i;             /* put index of ampersand to result */
        }                           /* to indicate that command contains "&" */

        i++;                        /* increment the args index counter */
    }

    args[i] = NULL;                 /* last arg needs to be NULL */
    return result;
}



void run_command(char **args, int ampersand) {
    pid_t pid;
    int status;
    pid = fork();     /* create a new process */

    if ( pid < 0 ){    /* something went wrong */
        perror("Fork failed");
        exit(1);
    }
    else if (pid == 0){  /* child process */

        if (ampersand != -1) { /* no ampersand */
            setpgid(0, 0);     /* detach a child from current process group */
        }
        execvp(args[0],args);
        exit(0);
    }
    else {              /* parent process */

        if (ampersand == -1) {  /* no ampersand */
            wait(NULL);         /* wait for child to terminate */
        }
        else {                  /* there is ampersand */
            printf("Forked a child with PID: %i\n", (int) pid);
            /* return immediately if no child has exited */
            waitpid(pid,&status, WNOHANG);
        }
    }   // end outer else
}

void put_in_history(char *line, char **history, int last) {
  /* allocate space for copy on heap */
  char *line_copy = (char *)malloc(MAX_LINE * sizeof(char));
  strcpy(line_copy, line);
  history[last] = line_copy;
}


void print_history(char **history_array, int last_history_index) {

  /* determine the stop looping condition */
  int stop_looping_point = 0;
  if (last_history_index > 10) {
    stop_looping_point = last_history_index - 10;
  }

  /* print last 10 commands */
  for (int i = last_history_index; i > stop_looping_point; --i) {
    printf("%i %s\n", i, history_array[i]);
  }
}


int main(void) {

    char *args[MAX_LINE/2 + 1];         /* command line (of 80) has max of 40 arguments */
    char line[MAX_LINE + 1];            /* user input goes here */
    int should_run = 1;                 /* a flag that keeps while loop running */
    char *history_array[2000];          /* history of commands is going to be stored here */
    int last_history_index = 0;         /* last occupied index of the history array*/
    history_array[last_history_index] = "spaceholder";

    while (should_run) {
        printf("nsh> ");                /* show prompt */
        fgets(line,MAX_LINE,stdin);     /* get user input and save it to "line" */

        if (strcmp(line, "\n") == 0) {  /* user entered nothing */
               printf("Please enter a command! 🤓 \n");
        }
        else {                          /* user enetred something */

            /* clone user input so that the original input
               could be put into history */
            char line_clone[MAX_LINE +1];
            strcpy(line_clone,line);

            /* parse input and get an ampersand int flag
               which indicates if command contained "& "*/
            int ampersand = parse_input(line_clone, args);

            /******************************************************************
            *                                                                 *
            *                     PROCESS  USER  INPUT                        *
            *                                                                 *
            *******************************************************************/

            /* user entered "exit" */
            if (strcmp(args[0], "exit") == 0) {
                printf("Have a nice day! \n");
                /* break this while loop, i.e. terminate this shell */
                should_run = 0;
            }

            /* user entered "history" */
            else if (strcmp("history", args[0]) == 0) {
                print_history(history_array, last_history_index);
            }

            /* its a shortcut command that starts with "!" */
            else if (args[0][0] == '!') {
                if (args [0][1] == '!') {    /* run last command */
                  /* parse last command from history and get its ampersand flag */
                  ampersand = parse_input(history_array[last_history_index], args);
                  run_command(args, ampersand);
                }
                else { /* run command under specified number from history list */
                  /* get command number that comes after "!" */
                  int command_number = atoi(args[0]+1);

                  /* check if the command number is valid */
                  if (command_number > 0 && command_number <= last_history_index ) {
                    /* parse command from history and get its ampersand flag*/
                    ampersand = parse_input(history_array[command_number], args);
                    run_command(args, ampersand);
                  }
                }     // end else
            }         // end else if for shortcut commands

            /* user entered a console command */
            else {
                last_history_index++;     /* go to next history array slot */
                put_in_history(trim_line(line), history_array, last_history_index);
                run_command(args, ampersand);
            }
        }   // end very first else

        fflush(stdout);
    }   // end while loop

    return 0;
}
