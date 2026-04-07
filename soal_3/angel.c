#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>

volatile sig_atomic_t daemon_running = 1;

char base_dir[PATH_MAX] ;
char love_path[PATH_MAX] ;
char log_path[PATH_MAX] ;
char pid_path[PATH_MAX];

const char *sentences[] = {
    	"aku akan fokus pada diriku sendiri",
    	"aku mencintaimu dari sekarang hingga selamanya",
    	"aku akan menjauh darimu, hingga takdir mempertemukan kita di versi kita yang terbaik",
    	"kalau aku dilahirkan kembali, aku tetap akan terus menyayangimu"
} ;

const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void signal_handler(int sig) {
    daemon_running = 0;
}
void get_log_date(char *datebuf, size_t dsz, char *timebuf, size_t tsz) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(datebuf, dsz, "%d:%m:%Y", tm_info);
    strftime(timebuf, tsz, "%H:%M:%S", tm_info);
}
void write_log(const char *process_name, const char *status) {
    char datebuf[32], timebuf[32];
    get_log_date(datebuf, sizeof(datebuf), timebuf, sizeof(timebuf));

    FILE *fp = fopen(log_path, "a");
    if (fp != NULL) {
        fprintf(fp, "[%s]-[%s]_%s_%s\n", datebuf, timebuf, process_name, status);
        fclose(fp);
    }
}

int write_text_file(const char *path, const char *content) {
    FILE *fp = fopen(path, "w");
    if (fp == NULL) return 0;
    fputs(content, fp);
    fclose(fp);
    return 1;
}

char *read_text_file(const char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) return NULL;

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    char *buffer = malloc(size + 1);
    if (buffer == NULL) {
        fclose(fp);
        return NULL;
    }
    fread(buffer, 1, size, fp);
    buffer[size] = '\0';
    fclose(fp);
    return buffer;
}

char *base64_encode(const unsigned char *data, size_t input_length) {
    size_t output_length = 4 * ((input_length + 2) / 3);
    char *encoded = malloc(output_length + 1);
    if (!encoded) return NULL;

    int i = 0, j = 0;
    while (i < (int)input_length) {
        unsigned int octet_a = i < (int)input_length ? data[i++] : 0;
        unsigned int octet_b = i < (int)input_length ? data[i++] : 0;
        unsigned int octet_c = i < (int)input_length ? data[i++] : 0;
        unsigned int triple = (octet_a << 16) + (octet_b << 8) + octet_c;

        encoded[j++] = base64_table[(triple >> 18) & 0x3F];
        encoded[j++] = base64_table[(triple >> 12) & 0x3F];
        encoded[j++] = base64_table[(triple >> 6) & 0x3F];
        encoded[j++] = base64_table[triple & 0x3F];
    }

    int mod = input_length % 3;
    if (mod > 0) {
        encoded[output_length - 1] = '=';
        if (mod == 1) encoded[output_length - 2] = '=';
    }

    encoded[output_length] = '\0';
    return encoded;
}

int base64_index(char c) {
    if ('A' <= c && c <= 'Z') return c - 'A';
    if ('a' <= c && c <= 'z') return c - 'a' + 26;
    if ('0' <= c && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

char *base64_decode(const char *data, size_t *out_len) {
    size_t len = strlen(data);
    if (len % 4 != 0) return NULL;
    size_t alloc_len = len / 4 * 3;
    if (data[len - 1] == '=') alloc_len--;
    if (data[len - 2] == '=') alloc_len--;
    char *decoded = malloc(alloc_len + 1);
    if (!decoded) return NULL;
    size_t i, j = 0;
    for (i = 0; i < len; i += 4) {
        int a = base64_index(data[i]);
        int b = base64_index(data[i + 1]);
        int c = data[i + 2] == '=' ? 0 : base64_index(data[i + 2]);
        int d = data[i + 3] == '=' ? 0 : base64_index(data[i + 3]);

        unsigned int triple = (a << 18) | (b << 12) | (c << 6) | d;

        decoded[j++] = (triple >> 16) & 0xFF;
        if (data[i + 2] != '=') decoded[j++] = (triple >> 8) & 0xFF;
        if (data[i + 3] != '=') decoded[j++] = triple & 0xFF;
    }

    decoded[j] = '\0';
    if (out_len) *out_len = j;
    return decoded;
}

void setup_paths_from_cwd() {
    if (realpath(".", base_dir) == NULL) {
        perror("realpath");
        exit(EXIT_FAILURE);
    }
    if (chdir(base_dir) != 0) {
        perror("chdir");
        exit(EXIT_FAILURE);
    }
    if (realpath("LoveLetter.txt", love_path) == NULL) {
     	   FILE *fp = fopen("LoveLetter.txt", "a");
        if (fp != NULL) fclose(fp);
        if (realpath("LoveLetter.txt", love_path) == NULL) {
            perror("realpath LoveLetter.txt");
            exit(EXIT_FAILURE);
        }
    }
    if (realpath("ethereal.log", log_path) == NULL) {
     	   FILE *fp = fopen("ethereal.log", "a");
        if (fp != NULL) fclose(fp);
        if (realpath("ethereal.log", log_path) == NULL) {
            perror("realpath ethereal.log");
            exit(EXIT_FAILURE);
        }
    }
    if (realpath("angel.pid", pid_path) == NULL) {
 	       FILE *fp = fopen("angel.pid", "a");
        if (fp != NULL) fclose(fp);
        if (realpath("angel.pid", pid_path) == NULL) {
            perror("realpath angel.pid");
            exit(EXIT_FAILURE);
        }
    }
}

void setup_paths_from_env() {
    const char *env_dir = getenv("ANGEL_BASEDIR");
    if (env_dir == NULL) {
        fprintf(stderr, "ANGEL_BASEDIR tidak ditemukan\n");
        exit(EXIT_FAILURE);
    }
    if (realpath(env_dir, base_dir) == NULL) {
        perror("realpath ANGEL_BASEDIR");
        exit(EXIT_FAILURE);
    }
    if (chdir(base_dir) != 0) {
        perror("chdir");
        exit(EXIT_FAILURE);
    }
    if (realpath("LoveLetter.txt", love_path) == NULL) {
        	FILE *fp = fopen("LoveLetter.txt", "a");
        if (fp != NULL) fclose(fp);
        if (realpath("LoveLetter.txt", love_path) == NULL) {
            perror("realpath LoveLetter.txt");
            exit(EXIT_FAILURE);
        }
    }
    if (realpath("ethereal.log", log_path) == NULL) {
        	FILE *fp = fopen("ethereal.log", "a");
        if (fp != NULL) fclose(fp);
        if (realpath("ethereal.log", log_path) == NULL) {
            perror("realpath ethereal.log");
            exit(EXIT_FAILURE);
        }
    }
    if (realpath("angel.pid", pid_path) == NULL) {
 	       FILE *fp = fopen("angel.pid", "a");
        if (fp != NULL) fclose(fp);
        if (realpath("angel.pid", pid_path) == NULL) {
            perror("realpath angel.pid");
            exit(EXIT_FAILURE);
        }
    }
}
void write_pid_file() {
    FILE *fp = fopen(pid_path, "w");
    if (fp != NULL) {
        fprintf(fp, "%d\n", getpid());
        fclose(fp);
    }
}
void remove_pid_file() {
    unlink(pid_path);
}
void do_secret() {
    write_log("secret", "RUNNING");

    const char *chosen = sentences[rand() % 4];
    if (write_text_file(love_path, chosen)) {
        write_log("secret", "SUCCESS");
    } else {
        write_log("secret", "ERROR");
    }
}
void do_surprise() {
    write_log("surprise", "RUNNING");

    char *plain = read_text_file(love_path);
    if (plain == NULL) {
        write_log("surprise", "ERROR");
        return;
    }

    char *encoded = base64_encode((unsigned char *)plain, strlen(plain));
    if (encoded == NULL) {
        free(plain);
        write_log("surprise", "ERROR");
        return;
    }

    if (write_text_file(love_path, encoded)) {
        write_log("surprise", "SUCCESS");
    } else {
        write_log("surprise", "ERROR");
    }

    free(plain);
    free(encoded);
}
void run_daemon_loop() {
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    write_pid_file();
    srand(time(NULL) ^ getpid());

    while (daemon_running) {
        do_secret();
        do_surprise();

        for (int i = 0; i < 10 && daemon_running; i++) {
            sleep(1);
        }
    }

    remove_pid_file();
}
void start_daemon() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) exit(EXIT_FAILURE);

    setenv("ANGEL_INTERNAL", "1", 1);
    setenv("ANGEL_BASEDIR", cwd, 1);

    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    umask(0);
    if (setsid() < 0) exit(EXIT_FAILURE);
    if (chdir("/") < 0) exit(EXIT_FAILURE);

    execl("/proc/self/exe", "maya", NULL);
    exit(EXIT_FAILURE);
}
void decrypt_file() {
    setup_paths_from_cwd();

    write_log("decrypt", "RUNNING");

    char *encoded = read_text_file(love_path);
    if (encoded == NULL) {
        write_log("decrypt", "ERROR");
        printf("LoveLetter.txt tidak ditemukan.\n");
        return;
    }

    size_t out_len;
    char *decoded = base64_decode(encoded, &out_len);
    if (decoded == NULL) {
        write_log("decrypt", "ERROR");
        printf("Gagal decrypt isi file.\n");
        free(encoded);
        return;
    }

    if (write_text_file(love_path, decoded)) {
        write_log("decrypt", "SUCCESS");
        printf("LoveLetter.txt berhasil didecrypt.\n");
    } else {
        write_log("decrypt", "ERROR");
        printf("Gagal menulis hasil decrypt.\n");
    }

    free(encoded);
    free(decoded);
}
void kill_daemon() {
    setup_paths_from_cwd();

    write_log("kill", "RUNNING");

    FILE *fp = fopen(pid_path, "r");
    if (fp == NULL) {
        write_log("kill", "ERROR");
        printf("Daemon tidak sedang berjalan.\n");
        return;
    }

    pid_t pid;
    if (fscanf(fp, "%d", &pid) != 1) {
        fclose(fp);
        write_log("kill", "ERROR");
        printf("PID daemon tidak valid.\n");
        return;
    }
    fclose(fp);

    if (kill(pid, SIGTERM) == 0) {
        write_log("kill", "SUCCESS");
        printf("Daemon berhasil dihentikan.\n");
    } else {
        write_log("kill", "ERROR");
        printf("Gagal menghentikan daemon.\n");
    }
}
void print_usage() {
    printf("Penggunaan:\n");
    printf("./angel -daemon   : jalankan sebagai daemon (nama proses: maya)\n");
    printf("./angel -decrypt  : decrypt LoveLetter.txt\n");
    printf("./angel -kill     : kill proses\n");
}
int main(int argc, char *argv[]) {
    const char *internal = getenv("ANGEL_INTERNAL");

    if (internal != NULL && strcmp(internal, "1") == 0) {
        setup_paths_from_env();
        run_daemon_loop();
        return 0;
    }

    if (argc != 2) {
        print_usage();
        return 0;
    }

    if (strcmp(argv[1], "-daemon") == 0) {
        start_daemon();
    } else if (strcmp(argv[1], "-decrypt") == 0) {
        decrypt_file();
    } else if (strcmp(argv[1], "-kill") == 0) {
        kill_daemon();
    } else {
        print_usage();
    }

    return 0;
}
