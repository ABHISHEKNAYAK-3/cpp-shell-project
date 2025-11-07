# C++ Core Shell Project

A custom C++ shell for Linux, built as a B.Tech 4th-year project. This shell implements core OS concepts like process creation, execution, and inter-process communication.

---

## 🚀 Core Features

This shell successfully implements:
* **Command Parsing:** Reads and parses user input into arguments.
* **Command Execution:** Uses `fork()` and `execvp()` to run any standard Linux command.
* **Wait & Background:** Correctly waits for foreground commands with `waitpid()` and allows background tasks using `&`.
* **Built-in Commands:** Supports `exit` and `cd` directly within the shell.
* **I/O Redirection:** Redirects command output to a file using `>` (e.g., `ls -l > out.txt`).
* **Piping:** Chains two commands together using `|` (e.g., `ls -l | grep .cpp`).

---

## 🛠️ How to Build and Run

This project is built to run on a Linux environment (like Ubuntu via WSL).

1.  **Clone the repository (or download the file):**
    ```bash
    git clone [https://github.com/ABHISHEKNAYAK-3/cpp-shell-project.git](https://github.com/ABHISHEKNAYAK-3/cpp-shell-project.git)
    cd cpp-shell-project
    ```

2.  **Compile the C++ code:**
    ```bash
    g++ -o myshell myshell.cpp
    ```

3.  **Run the shell:**
    ```bash
    ./myshell
    ```
    You will see the `vintage-shell>` prompt.

---

## 📖 Usage Examples

Here are some commands you can try in the shell:

```bash
# Run a basic command
vintage-shell> ls -l

# Run a background process
vintage-shell> sleep 5 &

# Redirect output to a file
vintage-shell> echo "Hello World" > hello.txt
vintage-shell> cat hello.txt

# Use a pipe
vintage-shell> ls -l | grep .cpp
