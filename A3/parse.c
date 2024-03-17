/*****************************************************************************/
/*                           CSC209-24s A3 CSCSHELL                          */
/*       Copyright 2024 -- Demetres Kostas PhD (aka Darlene Heliokinde)      */
/*****************************************************************************/

#include "cscshell.h"
#include <ctype.h>

#define CONTINUE_SEARCH NULL
#define VAR_NAME_MAX_LEN 100
#define VAR_VALUE_MAX_LEN 100


char* trim_whitespace(char* str) {
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) 
        return str;
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end+1) = 0;
    return str;
}

int is_empty_or_comment(char* line) {
    line = trim_whitespace(line);
    return *line == '\0' || *line == '#';
}

/*
** The following functions are provided for you in _shell.c
** You should modify them as needed, but do *not* change their signatures
**
** If they are marked as "COMPLETE", do not change them!
*/

/*
** Parses a single line of text and returns a linked list of commands.
** The last command in the list has a next pointer that points to NULL.
**
** Return possibilities:
** 1. The first in a list of commands that should execute roughly
**    simultaneously (see instructions for details).
**
** 2. NULL if successfully parsed line, with *no commands*, this happens:
**    -- Case 1: Empty line
**    -- Case 2: Line is /exclusively/ a comment (i.e. first non-whitespace
**               char is '#'). Comments may also trail commands or assignments.
**               You must handle text before '#' characters.
**    -- Case 3: Shell variable assignment (e.g. VAR=VALUE)
**       -- The variable should added to the variables list
**       -- or updated if the variable already exists
**
** 3. If there is an error, returns -1 cast as a (Command *)
*/
Command *parse_line(char *line, Variable **variables) {
    if (line == NULL || is_empty_or_comment(line)) return NULL;

    char* trimmed_line = trim_whitespace(line);

    char* equal_sign = strchr(trimmed_line, '=');
    if (equal_sign != NULL) {
        
        // *equal_sign = '\0';
        // TODO: bugcheck this
        // check if illegal chars in variable (whitespace or number) 
        //

        // Temporarily terminate the string at the equal sign
        *equal_sign = '\0';

        // Assign the stuff before the equal sign to name
        char* name = strdup(trimmed_line);

        // Restore the equal sign
        *equal_sign = '=';

        // Assign the stuff after the equal sign to value
        char* value = strdup(equal_sign + 1);

        // Now you can use name and value...
        // Don't forget to free them when you're done
        printf("name: %s\n", name);
        printf("value: %s\n", value);
        
        for (int i = 0; name[i] != '\0'; i++) {
            if (isdigit(name[i]) || isspace(name[i])) {
                // Handle error: variable name contains a number or space
                return NULL;
            }
        }

        if (variables == NULL) {
            return NULL; // Can't do anything if variables is NULL
        }
        Variable *temp = *variables;
        if (temp == NULL) {
            // No variables yet, create the first one
            temp = (Variable *) malloc(sizeof(Variable));
            if (temp == NULL) {
                return NULL;
            }
            temp->name = strdup(name);
            temp->value = strdup(value);
            temp->next = NULL;
            *variables = temp; // Update the original pointer
        } else {
            // Find the variable with the same name or the last variable in the list
            Variable *prev = NULL;
            while (temp != NULL && strcmp(temp->name, name) != 0) {
                prev = temp;
                temp = temp->next;
            }
            if (temp != NULL) {
                // Variable with the same name found, update its value
                free(temp->value); // Free the old value first to avoid memory leaks
                temp->value = strdup(value);
            } else {
                // No variable with the same name found, create a new one
                temp = (Variable *) malloc(sizeof(Variable));
                if (temp == NULL) {
                    return NULL;
                }
                temp->name = strdup(name);
                temp->value = strdup(value);
                temp->next = NULL;
                prev->next = temp; // Add the new variable to the end of the list
            }
        }
        free(name); // Don't forget to free name and value when you're done with them
        free(value);

        return NULL;
    
    }

    Command **cmds = (Command **)malloc(sizeof(Command *));
    if (cmds == NULL) {
        return NULL;
    }

    char *token = strtok(line, " \t\n");
    if (token == NULL) {
        return NULL;
    }

    Command *cmd = (Command *)malloc(sizeof(Command));
    if (cmd == NULL) {
        return NULL;
    }

    char *executable = resolve_executable(strdup(token), *variables);
    if (executable == NULL) {
        return NULL;
    }

    cmd->exec_path = executable;
    if (cmd->exec_path == NULL) {
        return NULL;
    }

    cmd->args = (char **)malloc(sizeof(char *));
    if (cmd->args == NULL) {
        return NULL;
    }

    cmds[0] = cmd;

    char *temp_args[100];
    int i = 0;
    while ((token = strtok(NULL, " \t\n")) != NULL) {

        if (token[0] == '>') {
            token = strtok(NULL, " \t\n");
            // migt want to use heap for strings TODO
            cmd->redir_out_path = strdup(token);


        } else if (token[0] == '<') {
            token = strtok(NULL, " \t\n");
            cmd->redir_in_path = strdup(token);

        } else if (token[0] == '|') {

            token = strtok(NULL, " \t\n");
            char *next_exec = resolve_executable(strdup(token), *variables);
            if (next_exec == NULL) {
                return NULL;
            }

            Command *new_cmd = (Command *)malloc(sizeof(Command));
            if (new_cmd == NULL) {
                return NULL;
            }

            cmd->next = new_cmd;
            cmd = cmd->next;
            cmd->exec_path = next_exec;

            cmd->args = (char **)malloc(sizeof(char *));
            for (int j = 0; j <= i; j++) {
                cmd->args[j] = strdup(temp_args[j]);
            }
            i = 0;
        } else {
            cmd->args = (char **)realloc(cmd->args, sizeof(char *) * (i + 2)); // +2 to account for the current argument and NULL terminator
            if (cmd->args == NULL) {
                return NULL;
            }
            cmd->args[i] = strdup(token);
            cmd->args[++i] = NULL; // NULL terminate the args array
        }
    }

    cmd->next = NULL;

    return *cmds;
}

// COMPLETE
char *resolve_executable(const char *command_name, Variable *path){

    if (command_name == NULL || path == NULL){
        return NULL;
    }

    if (strcmp(command_name, CD) == 0){
        return strdup(CD);
    }

    if (strcmp(path->name, PATH_VAR_NAME) != 0){
        ERR_PRINT(ERR_NOT_PATH);
        return NULL;
    }

    char *exec_path = NULL;

    if (strchr(command_name, '/')){
        exec_path = strdup(command_name);
        if (exec_path == NULL){
            perror("resolve_executable");
            return NULL;
        }
        return exec_path;
    }

    // we create a duplicate so that we can mess it up with strtok
    char *path_to_toke = strdup(path->value);
    if (path_to_toke == NULL){
        perror("resolve_executable");
        return NULL;
    }
    char *current_path = strtok(path_to_toke, ":");

    do {
        DIR *dir = opendir(current_path);
        if (dir == NULL){
            ERR_PRINT(ERR_BAD_PATH, current_path);
            closedir(dir);
            continue;
        }

        struct dirent *possible_file;

        while (exec_path == NULL) {
            // rare case where we should do this -- see: man readdir
            errno = 0;
            possible_file = readdir(dir);
            if (possible_file == NULL) {
                if (errno > 0){
                    perror("resolve_executable");
                    closedir(dir);
                    goto res_ex_cleanup;
                }
                // end of files, break
                break;
            }

            if (strcmp(possible_file->d_name, command_name) == 0){
                // +1 null term, +1 possible missing '/'
                size_t buflen = strlen(current_path) +
                    strlen(command_name) + 1 + 1;
                exec_path = (char *) malloc(buflen);
                // also sets remaining buf to 0
                strncpy(exec_path, current_path, buflen);
                if (current_path[strlen(current_path)-1] != '/'){
                    strncat(exec_path, "/", 2);
                }
                strncat(exec_path, command_name, strlen(command_name)+1);
            }
        }
        closedir(dir);

        // if this isn't null, stop checking paths
        if (possible_file) break;

    } while ((current_path = strtok(CONTINUE_SEARCH, ":")));

res_ex_cleanup:
    free(path_to_toke);
    return exec_path;
}

/*
** This function is partially implemented for you, but you may
** scrap the implementation as long as it produces the same result.
**
** Creates a new line on the heap with all named variable *usages*
** replaced with their associated values.
**
** Returns NULL if replacement parsing had an error, or (char *) -1 if
** system calls fail and the shell needs to exit.
*/
char *replace_variables_mk_line(const char *line, Variable *variables){
    // NULL terminator accounted for here
    size_t new_line_length = strlen(line) + 1;
    char markers[new_line_length];

    memset(markers, '-', sizeof(markers));
    markers[sizeof(markers) - 1] = '\0';

    // Code to determine new length
    // and list of replacements in order
    size_t replacements_count = 0;
    Variable *current_variable = variables;
    while (current_variable != NULL) {
        const char *var_usage = strstr(line, current_variable->name);
        while (var_usage != NULL) {
            size_t var_usage_index = var_usage - line;
            memset(&markers[var_usage_index], ' ', strlen(current_variable->name));
            var_usage = strstr(var_usage + strlen(current_variable->name), current_variable->name);
            replacements_count++;
        }
        current_variable = current_variable->next;
    }

    char *new_line = (char *)malloc(new_line_length + replacements_count * (VAR_VALUE_MAX_LEN - VAR_NAME_MAX_LEN));
    if (new_line == NULL) {
        perror("replace_variables_mk_line");
        return (char *) -1;
    }
    memset(new_line, '\0', new_line_length + replacements_count * (VAR_VALUE_MAX_LEN - VAR_NAME_MAX_LEN));

    // Fill in the new line using markers and replacements
    size_t new_line_index = 0;
    size_t line_index = 0;
    while (line[line_index] != '\0') {
        if (markers[line_index] == '-') {
            new_line[new_line_index] = line[line_index];
            new_line_index++;
        } else {
            Variable *current_variable = variables;
            while (current_variable != NULL) {
                if (strncmp(&line[line_index], current_variable->name, strlen(current_variable->name)) == 0) {
                    strcat(new_line, current_variable->value);
                    new_line_index += strlen(current_variable->value);
                    break;
                }
                current_variable = current_variable->next;
            }
            line_index += strlen(current_variable->name) - 1;
        }
        line_index++;
    }

    return new_line;
}

// maybe done
void free_variable(Variable *var, uint8_t recursive){
    if (var == NULL) {
        return;
    }
    
    if (recursive) {
        free_variable(var->next, recursive);
    }
    free(var->name);
    free(var->value);
    free(var);
}
