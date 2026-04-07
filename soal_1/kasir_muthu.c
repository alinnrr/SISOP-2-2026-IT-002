#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void error_exit(){
    printf("[ERROR] Aiyaa! Proses gagal, file atau folder tidak ditemukan.\n") ;
    exit(EXIT_FAILURE) ;
}

void jalankan_perintah(char *path,char *argv[]) {
    pid_t pid;
    int status;

    pid = fork();

    if (pid < 0){
        error_exit();
    }
    if (pid==0) {
        execv(path, argv);
        exit(EXIT_FAILURE);
    } else {
        if (waitpid(pid, &status, 0) == -1) {
            error_exit() ;
        }
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0){
            error_exit();
        }
    }
}

int main() {
    char *mkdir_args[] = {"mkdir", "brankas_kedai",NULL} ;
    char *cp_args[] = {"cp", "buku_hutang.csv", "brankas_kedai/", NULL};
    char *grep_args[] = {
        "sh", "-c",
        "grep 'Belum Lunas' brankas_kedai/buku_hutang.csv > brankas_kedai/daftar_penunggak.txt",
        NULL
    } ;

    char *zip_args[] = {"zip", "-r", "rahasia_muthu.zip", "brankas_kedai", NULL};

	jalankan_perintah("/bin/mkdir", mkdir_args);

	jalankan_perintah("/bin/cp", cp_args);

    	jalankan_perintah("/bin/sh", grep_args);

    	jalankan_perintah("/usr/bin/zip", zip_args);

    printf("[INFO] Fuhh, selamat! Buku hutang dan daftar penagihan berhasil diamankan.\n");

    return 0;
}
