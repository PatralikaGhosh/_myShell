#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>
#include <ctime>
#include <pwd.h>
#include "Tokenizer.h"

// all the basic colours for a shell prompt
#define RED     "\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE	"\033[1;34m"
#define WHITE	"\033[1;37m"
#define NC      "\033[0m"

using namespace std;

int main () {

    // TODO: create copies of stdin/stdout; dup()
    int original_stdin = dup(STDIN_FILENO);
    int original_stdout = dup(STDOUT_FILENO);
    string input;
    char prevWorkingDir[256];
    

    for (;;) {
        // implement date/time with TODO
        // implement username with getlogin()
        // implement curdir with getcwd()
        // need date/time, username, and absolute path to current dir

        sleep(1);

        // Get the current time
        time_t now = time(0);
        struct tm* localTime = localtime(&now);
        char formattedTime[20]; // Buffer for formatted time (e.g., "Sep 23 18:31:46")

        strftime(formattedTime, sizeof(formattedTime), "%b %d %H:%M:%S", localTime);

        // Get the username
        struct passwd *pw = getpwuid(geteuid());
        const char *username = pw->pw_name;

        // Get the current working directory
        char currentWorkingDir[256];
        if (getcwd(currentWorkingDir, sizeof(currentWorkingDir)) == nullptr) {
            perror("getcwd");
            exit(EXIT_FAILURE);
        }

        // Display the prompt with date, time, username, and working directory
        cout << formattedTime << " " << username << ":" << currentWorkingDir << "$ ";
        
        // get user inputted command
        
        getline(cin, input);

        if (input == "exit") {  // print exit message and break out of infinite loop
            cout << RED << "Now exiting shell..." << endl << "Goodbye" << NC << endl;
            //delete prevWorkingDir;
            break;
        }

        // get tokenized commands from user input
        Tokenizer token(input);
        if (token.hasError()) {  // continue to next prompt if input had an error
            continue;
        }
        //chdir()
        // if dir (cd <dir>) is "-", then go back to previous working directory
        //variable storing previous working directory

            if (token.commands.size() > 0 && token.commands[0]->args[0] == "cd") {
                if (token.commands[0]->args.size() < 2) {
                    // Handle 'cd' without arguments (change to home directory)
                    chdir(getenv("HOME"));
                } else {
                    const string& dirArg = token.commands[0]->args[1];
                    if (dirArg == "-") {
                        // Handle 'cd -' (change to the previous working directory)
                        if (strlen(prevWorkingDir) > 0) {
                            chdir(prevWorkingDir);
                        } else {
                            cerr << "No previous working directory available." << endl;
                        }
                    } else if (dirArg == "..") {
                        // Handle 'cd ..' (move up one level in the directory hierarchy)
                        chdir("..");
                    } else if (dirArg == "../..") {
                        // Handle 'cd ../..' (move up two levels in the directory hierarchy)
                        chdir("../..");
                    } else {
                        // Handle 'cd <directory>' (change to the specified directory)
                        if (getcwd(prevWorkingDir, sizeof(prevWorkingDir)) != NULL) {
                            chdir(dirArg.c_str());
                        } else {
                            cerr << "Error getting current working directory." << endl;
                        }
                    }
                }
                continue; // Skip the fork-execution part for 'cd' command
            }
        
        // // print out every command token-by-token on individual lines
        // // prints to cerr to avoid influencing autograder
        // for (auto cmd : tknr.commands) {
        //     for (auto str : cmd->args) {
        //         cerr << "|" << str << "| ";
        //     }
        //     if (cmd->hasInput()) {
        //         cerr << "in< " << cmd->in_file << " ";
        //     }
        //     if (cmd->hasOutput()) {
        //         cerr << "out> " << cmd->out_file << " ";
        //     }
        //     cerr << endl;
        // }

        int pipefd[2];
        for (size_t i = 0; i < token.commands.size(); i++) {

            if (pipe(pipefd) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }

            // fork to create child
            pid_t child = fork();

            if (child < 0) {  // error check
                perror("fork");
                exit(2);
            }

            if (child == 0) {  // if child
                // Handle input redirection if present
                if (!token.commands[i]->in_file.empty()) {
                    int input_fd = open(token.commands[i]->in_file.c_str(), O_RDONLY);
                    if (input_fd == -1) {
                        perror("open");
                        exit(2);
                    }
                    dup2(input_fd, STDIN_FILENO);
                    close(input_fd);
                }

                // Handle output redirection if present
                if (!token.commands[i]->out_file.empty()) {
                    int output_fd = open(token.commands[i]->out_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
                    if (output_fd == -1) {
                        perror("open");
                        exit(2);
                    }
                    dup2(output_fd, STDOUT_FILENO);
                    close(output_fd);
                }

                if (i < token.commands.size() - 1) {
                    dup2(pipefd[1], STDOUT_FILENO);
                }

                close(pipefd[0]);

                // Execute the command
                vector<char*> args;
                for (const string& arg : token.commands[i]->args) {
                    args.push_back(const_cast<char*>(arg.c_str()));
                }
                args.push_back(nullptr);

                if (execvp(args[0], args.data()) < 0) {
                    perror("execvp");
                    exit(2);
                }
            } else {  // if parent
                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[1]);

                if (i == token.commands.size() - 1) {
                    wait(0);
                }
            }
        }
        //cout << YELLOW << "Shell$" << NC << " ";
        // Restore the stdin and stdout
        dup2(original_stdin, STDIN_FILENO);
        dup2(original_stdout, STDOUT_FILENO);
    }
}