#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

struct Block{
	int old;
	int valid;
	unsigned long tag;
};


int main(int argc,char *argv[])
{
	int i;
	FILE *fp = NULL;
	char *trace = NULL;
	struct Block **cache;

	int S = 0;
	int E = 0;
	int B = 0;

	int v_check = 0;

	int hit_count = 0;
	int miss_count = 0;
	int eviction_count = 0;
	int c;
	while( (c = getopt(argc,argv,"vs:E:b:t:") ) != -1){
		switch(c){
			case 'v':
				v_check = 1;
				break;
			case 's':
				S = atoi(optarg);
				break;
			case 'E':
				E = atoi(optarg);
				break;
			case 'b':
				B = atoi(optarg);
				break;
			case 't':
				trace = optarg;
				break;
		}
	}

	cache = calloc(sizeof(struct Block*),(1<<S));
	for(i=0;i<(1<<S);i++){
		cache[i] = calloc(sizeof(struct Block),E);
	}
	fp = fopen(trace,"r");
	while(!feof(fp)){
		char op;
		unsigned long address,size;
		unsigned long set,tag;
		fscanf(fp,"%c %lx,%lx",&op,&address,&size);
		if(op == 'S' || op == 'L' || op == 'M'){
			set = (address>>B) & ((1<<S)-1);
			tag = address>>(S+B);
			int count = 0;
			for(i=0;i<E;i++){
				if(cache[set][i].valid){
					count++;
					if(cache[set][i].tag == tag){
						break;
					}
				}
			}
			int block = i;
			if(v_check){
				printf("%c,%lx,%lx ",op,address,size);
			}
			if(block<E){
				hit_count++;
				if(v_check){
					printf("hit ");
				}
				cache[set][block].old = 0;
			}
			else{
				miss_count++;
				if(v_check){
					printf("miss ");
				}
				if(count < E){
					cache[set][count].old = 0;
					cache[set][count].valid = 1;
					cache[set][count].tag = tag;
				}
				else{
					int max = 0;
					eviction_count++;
					if(v_check){
						printf("eviction ");
					}
					for(i=0;i<E;i++){
						if(cache[set][max].old < cache[set][i].old) max = i;
					}
					cache[set][max].old = 0;
					cache[set][max].valid = 1;
					cache[set][max].tag = tag;
				}
			}
			if(op == 'M'){
				hit_count++;
				if(v_check){
					printf("hit ");
				}
			}
			for(i=0;i<E;i++){
				if(cache[set][i].valid)
					cache[set][i].old++;
			}
			if(v_check){
				printf("\n");
			}
		}
	}

	printSummary(hit_count, miss_count, eviction_count);
	return 0;
}
