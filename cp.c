#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void print_error(const char* s){
    printf("%s\n", s);
    exit(993);
}

int main(int argc, char **argv){
    FILE *f1, *f2;
    if(argc != 3) print_error("argumets error");
    if((f1 = fopen(argv[1], "r")) == NULL) print_error("file 1 open error");
    if((f2 = fopen(argv[2], "w")) == NULL) print_error("file 2 open error");
    char buff[1024];
    while(!feof(f1)){
        if(fgets(buff, 1024, f1))
            if(fputs(buff, f2) == EOF) print_error("fputs error");
    }
    fclose(f1);
    fclose(f2);
    return 0;
}