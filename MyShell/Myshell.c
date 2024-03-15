#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h> 		// permisiuni
#include <pwd.h> 		// getpwuid
#include <grp.h> 		// getgrgid
#include <time.h> 
#include <readline/readline.h> 	// citire linie input user
#include <readline/history.h> 	// istoric comenzi
#include <dirent.h> 		// ls
#include <fcntl.h> 
#include <signal.h> 		// SIGTSTP - suspendarea unui program
#include <pthread.h>
#include <stdbool.h> 

#define clear() printf("\033[H\033[J")

#define MAXIMC 1000

//Culori
#define GREEN "\x1b[92m"
#define BLUE "\x1b[94m"
#define CYAN "\x1b[96m"
#define MAX_PROC 64

int proc_pid[MAX_PROC];


void setup() {

    int i;
    for (i = 0; i < MAX_PROC; i++) {
        proc_pid[i] = -1;
    }

}

void assignPID(int childPid) {

   int i;
   for (i = 0; i < MAX_PROC; i++) {
        if (proc_pid[i] == -1) {
            proc_pid[i] = childPid;
            break;
            }
   }

}

void printDir() 
{
    char cwd[1000], aux[1000] = "MyShell:";
    if(getcwd(cwd, sizeof(cwd)) == NULL) 
        perror("getcwd() Eroare.\n");
    else
    {
        strcat(aux, cwd); strcat(aux, "\n");
        printf("%s", aux);
    }
}

void get_input(char *input) 
{
    char *aux;
    aux = readline("$ "); 

    if(strlen(aux))
    {
        add_history(aux);
        strcpy(input, aux);
    }
    free(aux);
}

int tip(char *input) 
{
    if(strstr(input, "&&") || strstr(input, "||")) {
        return 0;
    }
    if(strchr(input, '&')) return 1; //in Background
    if(strchr(input, '|')) return 2; //Pipe
    if(strchr(input, '<') || strchr(input, '>')) return 3;
    return 4;
}

int funct_exit()
{
    exit(0);
    return 0;
}

int funct_pwd()
{
    char cwd[1000];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
      perror("getcwd() error");
      return -1;
    }
    
    printf("%s\n", cwd);
    return 0;
}

int funct_echo(char *input)
{
   printf("%s\n\n", input + 5);
   return 0;
}

int funct_clear()
{
    clear(); 
    return 0;
}


void nume_file(struct dirent *name)
{
    if (name->d_type == DT_REG)
        printf("%s%s\n", BLUE, name->d_name);
    else if (name->d_type == DT_DIR)
        printf("%s%s\n", GREEN, name->d_name);
    else
        printf("%s%s\n", CYAN, name->d_name);
}

void afisare_l(struct stat detalii)
{
    printf("%s", (S_ISDIR(detalii.st_mode)) ? "d" : "-");
    printf("%1s", (detalii.st_mode & S_IRUSR) ? "r" : "-");
    printf("%1s", (detalii.st_mode & S_IWUSR) ? "w" : "-");
    printf("%1s", (detalii.st_mode & S_IXUSR) ? "x" : "-");	
    printf("%1s", (detalii.st_mode & S_IRGRP) ? "r" : "-");
    printf("%1s", (detalii.st_mode & S_IWGRP) ? "w" : "-");
    printf("%1s", (detalii.st_mode & S_IXGRP) ? "x" : "-");
    printf("%1s", (detalii.st_mode & S_IROTH) ? "r" : "-");
    printf("%1s", (detalii.st_mode & S_IWOTH) ? "w" : "-");
    printf("%1s", (detalii.st_mode & S_IXOTH) ? "x" : "-");
    printf("%2ld ", (unsigned long)(detalii.st_nlink));    //nr leg
    printf("%s ", (getpwuid(detalii.st_uid))->pw_name);    // utilizator
    printf("%s ", (getgrgid(detalii.st_gid))->gr_name);    //gr
    printf("%5lld ", (unsigned long long)(detalii.st_size));
    char timp[14];
    strftime(timp, 14, "%h %d %H:%M", localtime(&detalii.st_mtime));
    printf("%s ", timp);
}

int funct_lsar(int l, int a, int r)
{
    int i, total = 0;
    struct dirent **namelist;
    struct stat detalii;
    int nr = scandir(".", &namelist, 0, alphasort);
    
    if (nr == 0)
    {
        printf("Director Gol.\n\n");
        return 0;
    }

    printf("%sTotal din Director: %d.\n", CYAN, nr - 2);
    printf("\033[0m");

    if (r == 0)
    {
        if (a == 1)
        {
            for (i = 0; i < nr; i++)
            {
                if (stat(namelist[i]->d_name, &detalii))
                    continue;

                total += detalii.st_blocks;
                if (l == 1)
                    afisare_l(detalii);
                nume_file(namelist[i]);
                printf("\033[0m");
            }
        }
        else
        {
            for (i = 0; i < nr; i++)
            {
                if (strcmp(namelist[i]->d_name, ".") == 0 || strcmp(namelist[i]->d_name, "..") == 0)
                    continue;
                else
                {
                    if (stat(namelist[i]->d_name, &detalii))
                        continue;

                    total += detalii.st_blocks;
                    if (l == 1)
                        afisare_l(detalii);
                    nume_file(namelist[i]);
                    printf("\033[0m");
                }
            }
        }
    }
    else
    {
        if (a == 1)
        {
            for (i = nr - 1; i >= 0; i--)
            {
                if (stat(namelist[i]->d_name, &detalii))
                    continue;

                total += detalii.st_blocks;
                if (l == 1)
                    afisare_l(detalii);
                nume_file(namelist[i]);
                printf("\033[0m");
            }
        }
        else
        {
            for (i = nr - 1; i >= 0; i--)
            {
                if (strcmp(namelist[i]->d_name, ".") == 0 || strcmp(namelist[i]->d_name, "..") == 0)
                    continue;
                else
                {
                    if (stat(namelist[i]->d_name, &detalii))
                        continue;

                    total += detalii.st_blocks;
                    if (l == 1)
                        afisare_l(detalii);
                    nume_file(namelist[i]);
                    printf("\033[0m");
                }
            }
        }
    }
    printf("%sTotal (object contents): %d.\n", CYAN, total / 2);
    printf("\n");
    printf("\033[0m");
    return 0;
}

int funct_cd(char *path)
{
    if(chdir(path)==0) 
    {
        char aux[MAXIMC];
        char *path=getcwd(aux, sizeof(aux));
        strcpy(path, aux); 
        printf("\n"); 
        return 0;

    }
    else perror("Eroare la cd");
    return -1;
}

int funct_mkdir(char *nume_folder)
{
    if(mkdir(nume_folder, 0666)==-1) 
    {
        perror("Eroare la mkdir\n\n");
        return -1;
    }
    printf("\n");
    return 0;
}

int funct_rmdir(char *nume)
{
    if(rmdir(nume)==-1)
    {
        perror("Eroare la rmdir\n\n");
        return -1;
    }
    printf("\n");
    return 0;
}

int funct_cp(char *fila, char *copie) {
    FILE *f1, *f2;
    char cp;

    f1 = fopen(fila, "r");
    if (f1 == NULL) {
        perror("Eroare la citire fila.\n\n");
        return -1;
    }

    f2 = fopen(copie, "w");
    if (f2 == NULL) {
        perror("Eroare in CP la copie.\n\n");
        fclose(f1);
        return -1;
    }

    while ((cp = getc(f1)) != EOF) {
        putc(cp, f2);
    }

    printf("\n");
    fclose(f1);
    fclose(f2);
    return 0;
}

int funct_grep(const char *text, const char *keyword) {
    const char *found = strstr(text, keyword);

    if (found != NULL) {
        printf("Cuvantul cheie '%s' a fost gasit in text!\n", keyword);
        return 0;
    } else {
        printf("Cuvantul cheie '%s' nu a fost gasit in text.\n", keyword);
        return -1; 
    }
}

int comanda4(char *input)
{

    if(strstr(input, "exit")) { return funct_exit();}
    if(strstr(input, "clear")) {return funct_clear();}
    if(strstr(input, "pwd")) { return funct_pwd();}
    if(strstr(input, "echo")) {return funct_echo(input); }
    if(strstr(input, "ls"))
    {
   	int l = 0, a = 0, r = 0;
    	if(strstr(input + 2, "l")!=NULL)
    	{
	   l = 1;
	}
	if(strstr(input + 2, "a")!=NULL)
    	{
	   a = 1;
	}
	if(strstr(input + 2, "r")!=NULL)
    	{
	   r = 1;
	}
     	return funct_lsar(l,a,r);

    }
    
      if (strstr(input, "grep")) {
        char *cuvinte[3];

        cuvinte[0] = strtok(input, " "); // "grep"
        cuvinte[1] = strtok(NULL, " "); // cuvant_cheie
        cuvinte[2] = strtok(NULL, " "); // text

        if (cuvinte[1] != NULL && cuvinte[2] != NULL && strtok(NULL, " ") == NULL) {
            return funct_grep(cuvinte[2], cuvinte[1]);  // funct_grep(text, cuvant_cheie)
        } else {
            printf("Comanda 'grep' nu are argumente valide.\n");
        }

        return -1;
    }
     
    
    if (strstr(input, "cd")) 
    {
        char *path = strchr(input, ' ');
        if (path != NULL) {
            path++; // sarim peste spatiu
            return funct_cd(path);
        } else {
            printf("Comanda 'cd' nu are argument valid.\n");
            return -1;
        }
    } 
    
    if (strstr(input, "mkdir")) {
	char *token = strtok(input, " ");
	token = strtok(NULL, " ");
	    
	if (token != NULL) {
           return funct_mkdir(token);

	} else {
           printf("Comanda 'mkdir' nu are argument valid.\n");
          
	}
	    
	return -1;
    }
    
    if (strstr(input, "rmdir")) {
	char *token = strtok(input, " ");
	token = strtok(NULL, " ");
	    
	if (token != NULL) {
           return funct_rmdir(token);        
	} else {
           printf("Comanda 'rmdir' nu are argument valid.\n");
	}
	    
	return -1;
    }	
   
    
   if (strstr(input, "cp")) {
        char *cuvinte[3];

        cuvinte[0] = strtok(input, " "); // "cp"
        cuvinte[1] = strtok(NULL, " "); // fisier1
        cuvinte[2] = strtok(NULL, " "); // fisier2

        if (cuvinte[1] != NULL && cuvinte[2] != NULL && strtok(NULL, " ") == NULL) {
            return funct_cp(cuvinte[1], cuvinte[2]);  
        } else {
            printf("Comanda 'cp' nu are argumente valide.\n");
        }

        return -1;
    }
    
   return 0;
}


int comanda3(char *input) 
{

    char **arg;
    arg = malloc(8 * sizeof(char *));
        
       char **rezultat;
       rezultat= malloc(8 * sizeof(char *));
       
       separare(input, rezultat);
       
       
       char *semn;
       semn = malloc(8 * sizeof(char *));
   
       int i=0;
       
       while(*rezultat[i]!='>' && *rezultat[i]!='<'){    
           arg[i]=rezultat[i];
           i++;            
       }
       semn=rezultat[i];
       
       int pid;
       
       char *fisier=rezultat[i+1];
       
              
       switch(pid=fork())
        {
        case 0:
            if(*semn=='<')
            {
            	
		
                int inFile = open(fisier, O_RDONLY);
                
                if(inFile == -1){
                	perror("Nu am putut deschide fisierul de input.\n");
                }
		
		char buffer[4096];
       		
	        ssize_t bytesRead=read(inFile, buffer, sizeof(buffer));
		if(bytesRead==-1){
		     perror("A aparut o eroare la citirea inputului.\n");
		     return 0;
		}
		
       		
       		char *separator = " \t\n";
       		char *token;
       		i = 1;
	        token = strtok (buffer,separator);
	        arg[1]=token;
	        i++;
	        while (token != NULL)
  		{
    			token = strtok (NULL, separator);
  			arg[i]=token;
  			i++;
  		}
       		
       		
		arg[i]=NULL;
      			 
	    
                close(inFile);
            }

            if(*semn=='>')
            {
            	
                int outFile=open(fisier, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if(outFile==-1)
                {
                    perror("FIsier Iesire nu a putut fi deschis.\n"); 
                    return 0;
                }
                dup2(outFile, STDOUT_FILENO);
                close(outFile);
                arg[i]=NULL;
            }
            
		
            execvp(rezultat[0], arg);
            signal(SIGTSTP, SIG_DFL);
            fprintf(stderr, "EROARE: %s nu e program\n\n", input);
            //exit(1);
            break;
        case -1:
            perror("EROARE PROCES COPIL!\n\n");
            return 0;

        default:
            assignPID(pid);
            wait(NULL);

    }
    return 0;
}



void separare(char *input, char **rez) //sparge inputul dupa " "
{
	int i;
	for(i = 0; i < 80; i++)
    	{
    		rez[i] = strsep(&input, " ");
		if(rez[i] == NULL) 
			break;
		if(strlen(rez[i]) == 0) 
		   	i--;
	}
}




void comanda0(char *input) { //Comanda 0 (Operatii logice)
    if (strstr(input, "&&")) {
        op_cond_si(input);
    }
    else if (strstr(input, "||")) {
        op_cond_sau(input);
    }
    

}

void op_cond_si(char *input){

   char *comm1 = strtok(input, "&");
   comm1[strlen(comm1) - 1] = '\0'; //eliminam spatiul de dupa
	
   char *aux = strtok(NULL, "&\n");	
   char comm2[strlen(aux -1)];
   strcpy(comm2, aux + 1);  // eliminam spatiul de inainte
   
    if (comanda4(comm1) == 0) { //a mers fara erori
            comanda4(comm2);
        }
    else {
        if (strcmp("true", comm1) == 0) {
            comanda4(comm2);
            }
    }	
}



void op_cond_sau(char *input){

   char *comm1 = strtok(input, "|");
   comm1[strlen(comm1) - 1] = '\0'; //eliminam spatiul de dupa
	
   char *aux = strtok(NULL, "|\n");
   char comm2[strlen(aux -1)];
   strcpy(comm2, aux + 1);  // eliminam spatiul de inainte  

    if (comanda4(comm1) != 0) {//a mers cu eroare
        comanda4(comm2);
        }
    else {
        if (strcmp("false", comm1) == 0) {
            comanda4(comm2);
        }
    }
}


void comanda2(char *input) // Comanda 2 (Pipe)
{
    char **comm1 = malloc(8 * sizeof(char *)), **comm2 = malloc(8 * sizeof(char *)), *v_comms[2];
    int i;

    v_comms[0] = strsep(&input, "|");
    v_comms[1] = strsep(&input, "|");

    separare(v_comms[0], comm1);
    separare(v_comms[1], comm2);

    int pipefd[2];
    pid_t p1, p2;

    if (pipe(pipefd) < 0)
    {
        perror("Eroare la initializare Pipe");
        return;
    }

    p1 = fork();
    if (p1 < 0)
    {
        perror("Eroare la FORK 1 in Pipe");
        return;
    }

    if (p1 == 0) // copil_1
    {
        close(pipefd[0]); 
        dup2(pipefd[1], STDOUT_FILENO); 
        close(pipefd[1]);

        // Execută prima comandă în shell
        if (execvp(comm1[0], comm1) < 0)
        {
            perror("Eroare la execvp 1");
            exit(EXIT_FAILURE);
        }
    }
    else // parinte_1
    {
        p2 = fork();
        if (p2 < 0)
        {
            perror("Eroare la FORK 2 in Pipe");
            return;
        }

        if (p2 == 0) // copil_2
        {
            close(pipefd[1]); 
            dup2(pipefd[0], STDIN_FILENO); 
            close(pipefd[0]); 


            if (execvp(comm2[0], comm2) < 0)
            {
                perror("Eroare la execvp 2");
                exit(EXIT_FAILURE);
            }
        }
        else // parinte_2
        {
            close(pipefd[0]); 
            close(pipefd[1]); 


            waitpid(p1, NULL, 0);
            waitpid(p2, NULL, 0);
        }
    }

    printf("\n");
    free(comm1);
    free(comm2);
}


// Suspendarea proceselor care ruleaza
void sigHandler(int sig_num)
{
    
    signal(SIGTSTP, sigHandler);
    int i;
    if (proc_pid[0] != -1) {
        printf("\nProcess with pid %d suspended\n", proc_pid[0]);
    }
    for (i = 0; i < MAX_PROC; i++) {
        if (proc_pid[i] != -1) {
            kill(proc_pid[i],SIGKILL);
            proc_pid[i] = -1;
        }

    }
}

int main(int argv, char *argc[])
{
    clear();
    printf("CTRL+C / EXIT.\n");
    char input[MAXIMC];

    setup();

    signal(SIGTSTP, sigHandler);

    while(1)
    {
        printDir();
        get_input(input);
        int t = tip(input);
        if(t==0) comanda0(input);
        else if(t==2) comanda2(input);
        	else if(t==3) comanda3(input);
        		else if(t==4) comanda4(input);

    }

    return 0;
}
