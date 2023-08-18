# The assignments were 4 in total
- `Assignment 2` was based on building a Shell
- `Assignments 3&4` were about building a Multithreaded server & a Load Tester
# Shell Description
- built a simple shell to execute user commands, similar to the bash shell in
Linux.

## PART 1: A Simple C Shell
- A shell takes in user input, forks one or more child processes using the fork system call, calls exec from these children to execute user commands, and reaps the dead children using the wait system call.

- Incorporated support for the **cd** command

## PART 2: Background execution of commands
- Extended shell program to support background execution of commands (followed by &)
- Ensured proper reaping of both foreground and background process

## PART 3: The exit command
- Implementation of **exit** command to terminate the shell from infinite loop and exit.

## PART 4 : Handling the Ctrl+C signal
- Modified the shell with a custom signal handler such that SIGINT does not terminate the shell but only terminates the foreground process it is running.

---

# Multithreaded-Server and Closed Loop testing
- Build a multi-threaded web server. The web server can handle simple HTTP requests and return suitable responses. It uses a master-worker thread pool architecture to handle multiple concurrent clients. Where the master thread accepts new connections, and the HTTP processing is done by the worker threads in the thread pool.

- Built a load generator client to simulate load testing on the web server built earlier.

---
The repo contains all the required code and the reports generated for both these tasks.

