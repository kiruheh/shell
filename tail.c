#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>


void print_error(const char * error){
    printf("%s\n",error);
    exit(25);
}

int main(int argc, char **argv){
    int n, minus = 1;
   
    if(argc > 3) print_error("Flags error 1");
    if(argc < 3) n = 10;
    else{
        //if(argv[2][0] == '-') minus = -1;
        if(argv[2][0] != '+' && argv[2][0] != '-') print_error("Flags error 2");
        if(sscanf(argv[2], "%d", &n) != 1) print_error("Flags error 3");
        //n*= minus;
    }

    FILE* f;
    if((f = fopen(argv[1], "r")) == NULL) print_error("File open error");
    char buff[1024];
    int i = 0;
    
    if(n<0){
        while(!feof(f)){
            if(fgets(buff, 1024, f) && (++i >= -n))
                printf("%s", buff);
        }
    }
    else{
        int lines = 0;
        while(fgets(buff, 1024, f)) lines++;
        fseek(f, SEEK_SET, 0);
        while(!feof(f)){
            if(fgets(buff, 1024, f) && (++i > lines - n))
                printf("%s", buff);
        }
    }
    fclose(f);
    return 0;
}