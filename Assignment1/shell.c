#include "parser/ast.h"
#include "shell.h"
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

void signalHandler(){   
    //signal(SIGTSTP, signalHandler);
    perror(""); 
}

void initialize(void)
{
    /* This code will be called once at startup */
    if (prompt)
        prompt = "vush$ ";
}

void handlecmd(node_t *n){
    //signal(SIGTSTP, signalHandler);
    int status;
    pid_t pid;

    if (strcmp("exit", n->command.program) == 0){
        int exit_number = atoi(n->command.argv[1]);
        exit(exit_number);
    }

    if (strcmp("cd",n->command.program) == 0){
        chdir(n->command.argv[1]);
        return;
    }

    pid = fork();

    if (pid == 0){
        if(execvp(n->command.program, n->command.argv) == -1){
            perror("No such file or directory");
        } else {
            exit(0);
        }
    } else {
        wait(&status);
    }
}


void handlepipe(node_t *n){
    pid_t first, second;
    int fd[2];
    pipe(fd);

    first = fork();

    if(first == 0){
        close(fd[0]);
        close(1);
        dup(fd[1]);
        execvp(n->pipe.parts[0]->command.program, n->pipe.parts[0]->command.argv);
    }

    second = fork();
    
    if (second == 0){
        close(fd[1]);
        close(0);
        dup(fd[0]);
        execvp(n->pipe.parts[1]->command.program, n->pipe.parts[1]->command.argv);
    }

    close(fd[0]);
    close(fd[1]);

    waitpid(first, NULL, 0);
    waitpid(second, NULL,0);
}

void handleseq(node_t *n){
    run_command(n->sequence.first);
    run_command(n->sequence.second);
}


void run_command(node_t *node)
{
    pid_t pid;
    int status;
    /* Print parsed input for testing - comment this when running the tests! */
    //print_tree(node);

    signal(SIGINT, signalHandler);
    //signal(SIGTSTP, signalHandler);

    switch (node->type)
    {
    case NODE_COMMAND:
        handlecmd(node);
        break;
    case NODE_PIPE:
        handlepipe(node);
        break;
    case NODE_SEQUENCE:
        handleseq(node);
        break;
    case NODE_SUBSHELL:
        pid = fork();
        if (pid == 0)
            run_command(node->subshell.child->subshell.child);
        else {
            wait(&status);
        }
        break;
    case NODE_DETACH:
        pid = fork();
        if (pid == 0){
            run_command(node->detach.child);
            exit(0);
        }
        break;
    default:
        break;
    }

    if (prompt)
        prompt = "vush$ ";
}
