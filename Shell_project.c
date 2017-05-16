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
Alumno: Juan Palma Borda
DNI : 77180719X
**/

#include "job_control.h"   // remember to compile with module job_control.c
#include <string.h>
#include <pthread.h>
Listatrabajos listaprocesos;
void hijos(char * args[],int numero){
	char *prog[MAX_LINE/2];
	int i=0;
	while(args[numero]!=NULL){
		prog[i]=args[numero];
		numero++;
		i++;
	}
	if(i>=2){
		if(0==strcmp(args[i-1],"+")){
			prog[i-1]=NULL;
		}
	}
	prog[i]=NULL;
	restore_terminal_signals();
	new_process_group(getpid());
	execvp(prog[0], prog);
	printf(ROJO"Error, comand not found: %s \n"NEGRO, args[numero]);
	exit(-1);
}
// -----------------------------------------------------------------------
//                            Manejador
// -----------------------------------------------------------------------
void manejador(int numero){
	Listatrabajos k=listaprocesos;
	int status=0;
	int info;
	enum status status_res;
	pid_t hijo;
	block_SIGCHLD();
	while(k!=NULL){
		//algo con cada proceso
		hijo=waitpid(k->pgid,&status, WNOHANG|WUNTRACED|WCONTINUED);
		status_res=analyze_status(status,&info);

		if(k->state==BACKGROUND){
			if(WIFCONTINUED(status)){
				k->state=BACKGROUND;
			}else if(hijo!=0&&status_res!=0){
				if(k->respawn){
					pid_t pid_fork=fork();
					if(pid_fork==0){
						hijos(k->all, 0);
					}else{
						k->pgid=pid_fork;
					}
					printf(CIAN"Background job running... pid: %d, command: %s\n"NEGRO, pid_fork, k->command);
				}else{
					delete_job(&listaprocesos,hijo);
				}
			}else if(hijo!=0){
				k->state=STOPPED;
			}
		}else{
			if(WIFCONTINUED(status)){
				k->state=BACKGROUND;
			}else{
				if(status_res==0){
					delete_job(&listaprocesos,hijo);
				}
			}

		}
		k=k->next;
	}
	unblock_SIGCHLD();
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
		printf(ROJO"Error, directory not found: %s \n"NEGRO, args[1]);
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
			listaprocesos->respawn=0;
			set_terminal(p);
			waitpid(p, &status, WUNTRACED);
			set_terminal(getpid());
			status_res=analyze_status(status, &info);
			if(info==255){
				delete_job(&listaprocesos,listaprocesos->pgid);
				printf(CIAN"Foreground pid: %d, command: %s, %s, info: %d\n"NEGRO, p, l, "Error", info);
			}else {
				if(status_res==0){
					listaprocesos->state=STOPPED;
				}else{
					delete_job(&listaprocesos,listaprocesos->pgid);
				}
					printf(CIAN"Foreground pid: %d, command: %s, %s, info: %d\n"NEGRO, p, l, status_strings[status_res], info);
			}
			free(l);
		}else{
					printf(ROJO"Error no existen procesos en background o suspendido\n"NEGRO);
		}
	}else{
		int i=0;
		i=atoi(args[1]);
		if(i!=0){
			Listatrabajos j=buscarnumero(listaprocesos, i);
			if(j!=NULL){
				killpg(j->pgid,SIGCONT);
				pid_t p=j->pgid;
				char*  l=(char*)malloc(sizeof(strlen(j->command)));
				j->respawn=0;
				strcpy(l,j->command);
				set_terminal(p);
				waitpid(p, &status, WUNTRACED);
				set_terminal(getpid());
				status_res=analyze_status(status, &info);
				if(info==255){
					delete_job(&listaprocesos,j->pgid);
					printf(CIAN"Foreground pid: %d, command: %s, %s, info: %d\n"NEGRO, p, l, "Error", info);
				}else {
					if(status_res==0){
						j->state=STOPPED;
					}else{
						delete_job(&listaprocesos,j->pgid);
					}
						printf(CIAN"Foreground pid: %d, command: %s, %s, info: %d\n"NEGRO,p, l, status_strings[status_res], info);
				}
					free(l);
			}else{
				printf(ROJO"Error no existe ese proceso"NEGRO);
			}
		}else{
				printf(ROJO"Error no existe ese proceso"NEGRO);
		}
	}
}
void bg(char* args[]){
	if(args[1]==NULL){
		Listatrabajos cambio=buscarnumero(listaprocesos,1);
		if(cambio==NULL){
			printf(ROJO"Error no existen procesos en background o suspendidos\n"NEGRO);
		}else{
			killpg(cambio->pgid,SIGCONT);
			cambio->respawn=0;
			cambio->state=BACKGROUND;
			printf(CIAN"Background job running... pid: %d, command: %s\n"NEGRO, cambio->pgid, cambio->command);
		}
	}else{
		int i=0;
		i=atoi(args[1]);
		if(i!=0){
			Listatrabajos cambio=buscarnumero(listaprocesos,i);
			if(cambio!=NULL){
				killpg(cambio->pgid,SIGCONT);
				cambio->respawn=0;
				cambio->state=BACKGROUND;
				printf(CIAN"Background job running... pid: %d, command: %s\n"NEGRO, cambio->pgid, cambio->command);
			}else{
				printf(ROJO"Error no existe ese proceso\n"NEGRO);
			}
		}else{
				printf(ROJO"Error parametro no valido\n"NEGRO);
		}
	}
}
int timeout(char* args[]){
	int i=0;
	i=atoi(args[1]);
	if(i!=0){
		return 1;
	}else{
		printf(ROJO"Error parametro no valido\n"NEGRO);
		return 0;
	}
}
void *do_work(void *p){
	arguments* h=p;
	sleep((unsigned int)(*h)->i);
	killpg((*h)->pid, SIGTERM);
	killpg((*h)->pid, SIGCONT);
	free((void*)*h);
	pthread_exit(NULL);
}
int buscarespawneable(char* args[]){
	int i=0;
	while(args[i]!=NULL){
		i++;
	}
	if(i<1||0!=strcmp(args[i-1],"+")){
		return 0;
	}else{
		return 1;
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
	int numero=0;
	//crear funciones
	crear(&listaprocesos);
	ignore_terminal_signals();
	signal(SIGCHLD,manejador);
	while (1)   /* Program terminates normally inside get_command() after ^D is typed*/
	{
		printf(VERDE"%s:"NEGRO, getcwd(buffer,512));
		fflush(stdout);
		get_command(inputBuffer, MAX_LINE, args, &background, listaprocesos);  /* get next command */

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
			int y=0;
			if(0==strcmp(args[0],"timeout")){
				y=timeout(args);
				if(!y){
					continue;
				}
				numero=2;
			}
			int g=buscarespawneable(args);
			pid_fork=fork();
				if(pid_fork==0){
					hijos(args,numero);
				}else{
					if(y){
						arguments h=(arguments)malloc(sizeof(struct arg));
						h->pid=pid_fork;
						h->i=atoi(args[1]);
						int rc;
						pthread_t tid;
						rc=pthread_create(&tid, NULL,do_work,(void*)(&h));
					}
					if(background==0&&!g){
						set_terminal(pid_fork);
						waitpid(pid_fork, &status, WUNTRACED);
						set_terminal(getpid());
						status_res=analyze_status(status, &info);
						if(info==255){
							printf(CIAN"Foreground pid: %d, command: %s, %s, info: %d\n"NEGRO, pid_fork, args[numero], "Error", info);
						}else {
							if(status_res==0){
								insert(&listaprocesos,pid_fork,args[numero],STOPPED,g,args);
							}
								printf(CIAN"Foreground pid: %d, command: %s, %s, info: %d\n"NEGRO, pid_fork, args[numero], status_strings[status_res], info);
						}
						numero=0;
						continue;
					}else{
						insert(&listaprocesos,pid_fork,args[numero],BACKGROUND,g,args);
						printf(CIAN"Background job running... pid: %d, command: %s\n"NEGRO, pid_fork, args[numero]);
						numero=0;
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
