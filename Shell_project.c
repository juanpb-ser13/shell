/**
UNIX Shell Project

Sistemas Operativos
Grados I. Informatica, Computadores & Software
Dept. Arquitectura de Computadores - UMA

Some code adapted from "Fundamentos de Sistemas Operativos", Silberschatz et al.

To compile and run the program:
   $ gcc Shell_project.c job_control.c -o Shell
   $ ./Shell
	(then type ^D to exit program)

**/

#include "job_control.h"   // remember to compile with module job_control.c
#include <string.h>
#define MAX_LINE 256 /* 256 chars per line, per command, should be enough. */
Listatrabajos listaprocesos;
// -----------------------------------------------------------------------
//                            Manejador
// -----------------------------------------------------------------------
//CAMBIAR COSAS
void manejador(int numero){
	Listatrabajos k=listaprocesos;
	int status;
	int info;
	enum status status_res;
	pid_t hijo;
	while(k!=NULL){
		//algo con cada proceso
		hijo=waitpid(k->pgid,&status, WNOHANG|WUNTRACED);
		status_res=analyze_status(status,&info);
		if(hijo!=0&&status_res!=0&&k->state==BACKGROUND){
			delete_job(&listaprocesos,hijo);
		}else if(hijo!=0&&k->state==BACKGROUND){
			k->state=STOPPED;
		}else if(status_res==0){
			delete_job(&listaprocesos,hijo);
		}
		k=k->next;
	}
}
// -----------------------------------------------------------------------
//                            Funciones internas
// -----------------------------------------------------------------------
void cd(char* args[]){
	int p=0;
	if(args[1]==NULL){
		p=chdir(getenv("HOME"));
	}else{
		p=chdir(args[1]);
	}
	if(p!=0){
		printf("Error, directory not found: %s \n", args[1]);
	}
}
void jobs(){
	block_SIGCHLD();
	show(listaprocesos);
	unblock_SIGCHLD();
}
void fg(char* args[]){
	int status;
	enum status status_res;
	int info;
	if(args[1]==NULL){
		if(listaprocesos!=NULL){
			killpg(listaprocesos->pgid,SIGCONT);
			pid_t p=listaprocesos->pgid;
			char*  l=(char*)malloc(sizeof(strlen(listaprocesos->command)));
			strcpy(l,listaprocesos->command);
			delete_job(&listaprocesos,listaprocesos->pgid);
			set_terminal(p);
			waitpid(p, &status, WUNTRACED);
			set_terminal(getpid());
			status_res=analyze_status(status, &info);
			if(info==255){
				printf("Foreground pid: %d, command: %s, %s, info: %d\n", p, l, "Error", info);
			}else {
				if(status_res==0){
					insert(&listaprocesos,p,l,STOPPED);
				}
					printf("Foreground pid: %d, command: %s, %s, info: %d\n", p, l, status_strings[status_res], info);
			}
		}else{
					printf("Error no existen procesos en background o suspendidos");
		}
	}else{
		int i=0;
		i=atoi(args[1]);
		if(i!=0){
			Listatrabajos j=listaprocesos;
			i--;
			while(j!=NULL&&i>0){
				j=j->next;
				i--;
			}
			if(j!=NULL){
				killpg(j->pgid,SIGCONT);
				pid_t p=j->pgid;
				char*  l=(char*)malloc(sizeof(strlen(j->command)));
				strcpy(l,j->command);
				delete_job(&listaprocesos,j->pgid);
				set_terminal(p);
				waitpid(p, &status, WUNTRACED);
				set_terminal(getpid());
				status_res=analyze_status(status, &info);
				if(info==255){
					printf("Foreground pid: %d, command: %s, %s, info: %d\n", p, l, "Error", info);
				}else {
					if(status_res==0){
						insert(&listaprocesos,p,l,STOPPED);
					}
						printf("Foreground pid: %d, command: %s, %s, info: %d\n",p, l, status_strings[status_res], info);
				}
			}else{
				printf("Error no existe ese proceso");
			}
		}else{
				printf("Error no existe ese proceso");
		}
	}
}
void bg(char* args[]){
	if(args[1]==NULL){
		if(listaprocesos!=NULL){
			killpg(listaprocesos->pgid,SIGCONT);
			listaprocesos->state=BACKGROUND;
			printf("Background job running... pid: %d, command: %s\n", listaprocesos->pgid, listaprocesos->command);
		}else{
					printf("Error no existen procesos en background o suspendidos");
		}
	}else{
		int i=0;
		i=atoi(args[1]);
		if(i!=0){
			Listatrabajos j=listaprocesos;
			i--;
			while(j!=NULL&&i>0){
				j=j->next;
				i--;
			}
			if(j!=NULL){
				killpg(j->pgid,SIGCONT);
				j->state=BACKGROUND;
				printf("Background job running... pid: %d, command: %s\n", j->pgid, j->command);
			}else{
				printf("Error no existe ese proceso");
			}
		}else{
				printf("Error no existe ese proceso");
		}
	}
}
// -----------------------------------------------------------------------
//                            MAIN
// -----------------------------------------------------------------------

int main(void){
	char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
	int background;             /* equals 1 if a command is followed by '&' */
	char *args[MAX_LINE/2];     /* command line (of 256) has max of 128 arguments */
	// probably useful variables:
	int pid_fork, pid_wait; /* pid for created and waited process */
	int status;             /* status returned by wait */
	enum status status_res; /* status processed by analyze_status() */
	int info;				/* info processed by analyze_status() */
	char buffer[512];
	//crear funciones
	crear(&listaprocesos);
	ignore_terminal_signals();
	signal(SIGCHLD,manejador);
	while (1)   /* Program terminates normally inside get_command() after ^D is typed*/
	{
		printf("%s:", getcwd(buffer,512));
		fflush(stdout);
		get_command(inputBuffer, MAX_LINE, args, &background);  /* get next command */

		if(args[0]==NULL) continue;   // if empty command

		if(0==strcmp(args[0],"cd")){
				cd(args);
				continue;
			}

			if(0==strcmp(args[0],"jobs")){
				jobs();
				continue;
			}
			if(0==strcmp(args[0],"fg")){
				fg(args);
				continue;
			}
			if(0==strcmp(args[0],"bg")){
				bg(args);
				continue;
			}
			pid_fork=fork();
				if(pid_fork==0){
					restore_terminal_signals();
					new_process_group(getpid());
					execvp(args[0], args);
					printf("Error, comand not found: %s \n", args[0]);
					exit(-1);
				}else{
					if(background==0){
						set_terminal(pid_fork);
						waitpid(pid_fork, &status, WUNTRACED);
						set_terminal(getpid());
						status_res=analyze_status(status, &info);
						if(info==255){
							printf("Foreground pid: %d, command: %s, %s, info: %d\n", pid_fork, args[0], "Error", info);
						}else {
							if(status_res==0){
								insert(&listaprocesos,pid_fork,args[0],STOPPED);
							}
								printf("Foreground pid: %d, command: %s, %s, info: %d\n", pid_fork, args[0], status_strings[status_res], info);
						}
						continue;
					}else{
						insert(&listaprocesos,pid_fork,args[0],BACKGROUND);
						printf("Background job running... pid: %d, command: %s\n", pid_fork, args[0]);
						continue;
					}
				}
		/* the steps are:
			 (1) fork a child process using fork()
			 (2) the child process will invoke execvp()
			 (3) if background == 0, the parent will wait, otherwise continue
			 (4) Shell shows a status message for processed command
			 (5) loop returns to get_commnad() function
		*/

	} // end while
}
