#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define MAX_CMD_LEN 1024

int main(int argc, char *argv[])
{
    FILE *input = stdin;
    int interactive_mode = 0;
    int last_success = 1;

    // Handle command line arguments
    if (argc > 2)
    {
        fprintf(stderr, "Usage: %s [script_file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (argc == 2)
    {
        input = fopen(argv[1], "r");
        if (!input)
        {
            perror(argv[1]);
            exit(EXIT_FAILURE);
        }
        interactive_mode = 0; // File input is never interactive
    }
    else
    {
        // Check if stdin is connected to a terminal
        interactive_mode = isatty(STDIN_FILENO);
    }

    // Print welcome message in interactive mode
    if (interactive_mode)
    {
        printf("Welcome to my shell!\n");
    }

    // Main shell loop
    char command[MAX_CMD_LEN];
    static char read_buffer[MAX_CMD_LEN];
    static int buffer_pos = 0;
    static int buffer_len = 0;
    int input_fd = fileno(input);

    while (1)
    {
        // Print prompt in interactive mode
        if (interactive_mode)
        {
            printf("mysh> ");
            fflush(stdout);
        }

        // Read command using read() as required
        int cmd_pos = 0;
        int found_newline = 0;

        while (!found_newline && cmd_pos < MAX_CMD_LEN - 1)
        {
            // If buffer is empty, read more data
            if (buffer_pos >= buffer_len)
            {
                ssize_t bytes_read = read(input_fd, read_buffer, sizeof(read_buffer));
                if (bytes_read <= 0)
                {
                    // EOF or error
                    if (cmd_pos == 0)
                    {
                        // No partial command, exit loop
                        found_newline = -1; // Signal EOF
                        break;
                    }
                    // We have a partial command without newline, treat as complete
                    break;
                }
                buffer_len = bytes_read;
                buffer_pos = 0;
            }

            // Process one character from buffer
            char c = read_buffer[buffer_pos++];

            if (c == '\n')
            {
                found_newline = 1;
                break;
            }
            // if comment detected set char to terminator to ignore everything after the comment
            if(c == '#'){
                c = '\0';
            }

            command[cmd_pos++] = c;
        }

        // Check for EOF
        if (found_newline == -1)
        {
            break;
        }
        

        // Null-terminate the command
        command[cmd_pos] = '\0';

        // Skip empty commands
        if (strlen(command) == 0)
        {
            continue;
        }

        // Handle built-in commands
        // Parse command into tokens for argument handling
        char *tokens[MAX_CMD_LEN];
        int token_count = 0;
        char cmd_copy[MAX_CMD_LEN];
        strcpy(cmd_copy, command);

        char *token = strtok(cmd_copy, " \t");
        while (token != NULL && token_count < MAX_CMD_LEN - 1)
        {
            tokens[token_count++] = token;
            token = strtok(NULL, " \t");
        }
        tokens[token_count] = NULL;

        if (token_count == 0)
        {
            continue;
        }

        // Detect leading and/or
        int cmdIsAnd = 0;
        int cmdIsOr = 0;

        if(strcmp(tokens[0], "and")     == 0){cmdIsAnd = 1;}
        else if(strcmp(tokens[0], "or") == 0){cmdIsOr = 1;}
        
        // shift tokens if there is a leading and/or
        int startIndex = 0;
        if(cmdIsAnd || cmdIsOr){
            startIndex = 1;
            for(int i = 1 ; i <= token_count ; i++){
                tokens[i - 1] = tokens[i];
            }
            token_count--;

            // if there are no tokens after shift just skip line
            if(token_count == 0){
                continue;
            }
            
        }

        if (strcmp(tokens[0], "pwd") == 0)
        {
            char cwd[MAX_CMD_LEN];
            if (getcwd(cwd, sizeof(cwd)) != NULL)
            {
                printf("%s\n", cwd);
            }
            else
            {
                perror("pwd");
            }
        }
        else if (strcmp(tokens[0], "cd") == 0)
        {
            if (token_count != 2)
            {
                fprintf(stderr, "cd: wrong number of arguments\n");
            }
            else
            {
                if (chdir(tokens[1]) != 0)
                {
                    perror("cd");
                }
            }
        }
        else if (strcmp(tokens[0], "exit") == 0)
        {
            break;
        }
        else
        {
            // TODO: Parse and execute external commands
            printf("You entered: %s\n", command);
        }
    }

    // Print goodbye message in interactive mode
    if (interactive_mode)
    {
        printf("Exiting my shell.\n");
    }

    // Close file if we opened one
    if (input != stdin)
    {
        fclose(input);
    }

    return EXIT_SUCCESS;
}