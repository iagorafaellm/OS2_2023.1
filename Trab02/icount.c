/* Autores: Iago Rafael Lucas Martins (119181254) e Lucas Moreno Silva (119140949) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <getopt.h>
#include <limits.h>

#define MAX_PATH_LEN 4096

int inode_type = S_IFREG;
int count = 0;

void count_inode(const char *path) {
    /* função contadora que é chamada para cada entrada encontrada no diretório */
    struct stat st;
    if (lstat(path, &st) == -1) {
        fprintf(stderr, "Error lstat on path %s\n", path);
        return;
    }
    if ((st.st_mode & S_IFMT) == inode_type) {
        count++;
    }
    /* tipo de INODE da entrada usando a função stat e incrementa um contador se o tipo for o desejado */
}

void print_count(const char *dir_path) {
    printf("%s: %d\n", dir_path, count);
}

void usage() {
    printf("Usage: icount [-rdlbc] [<dir> ...]\n");
    printf("Options:\n");
    printf("  -r: regular file (default)\n");
    printf("  -d: directory\n");
    printf("  -l: symbolic link\n");
    printf("  -b: block device\n");
    printf("  -c: character device\n");
}

int walk_dir(const char *path, void (*func)(const char *)) {
  DIR *dirp;
  struct dirent *dp;
  char *p, *full_path;
  int len;
  /* abre o diretório */
  if ((dirp = opendir(path)) == NULL) return (-1);
  len = strlen(path);
  /* aloca uma área na qual, garantidamente, o caminho caberá */
  if ((full_path = malloc(len + NAME_MAX + 2)) == NULL) {
    closedir(dirp);
    return (-1);
  }
  /* copia o prefixo e acrescenta a ‘/’ ao final */
  memcpy(full_path, path, len);
  p = full_path + len;
  *p++ = '/'; /* deixa “p” no lugar certo! */
  while ((dp = readdir(dirp)) != NULL) {
    /* ignora as entradas “.” e “..” */
    if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) continue;
    strcpy(p, dp->d_name);
    /* “full_path” armazena o caminho */
    (*func)(full_path);
  }
  free(full_path);
  closedir(dirp);
  return (0);
}

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "rdlbc")) != -1) {
        /* função getopt para obter as opções e argumentos da linha de comando */
        switch (opt) {
            case 'r':
                inode_type = S_IFREG;
                break;
            case 'd':
                inode_type = S_IFDIR;
                break;
            case 'l':
                inode_type = S_IFLNK;
                break;
            case 'b':
                inode_type = S_IFBLK;
                break;
            case 'c':
                inode_type = S_IFCHR;
                break;
            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }
    if (optind == argc) {
        /* sem diretório especificado, usar o diretório atual */
        count = 0;
        char path[MAX_PATH_LEN];
        if (getcwd(path, MAX_PATH_LEN) == NULL) {
            perror("Error getting current directory");
            exit(EXIT_FAILURE);
        }
        if (walk_dir(path, count_inode) == -1) {
            /* walk_dir para percorrer o diretório e contar os INODEs do tipo desejado */
            fprintf(stderr, "Error walking directory %s\n", path);
            exit(EXIT_FAILURE);
        }
        print_count(".");
    } else {
        /* diretórios de processos especificados como argumentos */
        for (int i = optind; i < argc; i++) {
            count = 0;
            if (walk_dir(argv[i], count_inode) == -1) {
                fprintf(stderr, "Error walking directory %s\n", argv[i]);
                exit(EXIT_FAILURE);
            }
            print_count(argv[i]);
        }
    }
    exit(EXIT_SUCCESS);
}