#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h> // Adiciona a biblioteca dirent.h para o macOS

#define LOG_FILE "zombie_log.txt" // Log file path

void handle_signal(int sig) // Signal handler
{
    if (sig == SIGTERM) // If signal is SIGTERM
    {
        FILE *log_file = fopen(LOG_FILE, "a"); // Append to log file
        if (log_file != NULL)                  // If log file was opened successfully
        {
            fprintf(log_file, "Daemon terminated.\n"); // Write to log file
            fclose(log_file);                          // Close log file
        }
        exit(0); // Exit with success
    }
}

void get_parameter(char *parameter, char *token) // Get parameter from line
{
    token = strtok(NULL, ":");
    // Remove leading spaces
    while (*token == ' ' || *token == '\t')
    {
        token++;
    }

    // Remove \n from end of line
    if (token[strlen(token) - 1] == '\n')
    {
        token[strlen(token) - 1] = '\0';
    }
    strcpy(parameter, token);
}

void write_zombies_to_log(FILE *log_file) // Write zombie processes to log file
{

    DIR *proc_dir = opendir("/proc"); // Open /proc directory
    if (proc_dir == NULL)             // If /proc directory was not opened successfully
    {
        perror("Failed to open /proc directory"); // Print error message
        exit(1);                                  // Exit with error
    }
    struct dirent *entry;                       // Directory entry
    while ((entry = readdir(proc_dir)) != NULL) // For each entry in /proc directory
    {
        if (entry->d_type == DT_DIR) // If entry is a directory
        {
            if (atoi(entry->d_name) != 0) // If entry name is a valid PID
            {

                char proc_path[512];                                                      // Path to process status file
                snprintf(proc_path, sizeof(proc_path), "/proc/%s/status", entry->d_name); // Create path to process status file
                FILE *status_file = fopen(proc_path, "r");                                // Open process status file

                if (status_file == NULL) // If process status file was not opened successfully
                {
                    perror("Failed to open process status file");              // Print error message
                    fprintf(log_file, "Failed to open process status file\n"); // Write error message to log file
                    exit(1);                                                   // Exit with error
                }

                if (status_file != NULL) // If process status file was opened successfully
                {
                    char pid[256], ppid[256]; // Process ID and parent process ID
                    char name[256];           // Process name
                    char line[256];           // Line from process status file
                    char status[256];         // Process status

                    int isZombie = 0; // Is process a zombie?

                    while (fgets(line, sizeof(line), status_file) != NULL) // For each line in process status file
                    {
                        // split line in ':'
                        char *token = strtok(line, ":");

                        // Check if token is state
                        if (strcmp(token, "State") == 0)
                        {
                            get_parameter(status, token);

                            // fprintf(log_file, "Status: %s\n", status);

                            for (int i = 0; status[i] != '\0'; i++)
                            {
                                // Verifica se o caractere é 'Z'
                                if (status[i] == 'Z')
                                {
                                    isZombie = 1;
                                    break; // Se encontrou 'Z', sai do loop
                                }
                            }
                        }

                        // Check if token is name
                        if (strcmp(token, "Name") == 0)
                        {
                            // fprintf(log_file, "Name: %s\n", name);
                            get_parameter(name, token);
                        }

                        // Check if token is pid
                        if (strcmp(token, "Pid") == 0)
                        {
                            // fprintf(log_file, "Pid: %s\n", pid);
                            get_parameter(pid, token);
                        }

                        // Check if token is ppid
                        if (strcmp(token, "PPid") == 0)
                        {
                            // fprintf(log_file, "PPid: %s\n", ppid);
                            get_parameter(ppid, token);
                        }
                    }

                    if (isZombie != 0) // If process is a zombie
                    {
                        fprintf(log_file, "%9s     | %8s     |     %s\n", pid, ppid, name); // Write process information to log file
                    }

                    fclose(status_file); // Close process status file
                }
            }
        }
    }
    closedir(proc_dir); // Close /proc directory
}

int main(int argc, char **argv) // Main function
{
    if (argc != 2) // If number of arguments is not 2
    {
        fprintf(stderr, "Usage: %s <interval>\n", argv[0]); // Print usage message
        return 1;                                           // Exit with error
    }

    int interval = atoi(argv[1]); // Interval between log writes
    if (interval <= 0)            // If interval is not positive
    {
        fprintf(stderr, "Interval must be a positive integer\n"); // Print error message
        return 1;                                                 // Exit with error
    }

    pid_t pid = fork(); // Daemonize (Fork) the process
    if (pid < 0)        // If fork failed
    {
        perror("Failed to daemonize"); // Print error message
        return 1;                      // Exit with error
    }
    else if (pid > 0) // If parent process
    {
        exit(0); // Exit with success
    }

    setsid(); // Create new session

    signal(SIGTERM, handle_signal); // Register signal handler

    //umask(0); // Set file mode creation mask to 0

    //close(STDIN_FILENO);  // Close standard input
    //close(STDOUT_FILENO); // Close standard output
    //close(STDERR_FILENO); // Close standard error

    FILE *log_file = fopen(LOG_FILE, "w"); // Write to log file
    if (log_file == NULL)                  // If log file was not opened successfully
    {
        perror("Failed to open log file"); // Print error message
        exit(1);                           // Exit with error
    }

    fprintf(log_file, "      PID     |     PPID     |     Nome do Programa\n"); // Write header to log file

    // Main loop
    while (1)
    {
        fprintf(log_file, "========================================================\n"); // Write header to log file
        write_zombies_to_log(log_file);                                    // Write zombie processes to log file
        fflush(log_file);                                                  // Flush buffer to ensure data is written immediately
        sleep(interval);                                                   // Sleep for interval seconds
    }

    fclose(log_file); // Close log file
    return 0;         // Exit with success
}
