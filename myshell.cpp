#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

// --- PARSING FUNCTIONS (FROM DAY 4) ---
void parseRedirect(std::vector<std::string>& args, std::string& outputFile) {
    outputFile = "";
    std::vector<std::string> new_args;
    bool foundRedirect = false;

    for (const auto& arg : args) {
        if (arg == ">") {
            foundRedirect = true;
        } else if (foundRedirect) {
            outputFile = arg;
            foundRedirect = false;
        } else {
            new_args.push_back(arg);
        }
    }
    args = new_args;
}

// --- NEW: Helper function to split commands by pipe ---
std::vector<std::string> splitCommand(std::vector<std::string>& args, int& pipeIndex) {
    pipeIndex = -1;
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "|") {
            pipeIndex = i;
            break;
        }
    }

    if (pipeIndex == -1) {
        return {}; // No pipe found
    }

    // Split args into 'command1' and 'command2'
    std::vector<std::string> command2;
    for (size_t i = pipeIndex + 1; i < args.size(); ++i) {
        command2.push_back(args[i]);
    }
    args.erase(args.begin() + pipeIndex, args.end()); // 'args' is now 'command1'
    return command2;
}

// --- NEW: Helper function to convert vector to char* array ---
std::vector<char*> get_argv(std::vector<std::string>& args) {
    std::vector<char*> argv;
    for (const auto& arg : args) {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    argv.push_back(nullptr);
    return argv;
}

// --- MODIFIED: Execute function to handle one command (with redirect) ---
void executeSimpleCommand(std::vector<std::string>& args, bool runInBackground) {
    std::string outputFile;
    parseRedirect(args, outputFile); // Check for '>'

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) { // --- CHILD ---
        if (!outputFile.empty()) {
            int fd = open(outputFile.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0644);
            if (fd < 0) { perror("open file"); exit(1); }
            if (dup2(fd, STDOUT_FILENO) < 0) { perror("dup2"); exit(1); }
            close(fd);
        }
        
        std::vector<char*> argv = get_argv(args);
        if (execvp(argv[0], argv.data()) == -1) {
            perror("execvp");
            exit(1);
        }
    } else { // --- PARENT ---
        if (!runInBackground) {
            int status;
            waitpid(pid, &status, 0);
        } else {
            std::cout << "[Started background job " << pid << "]\n";
        }
    }
}

// --- NEW: Execute function for piped commands ---
void executePipedCommand(std::vector<std::string>& cmd1, std::vector<std::string>& cmd2) {
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        perror("pipe");
        return;
    }

    pid_t pid1 = fork();
    if (pid1 < 0) { perror("fork"); return; }

    if (pid1 == 0) { // --- CHILD 1 (runs cmd1) ---
        close(pipefd[0]); // Close read end
        dup2(pipefd[1], STDOUT_FILENO); // Output goes to pipe
        close(pipefd[1]); // Close original write end

        std::vector<char*> argv = get_argv(cmd1);
        if (execvp(argv[0], argv.data()) == -1) {
            perror("execvp cmd1");
            exit(1);
        }
    }

    pid_t pid2 = fork();
    if (pid2 < 0) { perror("fork"); return; }

    if (pid2 == 0) { // --- CHILD 2 (runs cmd2) ---
        close(pipefd[1]); // Close write end
        dup2(pipefd[0], STDIN_FILENO); // Input comes from pipe
        close(pipefd[0]); // Close original read end

        std::vector<char*> argv = get_argv(cmd2);
        if (execvp(argv[0], argv.data()) == -1) {
            perror("execvp cmd2");
            exit(1);
        }
    }

    // --- PARENT ---
    // Parent MUST close both ends of the pipe
    close(pipefd[0]);
    close(pipefd[1]);
    
    // Wait for both children
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

// --- MAIN EXECUTE FUNCTION ---
void executeCommand(std::vector<std::string>& args) {
    if (args.empty()) return;

    if (args[0] == "exit") exit(0);
    if (args[0] == "cd") {
        if (args.size() < 2) {
            std::cerr << "cd: expected argument\n";
        } else if (chdir(args[1].c_str()) != 0) {
            perror("chdir");
        }
        return;
    }

    bool runInBackground = (!args.empty() && args.back() == "&");
    if (runInBackground) args.pop_back();

    int pipeIndex;
    std::vector<std::string> command2 = splitCommand(args, pipeIndex);

    if (pipeIndex != -1) {
        executePipedCommand(args, command2); // args is now command1
    } else {
        executeSimpleCommand(args, runInBackground);
    }
}

// --- Main loop is unchanged ---
int main() {
    std::string userInput;
    while (true) {
        std::cout << "vintage-shell> ";
        std::cout.flush();

        if (!std::getline(std::cin, userInput)) break;

        std::stringstream ss(userInput);
        std::string token;
        std::vector<std::string> args;
        while (ss >> token) {
            args.push_back(token);
        }
        
        executeCommand(args);
    }

    std::cout << "\nExiting shell." << std::endl;
    return 0;
}
