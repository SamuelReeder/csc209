#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 256

#define SUCCESS "Password verified\n"
#define INVALID "Invalid password\n"
#define NO_USER "No such user\n"

int main(void)
{
    char user_id[MAXLINE];
    char password[MAXLINE];

    /* The user will type in a user name on one line followed by a password
       on the next.
       DO NOT add any prompts.  The only output of this program will be one
       of the messages defined above.
       Please read the comments in validate carefully
     */

    if (fgets(user_id, MAXLINE, stdin) == NULL)
    {
        perror("fgets");
        exit(1);
    }
    if (fgets(password, MAXLINE, stdin) == NULL)
    {
        perror("fgets");
        exit(1);
    }

    // TODO
    // account for new line character with MAX_PASSWORD
    if (strlen(user_id) > 11 || strlen(password) > 11)
    {
        exit(1);
    }

    int status;
    int fd[2];
    if (pipe(fd) == -1)
    {
        perror("pipe");
        exit(1);
    }

    int inpipefd[2];
    pipe(inpipefd);

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(1);
    }
    else if (pid == 0)
    {
        close(fd[0]);
        close(inpipefd[1]);
        dup2(fd[1], STDOUT_FILENO);
        dup2(inpipefd[0], STDIN_FILENO);
        close(fd[1]);
        close(inpipefd[0]);
        execl("./validate", "validate", (char *)NULL);
        perror("execl");
        exit(1);
    }
    else
    {
        close(fd[1]);
        close(inpipefd[0]);
        write(inpipefd[1], user_id, 10);
        write(inpipefd[1], password, 10);
        close(inpipefd[1]);
        wait(&status);
        if (!WIFEXITED(status))
        {
            exit(-1);
        }
        switch (WEXITSTATUS(status))
        {
        case 0:
            printf(SUCCESS);
            break;
        case 1:
            exit(0);
        case 2:
            printf(INVALID);
            break;
        case 3:
            printf(NO_USER);
            break;
        default:
            exit(0);
        }
    }

    return 0;
}
