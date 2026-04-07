#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <signal.h>

char expected_content[1024] ;
volatile sig_atomic_t running = 1;

void tulis_log(char *pesan) {
    FILE *fp = fopen("work.log", "a");
    if (fp != NULL) {
        fprintf(fp, "%s\n", pesan);
        fclose(fp);
    }
}

void handle_signal(int sig) {
    tulis_log("We really weren’t meant to be together");
    running = 0;
}

void buat_contract(int restored ){
  FILE *fp = fopen("contract.txt", "w");
	if (fp == NULL) return ;

    	time_t now = time(NULL) ;
    	char *time_str = ctime(&now) ;

    if (restored) {
        sprintf(expected_content,
                "\"A promise to keep going, even when unseen.\"\nrestored at: %s",
                time_str);
    } else {
        sprintf(expected_content,
                "\"A promise to keep going, even when unseen.\"\ncreated at: %s",
                time_str);
    }
   fputs(expected_content, fp);
    fclose(fp);
}

int file_sama() {
    FILE *fp = fopen("contract.txt", "r");
    if (fp == NULL) return 0;
    
	char buffer[1024];
    	size_t n = fread(buffer, 1, sizeof(buffer) - 1, fp);
    	buffer[n] = '\0';
    
    	fclose(fp) ;

    return strcmp(buffer,expected_content) == 0;
}

int main() {
    pid_t pid = fork();

    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    umask(0);

    if (setsid() < 0) exit(EXIT_FAILURE);
    chdir(".");

	signal(SIGTERM, handle_signal);
    	signal(SIGINT, handle_signal);

    srand(time(NULL));
    buat_contract(0);

    char *status_list[] = {"[awake]", "[drifting]", "[numbness]"};
    int counter = 0;

    while (running) {
        sleep(1);
        counter++;

        if (access("contract.txt", F_OK) != 0) {
            buat_contract(1);
        } else if (!file_sama()) {
            tulis_log("contract violated.");
            buat_contract(1);
        }
        if (counter >= 5) {
            char log[100];
            sprintf(log, "still working... %s", status_list[rand() % 3]);
            tulis_log(log);
            counter = 0;
        }
    }

    return 0;
}
