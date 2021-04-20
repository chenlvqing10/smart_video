#ifndef __LINK__
#define __LINK__

struct Node{
	char music_name[64];
	struct Node* next;
	struct Node* prior;
};

typedef struct Node Node;

int InitLink();
int InsertLink(Node* head,const char* name);
void FindNextMusic(const char* cur_name,int mode,char* next_name);
void ToPriorMusic(const char* cur_name,int mode,char* prior_name);
void ToNextMusic(const char* cur_name,int mode,char* next_name);
#endif
