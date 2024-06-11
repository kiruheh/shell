#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
//структура шел команд
typedef struct cmd_inf {
    char** argv; //file, argv[1], argv[2]...
    char* infile; // <
    char* outfile; // >
    int back; //&
    int file; // >> check
    struct cmd_inf* pipe; // |
    struct cmd_inf* next;// &&
}cmd;
pid_t pid;

// перенаправление ctrl + C для убийств процессов детей
void ckill(int s){
    kill(pid, SIGKILL);
    printf("\n");
    signal(SIGINT, SIG_DFL);
}

//всякие полезные мелочи
void decorate(){
    printf(">>");
}
void print_error(const char* error) {
    printf("%s\n", error);
    exit(1);
}
int symb_check(char c) {
    char op[] = "|&><0";
    for (int i = 0; op[i] != '0'; i++)
        if (c == op[i]) return 1;
    return 0;
}



//ls -> /usr/bin/ls
char* command_check(char* string){
    char st[] = "/usr/bin/\0";
    size_t len = strlen(string) + strlen(st);
    char* ret = (char*)malloc(len);
    strcpy(ret, st);
    strcat(ret, string);
    free(string);
    return ret;
}




//по строке из инпута заполняет список команд. Возвращает указатель на первый элемент списка
cmd* zap(char* string, size_t* spaces) {

    //общие приготовления
    int str_count = 0;
    int space_count = 0;
 int argv_count = 0;
    size_t max_argv = 20;
    size_t add_argv = 10;
    cmd* ret = (cmd*)malloc(sizeof(cmd));
    if(ret == NULL) print_error("Zap malloc 1 error");
    ret->argv = (char**)malloc(sizeof(char*) * max_argv);
    if(ret->argv == NULL) print_error("zap malloc 2 error");
    ret->argv[0] = NULL;
    ret->infile = NULL;
    ret->outfile = NULL;
    ret->back = 0;
    ret->file = 0;
    ret->pipe = NULL;
    ret->next = NULL;

    while (1) {

        if (argv_count >= max_argv - 1) {
            max_argv += add_argv;
            ret->argv = (char**)realloc(ret->argv, sizeof(char*) * max_argv);
            if(ret->argv == NULL) print_error("Zap realloc argv error");
        }

        while(string[str_count] == ' ') str_count++;
        if(string[str_count] == '\n') return ret;

        if (!symb_check(string[str_count])) {
            ret->argv[argv_count] = (char*)malloc(spaces[space_count] + 1);
            if(ret->argv[argv_count] == NULL) print_error("Zap malloc 3 error");
            ret->argv[argv_count][spaces[space_count++]] = '\0';
            ret->argv[argv_count + 1] = NULL;
            for (int i = 0; string[str_count] != ' '; i++) {
                ret->argv[argv_count][i] = string[str_count++];
                if (string[str_count] == '\n') return ret;
            }
            argv_count++;
            str_count++;
        }
        switch (string[str_count]) {
        //на фоне или next
        case '&':
            if (string[str_count + 2] == '&'){
                ret->next = zap((char*)(string + str_count + 4), (size_t*)(spaces + space_count + 2));
                return ret;
            }
            ret->back = 1;
            space_count ++;
            str_count += 2;
            if(string[str_count] == '\n') return ret;
            break;
        //pipe
        case '|':
            ret->pipe = zap((char*)(string + str_count + 2), (size_t*)(spaces + space_count + 1));
            return ret;
            break;
        //infile
        case '<':
            str_count += 2;
            if (string[str_count] == '\n') print_error("Infile argument error");
            space_count++;
            ret->infile = (char*)malloc(spaces[space_count] + 1);
            for (int i = 0; string[str_count] != ' '; i++) {
                ret->infile[i] = string[str_count++];
                if (string[str_count] == '\n') {
                    ret->infile[spaces[space_count++]] = '\0';
return ret;
                }
            }
            str_count++;
            ret->infile[spaces[space_count++]] = '\0';
            break;
        //outfile (>  >>)
        case '>':
            str_count += 2;
            if (string[str_count] == '>') {
                ret->file = 1;
                str_count += 2;
                space_count++;
            }

            if (string[str_count] == '\n') print_error("Outfile argument error");
            space_count++;
            ret->outfile = (char*)malloc(spaces[space_count] + 1);
            for (int i = 0; string[str_count] != ' '; i++) {
                ret->outfile[i] = string[str_count++];
                if (string[str_count] == '\n') {
                    ret->outfile[spaces[space_count++]] = '\0';
                    return ret;
                }
            }
            str_count++;
            ret->outfile[spaces[space_count++]] = '\0';
            break;
        //выход из заполнения
        case '\n':
            return ret;
        default:
           if(string[str_count + 1] == '\n') return ret;
        }


    }
}

//сканируем длины отдельных слов для красивых маллоков
size_t* space_scan(char* string) {
    size_t max_size = 100;
    size_t add_size = 50;
    size_t* spaces = (size_t*)malloc(sizeof(size_t) * max_size);
    int s_count = 0;
    if (spaces == NULL) print_error("Spaces malloc error");

    for (int i = 0, j = 0;; i++, j++) {

        if (s_count >= max_size) {
            max_size += add_size;
            spaces = (size_t*)realloc(spaces, sizeof(size_t) * max_size);
            if (spaces == NULL) print_error("Spaces realloc error");
        }

        if (string[i] == ' ' || string[i] == '\n') {
            spaces[s_count++] = j;
            j = -1;
            if (string[i] == '\n') break;
        }
    }
    spaces[s_count] = 0;
    return spaces;
}

//отчищаем всё что маллочили
void free_all(cmd* all) {
    if (all == NULL)
        return;
    free_all(all->next);
    free_all(all->pipe);
    if (all->argv != NULL) {
        for (int i = 0; all->argv[i] != NULL; i++) free(all->argv[i]);
        free(all->argv);
    }
    if (all->infile != NULL) free(all->infile);
    if (all->outfile != NULL) free(all->outfile);
    free(all);
}

//считывает строку из стандартного ввода и возвращает её без лишних пробелов
char* get_string_without_spase() {
    size_t add = 100;
    size_t size = add;
    char* ret_str = (char*)malloc(size);
    if (ret_str == NULL) {
        perror("Input malloc error");
        exit(1);
    }
    char c = getchar();
    int i = 0;
    while (c == ' ' && c != '\n' && c != '\0') {
        c = getchar();
    }
    ret_str[i++] = c;
    if(symb_check(c)) ret_str[i++] = ' ';
    while (!(c == '\n' || c == '\0')) {
        c = getchar();
        if (c == ' ' && ret_str[i - 1] == ' ') continue;
        if (i >= size - 4) {
            size += add;
            ret_str = (char*)realloc(ret_str, size);
            if (ret_str == NULL) {
                perror("Input realloc error");
                exit(1);
            }
        }

        else if (symb_check(c)) {
            if (i != 0 && ret_str[i - 1] != ' ') ret_str[i++] = ' ';
            ret_str[i++] = c;
            ret_str[i++] = ' ';
        }
        else
            ret_str[i++] = c;
    }
    ret_str[i] = '\0';


    //если захочется посмотреть какая строка получается
    //printf("check srting:\n%s", ret_str);


    return ret_str;
}

//выполняет все команды
void do_comm(cmd* comm, int pfd) {
    int tout = dup(1);
    int fd3[2];
 //перенаправляем вывод для пайпа
    if(comm->pipe != NULL){
        pipe(fd3);
        if(dup2(fd3[1], 1) < 0) print_error("dup2 error");
        close(fd3[1]);
    }
    //сын запустит программу с аргументами comm->argv
    if((pid = fork()) == 0){
        if(comm->outfile != NULL){
            int fd;
            if(comm->file){
                fd = open(comm->outfile,O_RDWR | O_APPEND | O_CREAT, 0777);
            }
            else fd = open(comm->outfile, O_WRONLY | O_TRUNC | O_CREAT, 0777);
            if(fd < 0) print_error("open outfile error");
            if(dup2(fd, 1) < 0) print_error("dup2 error");}
        if(comm->infile != NULL){
            int fd2;
            fd2 = open(comm->infile, O_RDONLY, 0777);
            if(fd2 < 0) printf("open infile error");
            if(dup2(fd2, 0) < 0) print_error("dup2 error");
        }
        else dup2(pfd, 0);
        execv(comm->argv[0], comm->argv);
        comm->argv[0] = command_check(comm->argv[0]);
        execv(comm->argv[0], comm->argv);
        print_error("exec error");
    }
    //перенаправляем ctrl+C чобы он убивал сына
    signal(SIGINT, ckill);
    if(comm->pipe != NULL){
        //
        dup2(tout, 1);
        wait(NULL);
        do_comm(comm->pipe, fd3[0]);
    }
    int status;
    if(!comm->back) wait(&status);
    signal(SIGINT, SIG_DFL);
    if(comm->next != NULL){
        if(WIFEXITED(status))
            do_comm(comm->next, dup(0));
    }
}

int main(void) {
    printf(
        "***********************\n"
        "*                     *\n"
        "* Welcome to shell!!! *\n"
        "*                     *\n"
        "***********************\n\n\n");
    int b = 1;
    char* commands;
    cmd* test = NULL;
    size_t* spaces;

    //exit -> завершает программу
    while (b) {
        decorate();
        commands = get_string_without_spase();
        b = strcmp(commands, "exit\n");
        if (b) b = strcmp(commands, "exit \n");
        spaces = space_scan(commands);
        test = zap(commands, spaces);
 if(b) do_comm(test, dup(0));
        free_all(test);
        free(commands);
    }
    printf("\n\n\n"
        "***********************\n"
        "*                     *\n"
        "*  Have a nice day!!! *\n"
        "*                     *\n"
        "***********************\n");
    return 0;
}
