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
    if (!return_code) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    if (!head) { // no commands to execute
        *return_code = 0;
        return return_code;
    }

    int input_fd = 0; // start with standard input
    int pipefd[2];
    Command *cmd = head;
    pid_t last_pid;

    while (cmd != NULL) {
        if (cmd->next) {
            if (pipe(pipefd) == -1) {
                perror("pipe");
                *return_code = -1;
                return return_code;
            }
        }

        // setup redirections for current command if necessary
        if (cmd->redir_in_path) {
            cmd->stdin_fd = open(cmd->redir_in_path, O_RDONLY);
            if (cmd->stdin_fd == -1) {
                perror("open input redirection");
                *return_code = -1;
                return return_code;
            }
        } else {
            cmd->stdin_fd = input_fd;
        }

        if (cmd->redir_out_path) {
            int flags = O_WRONLY | O_CREAT | (cmd->redir_append ? O_APPEND : O_TRUNC);
            cmd->stdout_fd = open(cmd->redir_out_path, flags, 0644);
            if (cmd->stdout_fd == -1) {
                perror("open output redirection");
                *return_code = -1;
                return return_code;
            }
        } else if (cmd->next) { // pipe next
            cmd->stdout_fd = pipefd[1];
        } else { // last so standard output
            cmd->stdout_fd = STDOUT_FILENO;
        }

        last_pid = run_command(cmd);

        if (cmd->redir_in_path && cmd->stdin_fd != STDIN_FILENO) {
            close(cmd->stdin_fd);
        }

        if (input_fd != STDIN_FILENO) {
            close(input_fd); 
        }
        input_fd = pipefd[0]; 

        if (cmd->next) {
            close(pipefd[1]);
        }

        cmd = cmd->next;
    }

    if (waitpid(last_pid, return_code, 0) == -1) {
        perror("waitpid");
        *return_code = -1;
    } else {
        *return_code = WEXITSTATUS(*return_code);
    }

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


    // #ifdef DEBUG
    // printf("Parent process created child PID [%d] for %s\n", pid, command->exec_path);
    // #endif

     if (command == NULL || command->args == NULL || command->args[0] == NULL) {
        perror("run_command");
        return -1;
    }

    if (strcmp(command->args[0], "cd") == 0) {
        // call cd_cscshell instead of execvp
        return cd_cscshell(command->args[1]);
    }

    pid_t child = fork();
    if (child < 0){
        perror("fork in run_command");
        return -1;
    }

    if (child == 0){
        // child
        if (command->redir_in_path != NULL){
            int in_fd = open(command->redir_in_path, O_RDONLY);
            if (in_fd < 0) {
                perror("open input redirection file");
                exit(EXIT_FAILURE);
            }
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }

        if (command->redir_out_path != NULL){
            int flags = O_WRONLY | O_CREAT | (command->redir_append ? O_APPEND : O_TRUNC);
            int out_fd = open(command->redir_out_path, flags, 0644);
            if (out_fd < 0) {
                perror("open output redirection file");
                exit(EXIT_FAILURE);
            }
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }

        // for (int i = 0; command->args[i] != NULL; i++) {
        //     printf("args[%d]: %s\n", i, command->args[i]);
        // }

        // Execute the command
        if (execvp(command->exec_path, command->args) < 0){
            perror("execvp in run_command");
            exit(EXIT_FAILURE);
        }
    }

    return child;  // return child PID
}


int run_script(char *file_path, Variable **root){
    FILE *file = fopen(file_path, "r");
    if (file == NULL){
        perror("run_script");
        return -1;
    }

    char line[MAX_SINGLE_LINE];
    while (fgets(line, MAX_SINGLE_LINE, file) != NULL){
        // kill the newline
        line[strlen(line)] = '\0';

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

        // free the commands
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
