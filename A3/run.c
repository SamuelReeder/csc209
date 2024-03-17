/*****************************************************************************/
/*                           CSC209-24s A3 CSCSHELL                          */
/*       Copyright 2024 -- Demetres Kostas PhD (aka Darlene Heliokinde)      */
/*****************************************************************************/

#include "cscshell.h"


// COMPLETE
int cd_cscshell(const char *target_dir){
    if (target_dir == NULL) {
        char user_buff[MAX_USER_BUF];
        if (getlogin_r(user_buff, MAX_USER_BUF) != 0) {
           perror("run_command");
           return -1;
        }
        struct passwd *pw_data = getpwnam((char *)user_buff);
        if (pw_data == NULL) {
           perror("run_command");
           return -1;
        }
        target_dir = pw_data->pw_dir;
    }

    if(chdir(target_dir) < 0){
        perror("cd_cscshell");
        return -1;
    }
    return 0;
}


int *execute_line(Command *head){
    #ifdef DEBUG
    printf("\n***********************\n");
    printf("BEGIN: Executing line...\n");
    #endif

    #ifdef DEBUG
    printf("All children created\n");
    #endif

    // Wait for all the children to finish

    #ifdef DEBUG
    printf("All children finished\n");
    #endif

    #ifdef DEBUG
    printf("END: Executing line...\n");
    printf("***********************\n\n");
    #endif

    int *return_code = malloc(sizeof(int));
    if (return_code == NULL) {
        return NULL;
    }

    int pipefd[2];
    int input_fd = 0; // Start with standard input
    pid_t pid;

    Command *cmd = head;
    while (cmd != NULL) {
        if (pipe(pipefd) == -1) {
            perror("pipe");
            *return_code = -1;
            return return_code;
        }

        pid = fork();
        if (pid == 0) {
            // Child process
            if (input_fd != 0) {
                dup2(input_fd, 0); // Replace standard input with input pipe
                close(input_fd);
            }
            if (cmd->next != NULL) {
                dup2(pipefd[1], 1); // Replace standard output with output pipe
                close(pipefd[1]);
            }
            close(pipefd[0]);

            if (execvp(cmd->args[0], cmd->args) == -1) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        } else if (pid < 0) {
            // Fork failed
            perror("fork");
            *return_code = -1;
            return return_code;
        }

        // Parent process
        close(pipefd[1]);
        if (input_fd != 0) {
            close(input_fd);
        }
        input_fd = pipefd[0]; // Save the input pipe for the next command

        cmd = cmd->next;
    }

    // Wait for the last command to finish and save its return code
    waitpid(pid, return_code, 0);
    *return_code = WEXITSTATUS(*return_code);

    return return_code;
}


/*
** Forks a new process and execs the command
** making sure all file descriptors are set up correctly.
**
** Parent process returns -1 on error.
** Any child processes should not return.
*/
int run_command(Command *command){
    #ifdef DEBUG
    printf("Running command: %s\n", command->exec_path);
    printf("Argvs: ");
    if (command->args == NULL){
        printf("NULL\n");
    }
    else if (command->args[0] == NULL){
        printf("Empty\n");
    }
    else {
        for (int i=0; command->args[i] != NULL; i++){
            printf("%d: [%s] ", i+1, command->args[i]);
        }
    }
    printf("\n");
    printf("Redir out: %s\n Redir in: %s\n",
           command->redir_out_path, command->redir_in_path);
    printf("Stdin fd: %d | Stdout fd: %d\n",
           command->stdin_fd, command->stdout_fd);
    #endif


    #ifdef DEBUG
    printf("Parent process created child PID [%d] for %s\n", pid, command->exec_path);
    #endif

    int child = fork();
    if (child < 0){
        perror("run_command");
        return -1;
    }
    if (child == 0){
        // Child process
        if (command->stdin_fd != STDIN_FILENO){
            if (dup2(command->stdin_fd, STDIN_FILENO) < 0){
                perror("run_command");
                exit(EXIT_FAILURE);
            }
        }
        if (command->stdout_fd != STDOUT_FILENO){
            if (dup2(command->stdout_fd, STDOUT_FILENO) < 0){
                perror("run_command");
                exit(EXIT_FAILURE);
            }
        }

        if (execv(command->exec_path, command->args) < 0){
            perror("run_command");
            exit(EXIT_FAILURE);
        }
    }
    else {
        // Parent process
        #ifdef DEBUG
        printf("Parent process created child PID [%d] for %s\n", child, command->exec_path);
        #endif

        return child;  // Return child PID

    }

    return -1;
}

/*
** Executes an entire script line-by-line.
** Stops and indicates an error as soon as any line fails.
**
** Returns 0 on success, -1 on error
*/
int run_script(char *file_path, Variable **root){
    FILE *file = fopen(file_path, "r");
    if (file == NULL){
        perror("run_script");
        return -1;
    }

    char line[MAX_SINGLE_LINE];
    while (fgets(line, MAX_SINGLE_LINE, file) != NULL){
        // kill the newline
        line[strlen(line) - 1] = '\0';

        Command *commands = parse_line(line, root);
        if (commands == (Command *) -1){
            ERR_PRINT(ERR_PARSING_LINE);
            fclose(file);
            return -1;
        }
        if (commands == NULL) continue;

        int *last_ret_code_pt = execute_line(commands);
        if (last_ret_code_pt == NULL || *last_ret_code_pt == -1){
            ERR_PRINT(ERR_EXECUTE_LINE);
            free(last_ret_code_pt);
            fclose(file);
            return -1;
        }
        free(last_ret_code_pt);

        // Free the commands
        Command *tmp;
        while (commands != NULL) {
            tmp = commands;
            commands = commands->next;
            free(tmp);
        }
    }

    fclose(file);
    return 0;
}

void free_command(Command *command) {
    if (command == NULL) return;
    free(command->exec_path);
    for (int i=0; command->args[i] != NULL; i++){
        free(command->args[i]);
    }
    free(command->args);
    free(command->redir_in_path);
    free(command->redir_out_path);
    free(command);
}
