#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define max_line 100
#define max_len 100

char *const flags[] = {"-r", "-f", NULL};

int check_argv(char *argv){
    for(int i = 0; flags[i] != NULL; i++){
        if(!strcmp(flags[i], argv)) return i;
    }
    if(argv[0] == '+'){
        int s;
        if(sscanf(argv, "%d", &s) == 1) return s;
    }
    return -1;
}

void print_error(const char* error){
    printf("%s\n", error);
    exit(1);
}

int main(int argc, char** argv){
    
    int flag_kakoyto;
    int fl[] = {0,0,0};
    for(int i = 2; i < argc;i++){
        if((flag_kakoyto = check_argv(argv[i])) < 0)
            print_error("Flags error");
        if(flag_kakoyto > 2)
            if(fl[2] != 0) print_error("Plus Flag error");
            else fl[2] = flag_kakoyto;

        else
            fl[flag_kakoyto] = 1;
    }
    FILE *f;
    if((f = fopen(argv[1], "r")) == NULL) print_error("Open file error");

    char fsort[max_line][max_len];
    int size;
    for(size = 0; fgets(fsort[size], max_len, f); size++)
        if(size >= max_line - 1){
            fclose(f);
            print_error("Lines out of range");
        }

    fl[0] = 1 - 2 * fl[0];
    char t[max_len];



    char flagsort[max_line][max_len];
    if(fl[1]){
        for(int i = 0; i < size; i++)
            for(int j = 0;j < max_len && flagsort[i][j] != '\n'; j++){
                flagsort[i][j] = fsort[i][j];
                if(isupper(flagsort[i][j])) flagsort[i][j] = tolower(flagsort[i][j]);
            }

  }

    }




    for(int i = fl[2]; i<size; i++){
        for(int j = i; j<size; j++){
            if((fl[1] && (strcmp(flagsort[i], flagsort[j]) * fl[0]) > 0) || (!fl[1] &&  (strcmp(fsort[i], fsort[j]) * fl[0]) > 0)){
                strcpy(t, fsort[i]);
                strcpy(fsort[i], fsort[j]);
                strcpy(fsort[j], t);
                if(fl[1]){
                    strcpy(t, flagsort[i]);
                    strcpy(flagsort[i], flagsort[j]);
                    strcpy(flagsort[j], flagsort[i]);
                }
            }
        }
    }

    fclose(f);
    if((f = fopen(argv[1], "w")) == NULL) print_error("Open for write file error");
    for(int i = 0; i < size; i++) fputs(fsort[i], f);
    fclose(f);
    return 0;

}
