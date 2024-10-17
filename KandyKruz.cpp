#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <conio.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <ctype.h>

#define NORM "\033[0m"
#define IJO "\033[0;32m"
#define RED "\033[0;31m"
#define BIRU "\e[1;94m"
#define YELL "\e[1;93m"
#define PURP "\e[1;95m"
#define MERAH "\e[1;91m"
#define CYAN "\e[1;96m"
#define MIDORI "\e[1;32m"
#define MURASAKI "\e[45m"
#define KIIROI "\e[0;33m"

#define HASH_SIZE 36
#define MAPS 6
#define CANDIES 4

char user[100];

//Menu
int welcome(int pick);
void enPass(char* pass);
void hof(int pick);
void login(int cor);
void reg(int cor);
void userMenu(int pick);

//Game
char map[MAPS][MAPS][4];
const char candyType[4][4] = {"[1]", "[2]", "[3]", "[4]"};
const char* candyCol[4] = {BIRU, MERAH, PURP, YELL};
int x, y, x1, x2, y1, y2, lock1, lock2;
int startplay;
int delay;

int moves, score;

void startMap(char grid[MAPS][MAPS][4]); 
void prinMap(char grid[MAPS][MAPS][4]);
void changePos();
void swapCandy(char map[MAPS][MAPS][4], int x1, int x2, int y1, int y2);
int match(char map[MAPS][MAPS][4]);
void removeMatched(char map[MAPS][MAPS][4]);
void turuninNew(char map[MAPS][MAPS][4]);
void endScreen(char map[MAPS][MAPS][4]);

//Hash File
typedef struct Players{
	char usn[100];
	char pass[100];
	int high_score;
	Players *next;
}Players;

Players* hashTable[HASH_SIZE];

unsigned int hash(const char* usn){
	unsigned int hashV = 0;
	while(*usn){
		hashV += *usn++;
	}
	return hashV % HASH_SIZE;
}

void insertUser(const char *usn, const char *pass, int score){
	unsigned int index = hash(usn);
	
	Players* newUser = (Players*)malloc(sizeof(Players));
	
	strcpy(newUser->usn, usn);
	strcpy(newUser->pass, pass);
	newUser->high_score = score;
	newUser->next = NULL;
	
	if(hashTable[index]==NULL){
		hashTable[index]=newUser;	
		return;
	}
	
	Players* curr = hashTable[index];
	Players* prev = NULL;
	
	while(curr!=NULL && strcmp(curr->usn, usn)<0){
		prev=curr;
		curr=curr->next;
	}	
	if(prev==NULL){
		newUser->next=hashTable[index];
		hashTable[index] = newUser;
	}
	else{
		newUser->next=curr;
		prev->next = newUser;
	}
}

void readFile(){
	FILE *fp;
	fp=fopen("users.txt", "r");
	if(!fp){
		puts("Unable to open file");
		return;
	}
	
	char data_line[200];
	while(fgets(data_line, sizeof(data_line), fp)){
		char usn[100], pass[100];
		int score;
		sscanf(data_line, "%[^#]#%[^#]#%d", usn, pass, &score);	
		insertUser(usn, pass, score);
	}
	fclose(fp);
}

Players* searchUsn(const char* usn){
	unsigned int index= hash(usn);
	Players* curr = hashTable[index];
	
	while(curr!=NULL){
		if(strcmp(curr->usn, usn)==0){
			return curr;
		}
		curr=curr->next;
	}
	return NULL;
}

Players* regUsn(const char* usn){
	unsigned int index= hash(usn);
	Players* curr = hashTable[index];

	while(curr!=NULL){
		if(strcmp(curr->usn, usn)==0){
			reg(1);
		}
		curr=curr->next;
	}
	return NULL;
}

void prinHash(){
    for (int i = 0; i < HASH_SIZE; i++) {
        if (hashTable[i] != NULL) {
            printf("Index %d: \n", i);
            Players* curr = hashTable[i];
            while (curr != NULL) {
            	printf("Usn: %s\n", curr->usn);
            	printf("Pass: %s\n", curr->pass);
            	printf("HIgh Score: %d\n", curr->high_score);
            	puts("       |");
            	puts("       V");
          
                curr = curr->next;
            }
            printf("\n");
        }
    }
}

void freeHashTable() {
    for (int i = 0; i < HASH_SIZE; i++) {
        Players* curr = hashTable[i];
        while (curr != NULL) {
            curr = curr->next;
            free(curr); 
        }
        hashTable[i] = NULL; 
    }
}

void updateFile(){
	FILE *fp;
	fp=fopen("users.txt", "w");
	
	for(int i=0; i<HASH_SIZE; i++){
		Players *curr = hashTable[i];
		while(curr!=NULL){
			fprintf(fp, "%s#%s#%d\n", curr->usn, curr->pass, curr->high_score);
			curr=curr->next;
		}
	}
	
	fclose(fp);
}

void endGame(){
	Players* currp = searchUsn(user);
	if(currp != NULL){
		if(score>currp->high_score){
			currp->high_score = score;
			puts("That's a new High Score!");
			updateFile();
			return;
		}
	}
	printf("You are %d points off from your high score, %d points\n", currp->high_score-score, currp->high_score);
	printf("Please press Enter to continue...");
	getchar();
	userMenu(0);
}



void play(){
	int matches, checkx, checky;
	startplay=0;
	score=0; moves=15;
	x=0, y=0;
	x1=-1, y1=-1, x2=-1, y2=-1;
	lock1=0, lock2=0;
	delay=100;
	startMap(map);
	while(moves>0){
		do{
			matches = match(map);
			if(matches==1){
				removeMatched(map);
				turuninNew(map);
			}
		}while(matches==1);
		
		changePos();
		checkx=abs(x1-x2);
		checky=abs(y1-y2);
		if(lock1==1 && lock2==1){
			if((checkx==1 && y1==y2)||(checky==1 && x1==x2)){
				swapCandy(map, x1, x2, y1, y2);
				if(match(map)==1){
					moves--;
					removeMatched(map);
					turuninNew(map);			
				}
				else{
					swapCandy(map, x1, x2, y1, y2);
					getch();
				}
				lock1=lock2=0;
				x1=x2=y1=y2=-1;
			}
			else{
				lock1=lock2=0;
				x1=x2=y1=y2=-1;
			}
		}
	}
	if(moves<=0){
		endScreen(map);
		system("cls");
		printf(CYAN"GOOD JOB! You Achieved a Score of %d Points!\n", score);
		endGame();
		printf("Please press Enter to continue...");
		getchar();
		userMenu(0);
	}
}

void pause(int pick){
	system("cls");
	puts(NORM"Game Paused");
	if(pick==0) printf(IJO"Resume Game                            <<\n"NORM);
	else puts(NORM "Resume Game");
	if(pick==1)printf(IJO"Quit Game (Progress Will not be Saved) <<"NORM);
	else puts(NORM "Quit Game (Progress Will not be Saved)");
	
	switch(getch()){
		case 'w':
			if(pick==0) pause(0);
			if(pick>0) pause(0);
			break;
		case 's':
			if(pick==1) pause(1);
			if(pick<1) pause(1);
			break;
		case '\r':
			if(pick==0) return;
			else if(pick==1) {
				endScreen(map);
				userMenu(0);
			}
	}
}

void userMenu(int pick){
	system("cls");
	printf(NORM "Welcome to XYtrus, ");
	printf(CYAN "%s!\n", user);
    if(pick==0) printf(IJO "Play Game          <<\n");
    else printf(NORM "Play Game\n");   
    if(pick==1) printf(IJO "Hall of Fame       <<\n");
    else printf(NORM "Hall of Fame\n"); 
    if(pick==2) printf(IJO "Log Out            <<\n");
    else printf(NORM "Log Out\n");  
    if(pick==3) printf(IJO "Exit Game          <<\n");
    else printf(NORM "Exit Game\n"); 

	switch(getch()){
	    case 'w':
	        if(pick>0) pick-=1;
	        userMenu(pick);
	    case 's':
	        if(pick<3) pick+=1;
	        userMenu(pick);
	    case '\r':
	    	if(pick==0) play();
	    	else if(pick==1) hof(1);
	    	else if(pick==2) welcome(0);
	    	else if(pick==3) exit(0);
	}
	
}

void verify(char* usn, char* pass){
	Players* userNode = searchUsn(usn);
	if(userNode != NULL && strcmp(userNode->pass, pass)==0){
		strcpy(user, usn);
		userMenu(0);
	}
	else{
		login(1);
	}
}

void login(int cor){
	system("cls");
	
	char ch; 
	int usn_len=0, pass_len=0;
	char usn[100]; char pass[100];
	
	puts("Please Fill In Your Credentials");
	printf("Input your username: \n");
	printf(NORM "Password: \n");
	if(cor==0)printf("\n");
	else if(cor==1) printf(RED "Invalid credential, please try logging in again.\n" NORM);
	else if(cor==2) printf(RED "Both fields must be filled.\n" NORM);
	printf("Press esc to go back");
	printf("\033[3F"); printf("\033[22C");
	while(1) {
        ch = getch(); 
        if (ch == 27) { 
            welcome(0);
            return;
        } 
        else if(ch == '\r'&& usn_len<1) login(2);
		else if (ch == '\r') {
            usn[usn_len] = '\0'; 
            break;
        } 
		else if (ch == 8) { 
            if (usn_len > 0) {
                usn_len--;
                printf("\b \b"); 
            }
        } 
		else {
            usn[usn_len++] = ch;  
            printf("%c", ch); 
        }
    }
	printf("\n\033[10C");

	while(1) {
        ch = getch(); 
        if (ch == 27) { 
            welcome(0);
            return;
        } 
        else if(ch == '\r'&& pass_len<1) login(2);
		else if (ch == '\r') {  
            pass[pass_len] = '\0';
            break;
        } 
		else if (ch == 8) { 
            if (pass_len > 0) {
                pass_len--;
                printf("\b \b"); 
            }
        } 
		else {
            pass[pass_len++] = ch;
            printf("*"); 
        }
    }
	enPass(pass);
	verify(usn, pass);
}

void enPass(char* pass){
	int len = strlen(pass);

	for(int i=0; i<len; i++){
		pass[i] = pass[i]+10;
		if(pass[i]>122) pass[i] = pass[i]-26;
	}
	
}

void reg(int cor){
	system("cls");
	char ch; int usn_len=0, pass_len=0;
	char usn[100]; char pass[100];
	
	puts("Register New Account, username only starts with alphabets");
	printf("Input your username: \n");
	printf("Password: \n");
	if(cor==0)printf("\n");
	else if(cor==1) printf(RED "User already exist, please try another.\n" NORM);
	else if(cor==2) printf(RED "Both fields must be filled.\n" NORM);
	else if(cor==3) printf(RED "Username can only start with alphabets.\n" NORM);
	printf("Press esc to go back");
	printf("\033[3F"); printf("\033[22C");
	while(1) {
        ch = getch(); 
        if (ch == 27) { 
            welcome(0);
            return;
        } 
        else if (ch == 8) { 
            if (usn_len > 0) {
                usn_len--;
                printf("\b \b"); 
            }
        } 
        else if(ch == '\r'&& usn_len<1) reg(2);
		else if (ch == '\r') {
			if(!isalpha(usn[0]))reg(3);
            usn[usn_len] = '\0'; 
            break;
        }
		else {
            usn[usn_len++] = ch;  
            printf("%c", ch); 
        }
    }
    regUsn(usn);

	printf("\n\033[10C");
	while(1) {
        ch = getch(); 
        
        if (ch == 27) { 
            welcome(0);
            return;
        } 
        else if(ch == '\r'&& usn_len<1) reg(2);
		else if (ch == '\r') {  
            pass[pass_len] = '\0';
            break;
        } 
		else if (ch == 8) { 
            if (pass_len > 0) {
                pass_len--;
                printf("\b \b"); 
            }
        } 
		else {
            pass[pass_len++] = ch;
            printf("*"); 
        }
    }	
	enPass(pass);
	insertUser(usn, pass, 0);
	
	FILE *fp;
	fp = fopen("users.txt", "a");
	if(fp==NULL) fp = fopen("users.txt", "w");
	fprintf(fp, "%s#%s#%d\n", usn, pass, 0);
	fclose(fp);
	
	welcome(0);
}

int sortScore(const void* a, const void* b){
	Players* pa = *(Players**)a;
	Players* pb = *(Players**)b;
	return pb->high_score - pa->high_score;
}

void hof(int pick){
	Players* allp[1000];
	int count=0, ada=0, page=0, items=10;
	
	for(int i=0; i<HASH_SIZE; i++){
		Players* curr=hashTable[i];
		while(curr!= NULL){
			ada = 1;
			if(count < 1000){
				allp[count] = curr;
				count++;
			}
			curr=curr->next;
		}
	}
	
	qsort(allp, count, sizeof(Players*), sortScore);
	
	char input;
	
	do{
		system("cls");
		for(int i=0; i<6; i++){
			printf(BIRU"%c", 205);
		}
		printf("Hall of Fame");
		for(int i=0; i<6; i++){
			printf("%c", 205);
		}
		printf(NORM"\n");
		printf(CYAN"%c", 201);
		for(int i=0; i<16; i++){
			printf("%c", 205);
		}
		printf("%c", 203);
		for(int i=0; i<12; i++){
			printf("%c", 205);
		}
		printf("%c\n", 187);
		printf(CYAN"%c"NORM" Username       "CYAN"%c"NORM" High Score "CYAN"%c\n", 186, 186, 186);
		printf("%c", 204);
		for(int i=0; i<16; i++){
			printf("%c", 205);
		}
		printf("%c", 206);
		for(int i=0; i<12; i++){
			printf("%c", 205);
		}
		printf("%c\n", 185);
		
		int start = page*items;
		int end = start+items;
		if(end>count) end=count;
		
		for (int i = 0; i < count; i++) {
			Players* player = allp[i];
	        printf(CYAN"%c"NORM" %-15s"CYAN"%c", 186, player->usn, 186);
	        if(player->high_score==0)printf(NORM" N/A        "CYAN"%c", 186);
	        else printf(NORM" %-11d"CYAN"%c", player->high_score, 186);	
	        printf("\n");
	    }
	    printf(CYAN"%c", 200);
		for(int i=0; i<16; i++){
			printf("%c", 205);
		}
		printf("%c", 202);
		for(int i=0; i<12; i++){
			printf("%c", 205);
		}
		printf("%c\n", 188);
		
	    if(ada==0)puts(NORM "No User Data Currently");
	    else{
	    	printf(NORM"<A        Page %d of %d        D>\n", page+1, (count+items-1)/items);
	    	printf("Press Enter to Exit");
			input = getch();
		}	
		if (input=='d' && (page + 1) * items<count) page++; 
		else if (input=='a' && page>0)page--;
        else if(input=='\r') break;
		
	}while(input != 'r');
	
	if(pick==0)welcome(0);
	else userMenu(0);
	
}


int welcome(int pick){
    system("cls");
    puts(NORM "Welcome to "PURP"KandyKruz!");
    if(pick==0) printf(IJO "Login              <<\n");
    else printf(NORM "Login\n");   
    if(pick==1) printf(IJO "Register           <<\n");
    else printf(NORM "Register\n"); 
    if(pick==2) printf(IJO "Hall of Fame       <<\n");
    else printf(NORM "Hall of Fame\n");  
    if(pick==3) printf(IJO "Exit Game          <<\n");
    else printf(NORM "Exit Game\n"); 

	switch(getch()){
	    case 'w':
	        if(pick>0) pick-=1;
	        welcome(pick);
	    case 's':
	        if(pick<3) pick+=1;
	        welcome(pick);
	    case '\r':
	    	if(pick==0) login(0);
	    	else if(pick==1) reg(0);
	    	else if(pick==2) hof(0);
	    	else if(pick==3) exit(0);
	}
}

int main(){
	srand(time(0));
	readFile();		
	welcome(0);
	//prinHash();
    
    return 0;
}

void startMap(char map[MAPS][MAPS][4]){
	for(int i=0; i<MAPS; i++){
		for(int j=0; j<MAPS; j++){
			strcpy(map[i][j], candyType[rand()%4]);
		}
	}
}

void prinMap(char map[MAPS][MAPS][4], int x, int y, int x1, int y1, int x2, int y2, int lock1, int lock2){
	system("cls");
	for(int i=0; i<MAPS; i++){
		for(int j=0; j<MAPS; j++){
			if(strcmp(map[i][j], candyType[0])==0) printf(candyCol[0]);
			else if(strcmp(map[i][j], candyType[1])==0) printf(candyCol[1]);
			else if(strcmp(map[i][j], candyType[2])==0) printf(candyCol[2]);
			else if(strcmp(map[i][j], candyType[3])==0) printf(candyCol[3]);
			else if(strcmp(map[i][j], "[0]")==0) printf(MURASAKI);
			if(x==j && y==i)printf(MIDORI ":");
			else if(x1==j && y1==i&&lock1==1)printf(KIIROI "{");
			else if(x2==j && y2==i&&lock2==1)printf(KIIROI "{");
			else printf(" ");
			printf("%s", map[i][j]);
			if(x==j && y==i)printf(":");
			else if(x1==j && y1==i&&lock1==1)printf("}");
			else if(x2==j && y2==i&&lock2==1)printf("}");
			else printf(" ");
			printf(NORM);
		}
		printf("\n");
	}
	printf("Current Score : %d\n", score);
	printf("Moves Left    : %d\n", moves);
	printf("Press enter to lock or cancel, w a s d to select tiles, enter again on another tile to swap");
	return;
}

void endScreen(char map[MAPS][MAPS][4]){
    for(int i=0; i<MAPS; i++){
        for(int j=MAPS-1; j>=0; j--){
            strcpy(map[i][j], "[0]"); 
            prinMap(map, x, y, x1, y1, x2, y2, lock1, lock2); 
            Sleep(100); 
        }
    }
}

void changePos(){
	prinMap(map, x, y, x1, y1, x2, y2, lock1, lock2);
	switch(getch()){
		case 'w':
			if(y>0) y--;
			break;
		case 'a':
			if(x>0) x--;
			break;
		case 's':
			if(y<MAPS-1) y++;
			break;
		case 'd':
			if(x<MAPS-1) x++;
			break;	
		case 27:
			pause(0);
		case '\r':
			if(x1<0&&y1<0){
				x1=x; y1=y;
				lock1=1;
				startplay=1;
			}
			else if(x1==x && y1==y) {
				x1=-1; y1=-1;
				lock1=0;
			}
			else if(x1>=0&&y1>=0&&x2<0&&y2<0){
				x2=x; y2=y;
				lock2=1;	
			}
	}	
}

void swapCandy(char map[MAPS][MAPS][4], int x1, int x2, int y1, int y2){
	char temp[4];
	//printf("Swap %d %d , %d %d\n", x1, y1, x2, y2);
	strcpy(temp, map[y1][x1]);
	strcpy(map[y1][x1], map[y2][x2]);
	strcpy(map[y2][x2], temp);
	
	prinMap(map, x, y, x1, y1, x2, y2, lock1, lock2);
}

int match(char map[MAPS][MAPS][4]){ 
	int matched = 0;
	for(int i=0; i<MAPS; i++){
		for(int j=0; j<MAPS; j++){
			//Match 5
			if(j<MAPS-4 && strcmp(map[i][j], map[i][j+1])==0 && strcmp(map[i][j], map[i][j+2])==0 && strcmp(map[i][j], map[i][j+3])==0 && strcmp(map[i][j], map[i][j+4])==0){
				matched=1;
				if(startplay != 0){
					score += 500;
					moves +=3;
				}
			}
			if(i<MAPS-4 && strcmp(map[i][j], map[i+1][j])==0 && strcmp(map[i][j], map[i+2][j])==0 && strcmp(map[i][j], map[i+3][j])==0 && strcmp(map[i][j], map[i+4][j])==0){
				matched=1;
				if(startplay != 0){
					score += 500;
					moves +=3;
				}
			}
			//Match L
			if(i<MAPS-2 && j<MAPS-2 && strcmp(map[i][j], map[i+1][j])==0 && strcmp(map[i][j],map[i+2][j])==0 && strcmp(map[i][j],map[i][j+1])==0 && strcmp(map[i][j], map[i][j+2])==0){
				matched = 1;
				if(startplay != 0){
					score += 500;
				}
			}
			if(i>=2 && j<MAPS-2 && strcmp(map[i][j], map[i-1][j])==0 && strcmp(map[i][j],map[i-2][j])==0 && strcmp(map[i][j],map[i][j+1])==0 && strcmp(map[i][j], map[i][j+2])==0){
				matched = 1;
				if(startplay != 0){
					score += 500;
				}
			}
			if(i>=2 && j>=2 && strcmp(map[i][j], map[i-1][j])==0 && strcmp(map[i][j],map[i-2][j])==0 && strcmp(map[i][j],map[i][j-1])==0 && strcmp(map[i][j], map[i][j-2])==0){
				matched = 1;
				if(startplay != 0){
					score += 500;
				}
			}
			if(i<MAPS-2 && j>=2 && strcmp(map[i][j], map[i+1][j])==0 && strcmp(map[i][j],map[i+2][j])==0 && strcmp(map[i][j],map[i][j-1])==0 && strcmp(map[i][j], map[i][j-2])==0){
				matched = 1;
				if(startplay != 0){
					score += 500;
				}
			}
			//Match T
			if(i<MAPS-2 && j>0 && j<MAPS-1 && strcmp(map[i][j], map[i+1][j])==0 && strcmp(map[i][j],map[i+2][j])==0 && strcmp(map[i][j],map[i+1][j-1])==0 && strcmp(map[i][j], map[i+1][j+1])==0){
				matched = 1;
				if(startplay != 0){
					score += 500;
				}
			}
			if(i>=2 && j>0 && j<MAPS-1 && strcmp(map[i][j], map[i-1][j])==0 && strcmp(map[i][j],map[i-2][j])==0 && strcmp(map[i][j],map[i-1][j-1])==0 && strcmp(map[i][j], map[i-1][j+1])==0){
				matched = 1;
				if(startplay != 0){
					score += 500;
				}
			}
			if(j<MAPS-2 && i>0 && i<MAPS-1 && strcmp(map[i][j], map[i][j+1])==0 && strcmp(map[i][j],map[i][j+2])==0 && strcmp(map[i][j],map[i-1][j+1])==0 && strcmp(map[i][j], map[i+1][j+1])==0){
				matched = 1;
				if(startplay != 0){
					score += 500;
				}
			}
			if(j>=2 && i>0 && i<MAPS-1 && strcmp(map[i][j], map[i][j-1])==0 && strcmp(map[i][j],map[i][j-2])==0 && strcmp(map[i][j],map[i-1][j-1])==0 && strcmp(map[i][j], map[i+1][j-1])==0){
				matched = 1;
				if(startplay != 0){
					score += 500;
				}
			}
			//Match 4
			if(j<MAPS-3 && strcmp(map[i][j], map[i][j+1])==0 && strcmp(map[i][j], map[i][j+2])==0 && strcmp(map[i][j], map[i][j+3])==0){
				matched=1;
				if(startplay != 0){
					score += 400;
					moves +=1;
				}
			}
			if(i<MAPS-3 && strcmp(map[i][j], map[i+1][j])==0 && strcmp(map[i][j], map[i+2][j])==0 && strcmp(map[i][j], map[i+3][j])==0){
				matched=1;
				if(startplay != 0){
					score += 400;
					moves +=1;
				}
			}
			//Match 3
			if(j<MAPS-2 && strcmp(map[i][j], map[i][j+1])==0 && strcmp(map[i][j], map[i][j+2])==0){
				matched=1;
				if(startplay != 0){
					score += 300;
				}
			}
			if(i<MAPS-2 && strcmp(map[i][j], map[i+1][j])==0 && strcmp(map[i][j], map[i+2][j])==0){
				matched=1;
				if(startplay != 0){
					score += 300;
				}
			}
			
		}
	}
	return matched;
}

void bomb(char map[MAPS][MAPS][4], int rem[MAPS][MAPS], int i, int j) {
    for (int row = i - 1; row <= i + 1; row++) {
        for (int col = j - 1; col <= j + 1; col++) {
            if (row >= 0 && row < MAPS && col >= 0 && col < MAPS) {
                if (strcmp(map[row][col], "[ ]") != 0) rem[row][col] = 1; 
            }
        }
    }
}

void removeMatched(char map[MAPS][MAPS][4]){
	int remc = 0;
	int rem[MAPS][MAPS] = {0};
	
	for(int i=0; i<MAPS; i++){
		for(int j=0; j<MAPS; j++){
			//Match 5
			if(j<MAPS-4 && strcmp(map[i][j], map[i][j+1])==0 && strcmp(map[i][j], map[i][j+2])==0 && strcmp(map[i][j], map[i][j+3])==0 && strcmp(map[i][j], map[i][j+4])==0){
				for(int k=0; k<5; k++){
					rem[i][j+k]=1;
				}
				remc++;
			}
			else if(i<MAPS-4 && strcmp(map[i][j], map[i+1][j])==0 && strcmp(map[i][j], map[i+2][j])==0 && strcmp(map[i][j], map[i+3][j])==0 && strcmp(map[i][j], map[i+4][j])==0){
				for(int k=0; k<5; k++){
					rem[i+k][j]=1;
				}
				remc++;
			}
			//Match L
			else if(i<MAPS-2 && j<MAPS-2 && strcmp(map[i][j], map[i+1][j])==0 && strcmp(map[i][j],map[i+2][j])==0 && strcmp(map[i][j],map[i][j+1])==0 && strcmp(map[i][j], map[i][j+2])==0){
				rem[i][j]=1;
				rem[i+1][j]=1;
				rem[i+2][j]=1;
				rem[i][j+1]=1;
				rem[i][j+2]=1;
				bomb(map, rem, i, j);
				
				remc++;
			}
			else if(i>=2 && j<MAPS-2 && strcmp(map[i][j], map[i-1][j])==0 && strcmp(map[i][j],map[i-2][j])==0 && strcmp(map[i][j],map[i][j+1])==0 && strcmp(map[i][j], map[i][j+2])==0){
				rem[i][j]=1;
				rem[i-1][j]=1;
				rem[i-2][j]=1;
				rem[i][j+1]=1;
				rem[i][j+2]=1;
				bomb(map, rem, i, j);
	
				remc++;
			}
			else if(i>=2 && j>=2 && strcmp(map[i][j], map[i-1][j])==0 && strcmp(map[i][j],map[i-2][j])==0 && strcmp(map[i][j],map[i][j-1])==0 && strcmp(map[i][j], map[i][j-2])==0){
				rem[i][j]=1;
				rem[i-1][j]=1;
				rem[i-2][j]=1;
				rem[i][j-1]=1;
				rem[i][j-2]=1;
				bomb(map, rem, i, j);

				remc++;
			}
			else if(i<MAPS-2 && j>=2 && strcmp(map[i][j], map[i+1][j])==0 && strcmp(map[i][j],map[i+2][j])==0 && strcmp(map[i][j],map[i][j-1])==0 && strcmp(map[i][j], map[i][j-2])==0){
				rem[i][j]=1;
				rem[i+1][j]=1;
				rem[i+2][j]=1;
				rem[i][j-1]=1;
				rem[i][j-2]=1;
				bomb(map, rem, i, j);

				remc++;
			}
			//Match T
			else if(i<MAPS-2 && j>0 && j<MAPS-1 && strcmp(map[i][j], map[i+1][j])==0 && strcmp(map[i][j],map[i+2][j])==0 && strcmp(map[i][j],map[i+1][j-1])==0 && strcmp(map[i][j], map[i+1][j+1])==0){
				rem[i][j]=1;
				rem[i+1][j]=1;
				rem[i+2][j]=1;
				rem[i+1][j-1]=1;
				rem[i+1][j+1]=1;
				bomb(map, rem, i, j);

				remc++;
			}
			else if(i>=2 && j>0 && j<MAPS-1 && strcmp(map[i][j], map[i-1][j])==0 && strcmp(map[i][j],map[i-2][j])==0 && strcmp(map[i][j],map[i-1][j-1])==0 && strcmp(map[i][j], map[i-1][j+1])==0){
				rem[i][j]=1;
				rem[i-1][j]=1;
				rem[i-2][j]=1;
				rem[i-1][j-1]=1;
				rem[i-1][j+1]=1;
				bomb(map, rem, i, j);

				remc++;
			}
			else if(j<MAPS-2 && i>0 && i<MAPS-1 && strcmp(map[i][j], map[i][j+1])==0 && strcmp(map[i][j],map[i][j+2])==0 && strcmp(map[i][j],map[i-1][j+1])==0 && strcmp(map[i][j], map[i+1][j+1])==0){
				rem[i][j]=1;
				rem[i][j+1]=1;
				rem[i][j+2]=1;
				rem[i-1][j+1]=1;
				rem[i+1][j+1]=1;
				bomb(map, rem, i, j);

				remc++;
			}
			else if(j>=2 && i>0 && i<MAPS-1 && strcmp(map[i][j], map[i][j-1])==0 && strcmp(map[i][j],map[i][j-2])==0 && strcmp(map[i][j],map[i-1][j-1])==0 && strcmp(map[i][j], map[i+1][j-1])==0){
				rem[i][j]=1;
				rem[i][j-1]=1;
				rem[i][j-2]=1;
				rem[i-1][j-1]=1;
				rem[i+1][j-1]=1;
				bomb(map, rem, i, j);

				remc++;
			}
			//Match 4
			else if(j<MAPS-3 && strcmp(map[i][j], map[i][j+1])==0 && strcmp(map[i][j], map[i][j+2])==0 && strcmp(map[i][j], map[i][j+3])==0){
				for(int k=0; k<4; k++){
					rem[i][j+k]=1;	
				}
				remc++;
			}
			else if(j<MAPS-3 && strcmp(map[i][j], map[i][j+1])==0 && strcmp(map[i][j], map[i][j+2])==0 && strcmp(map[i][j], map[i][j+3])==0){
				for(int k=0; k<3; k++){
					rem[i][j+k]=1;	
				}
				remc++;
			}
			else if(i<MAPS-3 && strcmp(map[i][j], map[i+1][j])==0 && strcmp(map[i][j], map[i+2][j])==0 && strcmp(map[i][j], map[i+3][j])==0){
				for(int k=0; k<4; k++){
					rem[i+k][j]=1;
				}
				remc++;
			}
			//Match 3
			else if(j<MAPS-2 && strcmp(map[i][j], map[i][j+1])==0 && strcmp(map[i][j], map[i][j+2])==0){
				for(int k=0; k<3; k++){
					rem[i][j+k]=1;
				}
				remc++;
			}
			else if(i<MAPS-2 && strcmp(map[i][j], map[i+1][j])==0 && strcmp(map[i][j], map[i+2][j])==0){
				for(int k=0; k<3; k++){
					rem[i+k][j]=1;
				}
				remc++;
			}
		}
		
		if(remc>0){
			for(int i=0; i<MAPS; i++){
				for(int j=0; j<MAPS; j++){
					if(rem[i][j]==1) strcpy(map[i][j], "[ ]");
				}
			}
		}
	}
	
}

void turuninNew(char map[MAPS][MAPS][4]){
	int hasmove;
	do{
		hasmove=0;
		for(int i=0; i<MAPS; i++){
			for(int j=MAPS-1; j>0; j--){ 	
				if(strcmp(map[j][i], "[ ]")==0){
					int k=j-1;
					while(k>=0 && strcmp(map[k][i], "[ ]")==0) k--;
					if(k>=0){
						strcpy(map[j][i], map[k][i]);
						strcpy(map[k][i], "[ ]");
						hasmove=1;
						if(startplay!=0){
							prinMap(map, x, y, x1, y1, x2, y2, lock1, lock2);
							Sleep(delay);		
						}	
					}	
				}	
			}
		}
	}while (hasmove == 1);

	for(int i=0; i<MAPS; i++){
		for(int j=0; j<MAPS; j++){
			if(strcmp(map[j][i], "[ ]")==0){
				strcpy(map[j][i], candyType[rand()%4]);
				if(startplay!=0){
					prinMap(map, x, y, x1, y1, x2, y2, lock1, lock2);
					Sleep(delay);		
				}
			}
		}
	} 
}
