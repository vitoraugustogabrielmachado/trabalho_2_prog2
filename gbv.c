#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "biblioteca.h"

/*A funcao verifica se a biblioteca existe ou não, se não a funcao cria ela e escreve o superbloco zerado
dentro dela, se sim a funcao le do arquivo as informaçoes*/
void leitura_arquivo(FILE **GBV, biblioteca *biblio, char *argv[]){
    biblio->sup = inicializarSuperbloco();

    if(*GBV == NULL){
        *GBV = fopen(argv[2], "wb+");
        biblio->dir = NULL;
        fseek(*GBV, 0, SEEK_SET);
        fwrite(biblio->sup, 1, sizeof(super), *GBV);
    }else{
        fread(biblio->sup, sizeof(super), 1, *GBV);
        biblio->dir = malloc(biblio->sup->numeroDoc * sizeof(documento));
        if(biblio->dir == NULL){
            printf("Alocacao mal-sucedida\n");
            return;
        }
        fseek(*GBV, biblio->sup->offsetCont, SEEK_SET);
        fread(biblio->dir, sizeof(documento), biblio->sup->numeroDoc, *GBV);
    }
}

/*Trata das flags quando o programa é executado*/
void opcoes(int argc, char *argv[], biblioteca *biblio, FILE *GBV){
    char temp = argv[1][1];
    switch (temp){
        case 'a': 
            gbvAdicionar(argc, argv, biblio, GBV); //Adiciona novo(s) documento(s) na biblioteca
            break;
        case 'l':
            gbvListar(biblio); //Lista todos os arquivos que estão na biblioteca
            break;
        case 'r': 
            gbvRemover(argc, argv, biblio, GBV); //Remove o(s) documentos(s) requeridos
            break;
        case 'v':
            gbvVisualizar(argc, argv, biblio->dir, GBV); //Vizualiza o documento requerido por partes
            break;
        case 'o':
            gbvOrdenar(argv, biblio, GBV); //Ordena a biblioteca baseado na flag no momento da execução do programa
            break;
    }
}

int main(int argc, char *argv[]){
    biblioteca *biblio = malloc(sizeof(biblioteca));

    if(argc < 3){ //Caso o usuario não coloque a biblioteca na execução do programa
        printf("Comando invalido\n");
        return(0);
    }
    if(argv[1][0] != '-'){ //Caso o usuario não coloque o - junto com a opção que deseja na execução do programa
        printf("Flag invalida\n");
        return(0);
    }

    FILE *GBV = fopen(argv[2], "rb+");
    leitura_arquivo(&GBV, biblio, argv);
    opcoes(argc, argv, biblio, GBV);

    if(biblio->sup != NULL)
        free(biblio->sup);
    if(biblio->dir != NULL)
        free(biblio->dir);
    if(biblio != NULL)
        free(biblio);
    fclose(GBV);
}