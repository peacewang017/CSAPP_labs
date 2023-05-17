#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cachelab.h"

int T = 0;
int verbose = 0;
int s , b , S , B , E;    //S:组数 B:每行字节数 E:每组行数
typedef struct line{
    int t;
    __uint64_t tag;
}* group_node;
group_node* cache;    //全局变量cache
unsigned int HIT = 0, MISS = 0, EVICTION = 0;//result
char message[3][10]={"hit ", "miss ", "eviction "};

void init(int S , int E);
void print_cache(int S , int E);
int find_cache(__uint64_t tag , int group_id);
FILE *opt(int argc, char **argv);

int main(int argc ,char** argv)
{
    FILE* tracefile = opt(argc , argv);
    init(S , E);
    char op[2];
    __uint64_t address;
    int size;
    while(fscanf(tracefile , "%s %lx,%d\n",op,&address,&size)==3){
        if(op[0]=='I'){
            continue;
        }
        ++T;
        int group_id = (address >> b)& ~(~0u<<s);
        __uint64_t block_tag = address >> (b+s);
        int result = -1;
        result = find_cache(block_tag , group_id);
        if(op[0]=='M'){
            result = find_cache(block_tag , group_id);
        }
        if(verbose==1){
            fprintf(stdout,"%s %lx,%d %s\n",op,address,size,message[result]);
        }

    }
    printSummary(HIT , MISS , EVICTION);
    return 0;
}

void init(int S , int E)
{
    cache = (group_node*)malloc(sizeof(group_node)*S);
    for(int i=0;i<S;i++){
        cache[i] = (group_node)malloc(sizeof(struct line)*E);
        for(int j=0;j<E;j++){
            cache[i][j].t = 0;
        }
    }
}

void print_cache(int S  , int E){
    for(int i=0;i<S;i++){
        printf("S:%d\n",i);
        for(int j=0;j<E;j++){
            printf("E:%d t:%d tag:%ld\n",j,cache[i][j].t,cache[i][j].tag);
        }
    }
}

int find_cache(__uint64_t tag , int group_id)
{
    group_node the_group = cache[group_id];
    struct line the_line;
    int LRU_pos = 0;
    int is_empty = -1;
    for(int i=0;i<E;i++){
        the_line = the_group[i];
        if(the_line.t==0){
            is_empty = i;
        }
        else if(the_line.tag == tag){
            HIT++;
            return 0;
        }
        else if(the_line.t < the_group[LRU_pos].t){
            LRU_pos = i;
        }
    }
    MISS++;
    if(is_empty != -1){
        the_group[is_empty].t = T;
        the_group[is_empty].tag = tag;
        return 1;
    }else{
        the_group[LRU_pos].t = T;
        the_group[LRU_pos].tag = tag;
        EVICTION++;
        return 2;
    }
}

FILE *opt(int argc, char **argv)
{
    FILE *tracefile;
    for (int c; (c = getopt(argc, argv, "hvsEbt")) != EOF;) {
        switch (c) {
            case 'h':
                exit(1);
                break;
            case 'v': 
                verbose = 1;
                break;
            case 't':
                tracefile = fopen(argv[optind], "r");
                if (tracefile == NULL) exit(1);
                break;
            case 's':
                s = atoi(argv[optind]);
                if (s <= 0) exit(1);
                S = 1 << s;
                break;
            case 'E': 
                E = atoi(argv[optind]);
                if (E <= 0) exit(1);
                break;
            case 'b':
                b = atoi(argv[optind]);
                if (b <= 0) exit(1);
                B = 1 << b;
                break;
        }
    }
    return tracefile;
}
