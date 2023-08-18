#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
// To store the PID of shell process
int main_pid;
/* Splits the string by space and returns the array of tokens
 *
 */
char **tokenize(char *line)
{
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	int i, tokenIndex = 0, tokenNo = 0;

	for (i = 0; i < strlen(line); i++)
	{

		char readChar = line[i];

		if (readChar == ' ' || readChar == '\n' || readChar == '\t')
		{
			token[tokenIndex] = '\0';
			if (tokenIndex != 0)
			{
				tokens[tokenNo] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0;
			}
		}
		else
		{
			token[tokenIndex++] = readChar;
		}
	}

	free(token);
	tokens[tokenNo] = NULL;
	return tokens;
}

int main(int argc, char *argv[])
{	
	char line[MAX_INPUT_SIZE];
	char **tokens;
	int i;
	
	main_pid = getpid();
	//Ignore SIGINT by default
	// We only let the FG process to use default behaviour of SIGINT
	signal(SIGINT, SIG_IGN);
	while (1)
	{
		//Reap background processes if any
			int status;
			int j;
			while(1){
				j = waitpid(-1, &status, WNOHANG);
			if(j > 0)
				printf("Shell: Background process finished\n");
			else
				break;
			}
			
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		printf("$ ");
		scanf("%[^\n]", line);
		getchar();
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; // terminate with new line
		tokens = tokenize(line);

		// Tokens are ready for use
		
		// Find the number of tokens available to us
		int token_size = 0;
		char **token_iter = tokens;
		while (*token_iter != NULL)
		{
			token_size++;
			token_iter++;
		}
		// Count of tokens available now

		// When receiving empty ENTERS ignore them
		if (tokens[0] == NULL)
		{
			// Freeing the allocated memory
			for (i = 0; tokens[i] != NULL; i++)
			{
				free(tokens[i]);
			}
			free(tokens);
			continue;
		}
		// If the user prompts cd command, verify it is cd and then run chdir
		if (strcmp(tokens[0], "cd") == 0)
		{
			int cd_status;
			if(token_size != 2){
				printf("Shell: Incorrect CD command\n");
				// Freeing the allocated memory
				for (i = 0; tokens[i] != NULL; i++)
				{
					free(tokens[i]);
				}
				free(tokens);
				continue;
			}
			cd_status = chdir(tokens[1]);
			if (cd_status == -1)
			{
				printf("Shell: Incorrect command\n");
			}
			// Freeing the allocated memory
			for (i = 0; tokens[i] != NULL; i++)
			{
				free(tokens[i]);
			}
			free(tokens);
			continue;
		}
		// Implementing the EXIT command
		// Shell and CHILD processes have main_pid as PGID
		// send SIGQUIT to main_pid process group
		// except parent everyone will be killed
		// Parent reaps the children
		// parent exits 
		if (strcmp(tokens[0] , "exit") == 0){
			// Freeing the allocated memory
			for (i = 0; tokens[i] != NULL; i++)
			{
				free(tokens[i]);
			}
			free(tokens);
			//Only parent ignores SIGQUIT every other process in PG doesn't
			signal(SIGQUIT, SIG_IGN);
			kill(0, SIGQUIT);

			//Reap background processes if any
			int status;
			int j;
			while(1){
				j = waitpid(-1, &status, 0);
				if(j <= 0)
					break;
			}
			// Now Shell exits
			exit(0);

		}
		// Upto here these doesn't matter for BG processes.
		// check if BG process or not
		if (strcmp(tokens[token_size-1],"&") != 0){
			// Foreground Execution
			// fork-exec-wait mechanism
			int pid;
			pid = fork();
			if (pid > 0)
			{
				int j = waitpid(pid, &status, 0);
			}
			else if (pid == 0)
			{
				// FG process will respond to SIGINT
				signal(SIGINT, SIG_DFL);

				if(execvp((const char *)tokens[0], tokens) == -1)
					printf("Shell: Incorrect command\n");
				
				exit(0);
			}
			else
			{
				perror("fork");
			}
		}

		// Background process criteria runs here

		else{
			tokens[token_size-1] = NULL;
			int pid;
			pid = fork();
			if (pid == 0) {
				if(execvp((const char *)tokens[0], tokens) == -1)
					printf("Shell: Incorrect command\n");
				exit(0);
			}
			else if(pid < 0)
			{
				perror("fork");
			}
		}

		// Freeing the allocated memory
		for (i = 0; tokens[i] != NULL; i++)
		{
			free(tokens[i]);
		}
		free(tokens);
		// Shell loop ends here
	}
	return 0;
}
