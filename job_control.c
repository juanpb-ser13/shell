/*--------------------------------------------------------
UNIX Shell Project
job_control module

Sistemas Operativos
Grados I. Informatica, Computadores & Software
Dept. Arquitectura de Computadores - UMA

Some code adapted from "Fundamentos de Sistemas Operativos", Silberschatz et al.
Alumno: Juan Palma Borda
DNI : 77180719X
--------------------------------------------------------*/

#include "job_control.h"
#ifndef __APPLE__
#include <malloc.h>
#endif
#include <string.h>

// -----------------------------------------------------------------------
//  get_command() reads in the next command line, separating it into distinct tokens
//  using whitespace as delimiters. setup() sets the args parameter as a
//  null-terminated string.
// -----------------------------------------------------------------------

void get_command(char inputBuffer[], int size, char *args[],int *background,Listatrabajos j)
{
	int length, /* # of characters in the command line */
		i,      /* loop index for accessing inputBuffer array */
		start,  /* index where beginning of next command parameter is */
		ct;     /* index of where to place the next parameter into args[] */

	ct = 0;
	*background=0;

	/* read what the user enters on the command line */
	length = read(STDIN_FILENO, inputBuffer, size);

	start = -1;
	if (length == 0)
	{
		while(j!=NULL){
			Listatrabajos k=j;
			killpg(j->pgid,SIGTERM);
			delete_job(&j,j->pgid);
			if(k->state==SUSPENDED){
			killpg(k->pgid,SIGCONT);
			}
			j=k->next;
		}
		printf(ROJO"\nBye\n"NEGRO);
		exit(0);            /* ^d was entered, end of user command stream */
	}
	if (length < 0){
		perror(ROJO"error reading the command"NEGRO);
		exit(-1);           /* terminate with error code of -1 */
	}

	/* examine every character in the inputBuffer */
	for (i=0;i<length;i++)
	{
		switch (inputBuffer[i])
		{
		case ' ':
		case '\t' :               /* argument separators */
			if(start != -1)
			{
				args[ct] = &inputBuffer[start];    /* set up pointer */
				ct++;
			}
			inputBuffer[i] = '\0'; /* add a null char; make a C string */
			start = -1;
			break;

		case '\n':                 /* should be the final char examined */
			if (start != -1)
			{
				args[ct] = &inputBuffer[start];
				ct++;
			}
			inputBuffer[i] = '\0';
			args[ct] = NULL; /* no more arguments to this command */
			break;

		default :             /* some other character */

			if (inputBuffer[i] == '&') // background indicator
			{
				*background  = 1;
				if (start != -1)
				{
					args[ct] = &inputBuffer[start];
					ct++;
				}
				inputBuffer[i] = '\0';
				args[ct] = NULL; /* no more arguments to this command */
				i=length; // make sure the for loop ends now

			}
			else if (start == -1) start = i;  // start of new argument
		}  // end switch
	}  // end for
	args[ct] = NULL; /* just in case the input line was > MAXLINE */
}
//------------------------------------------------------------------------
//													CREATES
//------------------------------------------------------------------------
void crear(Listatrabajos* l){
	*l=NULL;
}
//-----------------------------------------------------------------------
//                          NUEVAS FUNCIONES LISTAS
//-----------------------------------------------------------------------
void insert(Listatrabajos* l, pid_t pid, char* command, enum job_state state){
	Listatrabajos p=(Listatrabajos)malloc(sizeof(struct job_));
	p->pgid=pid;
	p->state=state;
	p->command=(char*)malloc(sizeof(strlen(command)));
	strcpy((p->command),command);
	p->next=*l;
	*l=p;
}
int delete_job(Listatrabajos* l, pid_t pid){
	if(*l==NULL){
		return 0;
	}
	if((*l)->pgid==pid){
			Listatrabajos n=(*l)->next;
			free((*l)->command);
			free(*l);
			*l=n;
			return 1;
	}else{
		delete_job(&((*l)->next),pid);
	}
}
void show(Listatrabajos l){
	if(l==NULL){
		printf(ROJO"No hay procesos en background o suspendidos\n"NEGRO);
	}
	int i=1;
	printf(PURPURA);
	while(l!=NULL){
		printf("%d  pid: %d, command: %s, state: %s\n",i, l->pgid, l->command, state_strings[l->state]);
		i++;
		l=l->next;
	}
	printf(NEGRO);
}
Listatrabajos buscarnumero(Listatrabajos l, int i){
	if(i<0){
		return NULL;
	}else{
		Listatrabajos ptr=l;
		i--;
		while(ptr!=NULL&&i>0){
			ptr=ptr->next;
			i--;
		}
		if(i>0){
			return NULL;
		}else{
			return ptr;
		}
	}
}
// -----------------------------------------------------------------------
/* interpretar valor estatus que devuelve wait */
enum status analyze_status(int status, int *info)
{
	if (WIFSTOPPED (status))
	{
		*info=WSTOPSIG(status);
		return(SUSPENDED);
	}
	else
	{
		// el proceso termio
		if (WIFSIGNALED (status))
		{ *info=WTERMSIG (status); return(SIGNALED);}
		else
		{ *info=WEXITSTATUS(status); return(EXITED);}
	}
}

// -----------------------------------------------------------------------
// cambia la accion de las seÃ±ales relacionadas con el terminal
void terminal_signals(void (*func) (int))
{
	signal (SIGINT,  func); // crtl+c interrupt tecleado en el terminal
	signal (SIGQUIT, func); // ctrl+\ quit tecleado en el terminal
	signal (SIGTSTP, func); // crtl+z Stop tecleado en el terminal
	signal (SIGTTIN, func); // proceso en segundo plano quiere leer del terminal
	signal (SIGTTOU, func); // proceso en segundo plano quiere escribir en el terminal
}

// -----------------------------------------------------------------------
void block_signal(int signal, int block)
{
	/* declara e inicializa máscara */
	sigset_t block_sigchld;
	sigemptyset(&block_sigchld );
	sigaddset(&block_sigchld,signal);
	if(block)
	{
		/* bloquea señal */
		sigprocmask(SIG_BLOCK, &block_sigchld, NULL);
	}
	else
	{
		/* desbloquea señal */
		sigprocmask(SIG_UNBLOCK, &block_sigchld, NULL);
	}
}
