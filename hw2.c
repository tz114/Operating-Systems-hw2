#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
int size;
int a1s;

char *read_line(){
    char *line=NULL;
    size_t buffer=0;
    getline(&line,&buffer,stdin);
    return line;
    
}
char **split(char *line){
    char delim[]=" \n";
    int i=0;
    char **args=malloc(64*sizeof(char*));
    char *ptr = strtok(line,delim);
    while(ptr!=NULL){
        
        args[i]=ptr;
        i++;
        ptr=strtok(NULL,delim);
        
    }
    free(ptr);
    size=i;
    args[i]=NULL;
    return args;

}
int cd(char **args){
    
    if(args[1]==NULL){
        chdir(getenv("HOME"));
    }
    else{
        if (chdir(args[1]) != 0) {
            perror("cd");
        }
    }
    return 1;
}
int exit_(){
    return 0;
}

int command(char **args){
    pid_t pid;
    pid_t p2;
    int status;
    char delim[]="#";
    int background=0;
    char path[PATH_MAX];
    int pipes=0;
   
    int j;
    if(!strcmp(args[size-1],"&")){
        background=1;
    }
    char **args1=malloc(64*sizeof(char*));
    if(a1s){
        for(int i=0;i<a1s;i++){
            args1[i]=NULL;
        }
    }
        
    char **args2=malloc(64*sizeof(char*));
    for(int i=0;i<size;i++){
        if(pipes==1 && strcmp(args[i],"&")){
            args2[j]=args[i];
            j++;
        }
        else{
            if(strcmp(args[i],"|") && strcmp(args[i],"&")){
                args1[i]=args[i];
            }
        }
        if(!strcmp(args[i],"|")){
       
            pipes=1;
            a1s=i;
            j=0;
        }
    }

    if(pipes==0){
        args1[size]=NULL;
    }
    if(pipes==1){
        args2[j]=NULL;
        args1[a1s]=NULL;
    }
    
    if(getenv("MYPATH")){
        strcpy(path,getenv("MYPATH"));
    }
    else{
        strcpy(path,"/bin#.");
    }
    char *token;
    struct stat buffer;

    token=strtok(path,delim);
    
    int exist=0;
    int exist2=0;

    int fd[2];
    if(pipe(fd)<0){
        fprintf(stderr,"pipe fail");
    }
    
    while(token!=NULL){
        
        char compath[PATH_MAX];
        strcpy(compath,token);
        strcat(compath,"/");
        strcat(compath,args1[0]);

        if(lstat(compath, &buffer)==0){
            exist=1;
            pid = fork();

            if (pid == 0) {
                if(pipes==1){
                    close(fd[0]);
                    dup2(fd[1],1);
                    
                }
     
                execv(compath,args1);
                free(args1);
                free(args);
                exit(1);
                
                
            }
             else {
                if(background==1){
                        printf("[running background process \"%s\"]\n",args1[0]);
                    }
          
                                        
                    if(background==0){
                        waitpid(pid, &status, 0);

                    }
                    if(background==1 && WIFEXITED(status)){
                        int exit_status=WEXITSTATUS(status);
                        printf("[process %d terminated with exit status %d]\n",pid,exit_status);
                    }
                if(pipes==1){
       
                    if(getenv("MYPATH")){
                            strcpy(path,getenv("MYPATH"));
                        }
                        else{
                            strcpy(path,"/bin#.");
                        }
       
                    token=strtok(path,delim);
                    while(token!=NULL){
                        char compath2[PATH_MAX];
                        strcpy(compath2,token);
                        strcat(compath2,"/");
                        strcat(compath2,args2[0]);
                        
                        if(lstat(compath2,&buffer)==0){
                            exist2=1;
                            p2=fork();
                            if(p2==0){
                                close(fd[1]);
                                dup2(fd[0],0);
                                
                                execv(compath2,args2);
                                exit(1);
                            }
                            else{
                                close(fd[0]);
                                close(fd[1]);
                                if(background==1){
                                    printf("[running background process \"%s\"]\n",args2[0]);
                                }
          
                                                    
                                if(background==0){
                                    waitpid(p2,&status,0);
                                }
                                if(background==1 && WIFEXITED(status)){
                                    int exit_status=WEXITSTATUS(status);
                                    printf("[process %d terminated with exit status %d]\n",p2,exit_status);
                                }
                                
                            }
                        }

                        token=strtok(NULL,delim);

                    }
                }
                    
                
                close(fd[0]);
                close(fd[1]);
            }
            
            
        }

        token=strtok(NULL,delim);

                
    }
    
  
    if(exist==0){
        fprintf(stderr,"ERROR: command \"%s\" not found\n",args1[0]);
    }
    if(exist2==0 && pipes==1){
        fprintf(stderr,"ERROR: command \"%s\" not found\n",args2[0]);
    }
    free(args1);
    free(args2);
    return 1;
}  
int execute(char **args){
    char *special[]={"cd","exit"};
    
    for(int i=0;i<2;i++){
        if(strcmp(args[0],special[i])==0){
            
            if(i==0){
                return cd(args);
            }
            else{
                
                return exit_();
            }
        }
    }
    return command(args);
}
  

void loop(){
    char *line;
    char **args;
    int status;
    pid_t pid;
    int loop=1;

      do {
        char cwd[PATH_MAX];
        getcwd(cwd,sizeof(cwd));
        pid = waitpid(-1, &status, WNOHANG | WUNTRACED);
        if(pid > 0){
            if(WIFEXITED(status)){
                printf("[process %d terminated with exit status %d]\n", pid, WEXITSTATUS(status));
            }
        }
        printf("%s$ ",cwd);
        line=read_line();
        args=split(line);
        loop=execute(args);
        free(line);
        free(args);
      } while (loop);
    
    printf("bye\n");
}


int main(int argc, char **argv){
  setvbuf( stdout, NULL, _IONBF, 0 );
  loop();
  return EXIT_SUCCESS;
}
