#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<ncurses.h>
#include <dirent.h>
#include <errno.h>
#include <sys/wait.h>
#include"dsh.h"
#define WIN_HEIGHT 40
#define WIN_WIDTH 65
#define WIN1_START_X 10
#define WIN2_START_X (WIN1_START_X + WIN_WIDTH + 20)
#define WIN_START_Y 10
void catFile(int session, int outputNum){

    //command from pdf:
    //screen -X clear; screen -X exec cat .dsh/4/3.stdout
    int pid = fork();
    if(pid == 0){ //we have to fork exec the first part of the command from the pdf
        char *args[] = {"screen", "-X", "clear", 0}; //args terminated by null char
        char path[1024];
        strncpy(path, getenv("PATH"), 1024);
        //printf("pathinfindpath:%s\n", path);
        for ( char *dir = strtok( path, ":" ); dir; dir = strtok( NULL, ":" ) ) { //tokenize the path by :
            char s[100];
            snprintf(s,100,"%s/%s",dir,"screen"); //for every path token, build a path string ex: path+/+screen  
            execv(s, args);
        }
    }
    wait(0); //wait for child to exec

    int pid2 = fork();
    if(pid2 == 0){ //we have to fork exec the second part of the command from the pdf
        char folder[1024];
        snprintf(folder, 1024, "%s/.dsh/%d/%d.stdout", getenv("HOME"),session, outputNum); //build the arguments
        char *args[] = {"screen", "-X", "exec", "cat", folder, 0};
        char path[1024];
        strncpy(path, getenv("PATH"), 1024);
        //printf("pathinfindpath:%s\n", path);
        for ( char *dir = strtok( path, ":" ); dir; dir = strtok( NULL, ":" ) ) { //tokenize the path by :
            char s[100];
            snprintf(s,100,"%s/%s",dir,"screen"); //for every path token, build a path string ex: path+/+screen  
            execv(s, args);
        }
    }
    wait(0); //wait for child to exec
}
int checkDirectory(){
    //.dsh/4
    int count = 0;
    while(1){ //loop checks every folder in the .dsh directory
        char folder[100];
        snprintf(folder,100,"%s/.dsh/%d",getenv("HOME"),count); //continuously build directories until we get one that isn't there
                                                                //example $HOME/.dsh/0
        DIR* dir = opendir(folder);
        if(dir){
            closedir(dir); //if we find a directory that exists, we close it and try for the next one
        } else if(ENOENT == errno) {
            //directory doesn't exist so we take the previous one that did to becuase we now know the the last dir num
            if(count == 0) //edge case if this is the first session ever run
                return 0;
            else
                return count - 1;
        } else {
            //something bad happened
            perror("opendir error\n");
        }
        count++;
    }
    
}
int main(int argc, char** argv) {
    char *line=0;
    size_t size=0;
    dsh_init();
    initscr(); //initialize screen
    cbreak();
    noecho();
    mvwprintw(stdscr, WIN_START_Y - 2, WIN1_START_X, "%s", "Typing Window"); //create window
    WINDOW *input = newwin(WIN_HEIGHT, WIN_WIDTH, WIN_START_Y, WIN1_START_X);
    refresh();
    int topX = 1;
    int topY = 1;
    int orig_x = 1;
    int orig_y = 30;
    int input_y = 30;
    int input_x = 5;
    box(input, 0,0);
    wrefresh(input);
    keypad(input, true);
    int inputChar = 0;
    // int prevChar = 0;
    char commandArray[100][50];//array for storing commands
    mvwprintw(input,orig_y,orig_x,"%s", "dsh> ");
    char buf[50] = {""}; //array for storing what is typed
    int buffIndex = 0;//index for buf array
    int commandsGiven = -1;//starting index for commandArray
    int numUpPressed = -1;
    int cursor = -1;
    int sessionNum = checkDirectory(); //grab the session number
    while(1){
        inputChar = wgetch(input);

        if(inputChar == KEY_BACKSPACE){//backspace
            if(buffIndex != 0){//make sure that something has been typed to delete
                input_x--; //decrement input cord
                wmove(input, input_y,input_x);//move cursor back one space
                wdelch(input);//delete input under cursor
                buffIndex--;//adjust the input buf
                buf[buffIndex] = '0';
            } else {
                continue;
            }

        } else if(inputChar == '\n'){ //newline encountered
            commandsGiven++;
            strncpy(commandArray[commandsGiven],buf,buffIndex);
            buf[buffIndex] = '\n';
            dsh_run(buf);//run line
            buffIndex = 0; //reset the bufindex
            memset(buf, 0 , 50);//clear input string
            for(int i = commandsGiven; i > -1; i--){ //print command history
                mvwprintw(input,topY + i, topX, "%d:%s",i,commandArray[i]);
             }
            catFile(sessionNum, commandsGiven); //pass latest command to the function to cat it
            // box(input,0,0);
            input_x = 5; //reset x and y
            input_y = 30;
            wmove(input, orig_y,orig_x);//move the cursor back to the front of the input line and clear it
            wclrtoeol(input);
            
            mvwprintw(input,orig_y,orig_x,"%s", "dsh> ");//reprint the prompt
            cursor = -1;
            wrefresh(input);
        } else if(inputChar == KEY_UP | inputChar == KEY_DOWN){

            if(cursor == -1 && inputChar == KEY_UP){
                cursor = commandsGiven+1;
            } else if(cursor==-1 && inputChar == KEY_DOWN){
                continue;
            }
            if(inputChar == KEY_UP){
                cursor--;
            } else if(inputChar == KEY_DOWN){
                cursor++;
            }
            
            for(int i = commandsGiven; i > -1; i--){ //print command history
                if(cursor == i){
                    wattron(input, A_REVERSE);
                    mvwprintw(input,topY + i, topX, "%d:%s",i,commandArray[i]);
                    wattroff(input, A_REVERSE);
                    wmove(input, orig_y, orig_x);
                    wclrtoeol(input);
                    memset(buf,0,50); //clear the buff
                    buffIndex = strlen(commandArray[i]);
                    input_x = 6 + buffIndex;
                    strncpy(buf,commandArray[i],buffIndex);
                    mvwprintw(input, orig_y,orig_x,"%s%s","dsh> ", buf);
                    catFile(sessionNum,i);
                    

                } else {
                    mvwprintw(input,topY + i, topX, "%d:%s",i,commandArray[i]);
                }
                
            }
            wmove(input, orig_y, input_x + 1);
            
        } else{ //just a normal char
            buf[buffIndex] = inputChar;
            buffIndex++;
            mvwprintw(input,input_y,input_x,"%c",inputChar);
		    input_x++;
            cursor = -1;
        }
    }
}