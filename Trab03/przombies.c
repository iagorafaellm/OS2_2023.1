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
        if (log_file != NULL) // If log file was opened successfully
        {
            fprintf(log_file, "Daemon terminated.\n"); // Write to log file
            fclose(log_file); // Close log file
        }
        exit(0); // Exit with success
    }
}

void write_zombies_to_log() // Write zombie processes to log file
{
    FILE *log_file = fopen(LOG_FILE, "a"); // Append to log file
    if (log_file == NULL) // If log file was not opened successfully
    {
        perror("Failed to open log file"); // Print error message
        exit(1); // Exit with error
    }

    fprintf(log_file, "PID | PPID | Nome do Programa\n"); // Write header to log file
    fprintf(log_file, "==========================================\n"); // Write header to log file

    DIR *proc_dir = opendir("/proc"); // Open /proc directory
    if (proc_dir == NULL) // If /proc directory was not opened successfully
    {
        perror("Failed to open /proc directory"); // Print error message
        exit(1); // Exit with error
    }

    struct dirent *entry; // Directory entry
    while ((entry = readdir(proc_dir)) != NULL) // For each entry in /proc directory
    {
        if (entry->d_type == DT_DIR) // If entry is a directory
        {
            if (atoi(entry->d_name) != 0) // If entry name is a valid PID
            {
                char proc_path[256]; // Path to process status file
                snprintf(proc_path, sizeof(proc_path), "/proc/%s/status", entry->d_name); // Create path to process status file
                FILE *status_file = fopen(proc_path, "r"); // Open process status file
                if (status_file != NULL) // If process status file was opened successfully
                {
                    int pid, ppid; // Process ID and parent process ID
                    char name[256]; // Process name
                    char line[256]; // Line from process status file

                    while (fgets(line, sizeof(line), status_file) != NULL) // For each line in process status file
                    {
                        if (sscanf(line, "Pid:\t%d", &pid) == 1) // If line starts with "Pid:"
                        {
                            break; // Stop reading lines
                        }
                    }

                    rewind(status_file); // Rewind process status file

                    while (fgets(line, sizeof(line), status_file) != NULL) // For each line in process status file
                    {
                        if (sscanf(line, "PPid:\t%d", &ppid) == 1) // If line starts with "PPid:"
                        {
                            break; // Stop reading lines
                        }
                    }

                    rewind(status_file); // Rewind process status file

                    while (fgets(line, sizeof(line), status_file) != NULL) // For each line in process status file
                    {
                        if (sscanf(line, "Name:\t%s", name) == 1) // If line starts with "Name:"
                        {
                            break; // Stop reading lines
                        }
                    }

                    if (ppid == getpid()) // If process is a zombie
                    {
                        fprintf(log_file, "%d | %d | %s\n", pid, ppid, name); // Write process information to log file
                    }

                    fclose(status_file); // Close process status file
                }
            }
        }
    }

    closedir(proc_dir); // Close /proc directory
    fclose(log_file); // Close log file
}

int main(int argc, char **argv) // Main function
{
    if (argc != 2) // If number of arguments is not 2
    {
        fprintf(stderr, "Usage: %s <interval>\n", argv[0]); // Print usage message
        return 1; // Exit with error
    }

    int interval = atoi(argv[1]); // Interval between log writes
    if (interval <= 0) // If interval is not positive
    {
        fprintf(stderr, "Interval must be a positive integer\n"); // Print error message
        return 1; // Exit with error
    }

    pid_t pid = fork(); // Daemonize (Fork) the process
    if (pid < 0) // If fork failed
    {
        perror("Failed to daemonize"); // Print error message
        return 1; // Exit with error
    }
    else if (pid > 0) // If parent process
    {
        exit(0); // Exit with success
    }

    setsid(); // Create new session

    signal(SIGTERM, handle_signal); // Register signal handler

    umask(0); // Set file mode creation mask to 0

    close(STDIN_FILENO); // Close standard input
    close(STDOUT_FILENO); // Close standard output
    close(STDERR_FILENO); // Close standard error

    // Main loop
    while (1)
    {
        write_zombies_to_log(); // Write zombie processes to log file
        sleep(interval); // Sleep for interval seconds
    }

    return 0; // Exit with success
}
