/*--------------------------------------------------------
UNIX Shell Project
function prototypes, macros and type declarations for job_control module

Sistemas Operativos
Grados I. Informatica, Computadores & Software
Dept. Arquitectura de Computadores - UMA

Some code adapted from "Fundamentos de Sistemas Operativos", Silberschatz et al.
Alumno: Juan Palma Borda
DNI : 77180719X
--------------------------------------------------------*/

#ifndef _JOB_CONTROL_H
#define _JOB_CONTROL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
//------------ Colores--------------------------------------------------
#define ROJO "\x1b[31;1;1m"
#define NEGRO "\x1b[0m"
#define VERDE "\x1b[32;1;1m"
#define AZUL "\x1b[34;1;1m"
#define CIAN "\x1b[36;1;1m"
#define MARRON "\x1b[33;1;1m"
#define PURPURA "\x1b[35;1;1m"

// ----------- ENUMERATIONS ---------------------------------------------
enum status { SUSPENDED, SIGNALED, EXITED};
enum job_state { FOREGROUND, BACKGROUND, STOPPED };
static char* status_strings[] = { "Suspended","Signaled","Exited"};
static char* state_strings[] = { "Foreground","Background","Stopped" };

// ----------- JOB TYPE FOR JOB LIST ------------------------------------
typedef struct job_* Listatrabajos;
typedef struct job_
{
	pid_t pgid; /* group id = process lider id */
	char * command; /* program name */
	enum job_state state;
	Listatrabajos next; /* next job in the list */
}job;

// -----------------------------------------------------------------------
//      PUBLIC FUNCTIONS
// -----------------------------------------------------------------------

void get_command(char inputBuffer[], int size, char *args[],int *background);

void crear(Listatrabajos* l);
void insert(Listatrabajos* l, pid_t pid, char* command, enum job_state state);
int delete_job(Listatrabajos* l, pid_t pid);
void show(Listatrabajos l);
Listatrabajos buscarnumero(Listatrabajos l, int i);

enum status analyze_status(int status, int *info);

// -----------------------------------------------------------------------
//      PRIVATE FUNCTIONS: BETTER USED THROUGH MACROS BELOW
// -----------------------------------------------------------------------

// void print_item(job * item);
//
// void print_list(job * list, void (*print)(job *));

void terminal_signals(void (*func) (int));

void block_signal(int signal, int block);

// -----------------------------------------------------------------------
//      PUBLIC MACROS
// -----------------------------------------------------------------------

// #define list_size(list) 	 list->pgid   // number of jobs in the list
// #define empty_list(list) 	 !(list->pgid)  // returns 1 (true) if the list is empty
//
// #define new_list(name) 			 new_job(0,name,FOREGROUND)  // name must be const char *

// #define print_job_list(list) 	 print_list(list, print_item)

#define restore_terminal_signals()   terminal_signals(SIG_DFL)
#define ignore_terminal_signals() terminal_signals(SIG_IGN)

#define set_terminal(pid)        tcsetpgrp (STDIN_FILENO,pid)
#define new_process_group(pid)   setpgid (pid, pid)

#define block_SIGCHLD()   	 block_signal(SIGCHLD, 1)
#define unblock_SIGCHLD() 	 block_signal(SIGCHLD, 0)

// macro for debugging----------------------------------------------------
// to debug integer i, use:    debug(i,%d);
// it will print out:  current line number, function name and file name, and also variable name, value and type
#define debug(x,fmt) fprintf(stderr,"\"%s\":%u:%s(): --> %s= " #fmt " (%s)\n", __FILE__, __LINE__, __FUNCTION__, #x, x, #fmt)

// -----------------------------------------------------------------------
#endif
