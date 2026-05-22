#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int is_builtin(const char *cmd) {
    char *validCommands[] = {"exit", "echo", "type"};
    int size = sizeof(validCommands) / sizeof(validCommands[0]);
    for (int i = 0; i < size; i++) {
        if (strcmp(cmd, validCommands[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);
    while(1) {
        printf("$ ");
        char input[100];
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;
        
        if (strcmp(input, "exit") == 0) {
            break;
        } else if (strncmp(input, "echo", 4) == 0) {
            printf("%s\n", input + 5);
        } else if (strncmp(input, "type", 4) == 0) {
            char *arg = input + 5;
            if (is_builtin(arg)) {
                printf("%s is a shell builtin\n", arg);
            } else {
                char *path_env = getenv("PATH");
                char *path_copy = strdup(path_env);

                if (path_copy != NULL) {
                    char *dir = strtok(path_copy, ":");
                    char full_path[1024];

		    int found = 0;
                    while (dir != NULL) {
                        snprintf(full_path, sizeof(full_path), "%s/%s", dir, arg);

                        if (access(full_path, F_OK) == 0) {
                          if (access(full_path, X_OK) == 0) {
			    printf("%s is %s\n", arg, full_path);          
			     found = 1;
			     break;
			   }
                         }
                        dir = strtok(NULL, ":");
                    }
		    if (!found) {
                        printf("%s: not found\n", arg);
                    }
                    free(path_copy); 
                } else {
                    printf("%s: not found\n", arg);
                }
            } 
        } else {
	  char *args[64];
	  int arg_count = 0;
	  
	  char *arg_token = strtok(input, " \t");
	  
	  while (arg_token != NULL && arg_count < 63) {
	    args[arg_count++] = arg_token;
	    arg_token = strtok(NULL, " \t");
	  }

          args[arg_count] = NULL;

          if (arg_count == 0) continue;

          char *path_env = getenv("PATH");
	  char *path_copy = strdup(path_env);
	  char full_path[1024];
	  int found = 0;
	  
	  if (path_copy) {
	    char *dir = strtok(path_copy, ":");
	    while (dir != NULL) {
	      snprintf(full_path, sizeof(full_path), "%s/%s", dir, args[0]);
	      if (access(full_path, F_OK) == 0 && access(full_path, X_OK) == 0) {
		found = 1;
		break;
	      }
	      dir = strtok(NULL, ":");
	    }
	    free(path_copy);
          }
	  
	  if (!found) {
	    printf("%s: command not found\n", args[0]);
	    continue;
	  }

          pid_t p = fork();
	  if(p < 0) {
	    perror("fork fail");
          } else if (p == 0) {
	    execvp(args[0], args);
          } else {
	    int status;
	    waitpid(p, &status, 0);
          }
	  
	}
    }
    return 0;
}
