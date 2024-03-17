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


Command *parse_line(char *line, Variable **variables) {
    if (line == NULL || is_empty_or_comment(line)) return NULL;
    
    char* trimmed_line = trim_whitespace(line);

    char* equal_sign = strchr(trimmed_line, '=');
    if (equal_sign != NULL) {
        
        *equal_sign = '\0';
        // assign the stuff before the equal sign to name
        char* name = strdup(trimmed_line);
        *equal_sign = '=';

        // assign the stuff after the equal sign to value
        char* value = strdup(equal_sign + 1);

        // printf("name: %s\n", name);
        // printf("value: %s\n", value);
        
        for (int i = 0; name[i] != '\0'; i++) {
            if (isdigit(name[i]) || isspace(name[i])) {
                perror("parse_line");
                return NULL;
            }
        }

        if (variables == NULL) {
            return NULL; // can't do anything if variables is NULL
        }
        Variable *temp = *variables;
        if (temp == NULL) {
            // no variables yet, create first one
            temp = (Variable *) malloc(sizeof(Variable));
            if (temp == NULL) {
                return NULL;
            }
            temp->name = strdup(name);
            temp->value = strdup(value);
            temp->next = NULL;
            *variables = temp;
        } else {
            Variable *prev = NULL;
            while (temp != NULL && strcmp(temp->name, name) != 0) {
                prev = temp;
                temp = temp->next;
            }
            if (temp != NULL) {
                // variable with same name found, update its value
                free(temp->value);
                temp->value = strdup(value);
            } else {
                temp = (Variable *) malloc(sizeof(Variable));
                if (temp == NULL) {
                    return NULL;
                }
                temp->name = strdup(name);
                temp->value = strdup(value);
                temp->next = NULL;
                prev->next = temp;
            }
        }
        free(name); 
        free(value);

        return NULL;
    
    }

    Command *head = NULL; 
    Command *current = NULL;

    char *line_copy = strdup(line);
    if (line_copy == NULL) {
        return (Command *)-1;
    }
    char *saveptr;
    char *token = strtok_r(line_copy, " \t\n", &saveptr);

    while (token != NULL) {
        // ensure a new Command struct is created either at the start or following a pipe
        if (current == NULL || token[0] == '|') {
            Command *new_cmd = (Command *)malloc(sizeof(Command));
            if (new_cmd == NULL) {
                free(line_copy);
                return (Command *)-1;
            }
            memset(new_cmd, 0, sizeof(Command)); 

            if (head == NULL) {
                head = new_cmd;
            } else {
                Command *last = head;
                while (last->next != NULL) {
                    last = last->next;
                }
                last->next = new_cmd;
            }
            current = new_cmd;

            if(token[0] == '|') {
                token = strtok_r(NULL, " \t\n", &saveptr);
                if(token == NULL) break; // if there's nothing after |: end parsing
            }
        }

        if (token[0] == '<') {
            // handle redirection
            // printf("redir token: %s\n", token);
            token = strtok_r(NULL, " \t\n", &saveptr);
            if (token == NULL) {
                free(line_copy);
                return (Command *)-1;
            }
            current->redir_in_path = strdup(token);

        } else if (token[0] == '>') {
            // printf("redir token: %s\n", token);
            token = strtok_r(NULL, " \t\n", &saveptr);
            if (token == NULL) {
                free(line_copy);
                return (Command *)-1;
            }
            current->redir_out_path = strdup(token);
            current->redir_append = (strlen(token) > 1 && token[1] == '>'); 

        } else {
            // executable or argument
            if (current->exec_path == NULL) {
                current->exec_path = resolve_executable(token, *variables);
                if (current->exec_path == NULL) {
                    free(line_copy);
                    return (Command *)-1;
                }
                current->args = (char **)malloc(2 * sizeof(char *)); 
                current->args[0] = strdup(token); 
                current->args[1] = NULL;
            } else {
                // token is an argument to the command
                int argc = 0;
                while (current->args[argc] != NULL) argc++; 
                current->args = (char **)realloc(current->args, (argc + 2) * sizeof(char *));
                current->args[argc] = strdup(token);
                current->args[argc + 1] = NULL;
            }
        }

        token = strtok_r(NULL, " \t\n", &saveptr);
    }

    free(line_copy); 
    return head; // return head of the linked list of commands

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

    // Commented so no warnings in starter make
    // Variable *replacements = NULL;
    // Variable **current = &replacements;
    // const char *parse_var_st, *parse_var_end;
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

    // todo: change these values
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
