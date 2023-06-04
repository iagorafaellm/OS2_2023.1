/* Autores: Iago Rafael Lucas Martins (119181254) e Lucas Moreno Silva (119140949) */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int isopen(int fd) {
    /* Verifica se o descritor de arquivo está aberto */
    int flags = fcntl(fd, F_GETFD);
    if (flags == -1 && errno == EBADF) {
        /* Se o erro for EBADF, então nosso descritor não está aberto
        Caso o retorno do sistema for -1, tivemos um erro */
        return 0;
    }
    /* O descritor está aberto, retorna 1 */
    return 1;
}

int main(void) {

    /* testando abrir um arquivo a mais para ver 4 descritores abertos */
    FILE * fp;
    int nopen = 0, fd;
    fp = fopen("teste.txt", "w");

    if(fp == NULL) {
        printf("Erro ao abrir o arquivo");
        return 1;
    }


    for (fd = 0; fd < getdtablesize(); fd++) {
        /* percorre todos os possíveis descritores de arquivo (de 0 a getdtablesize() - 1) */
        if (isopen(fd)) {
            /* conta quantos deles estão abertos, utilizando a função isopen */
            nopen++;
        }
    }
    printf("Existem %d descritores abertos\n", nopen);
    return 0;
}