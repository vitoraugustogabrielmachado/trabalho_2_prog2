#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "biblioteca.h"

super *inicializarSuperbloco(){
    super *aux = malloc(sizeof(super));
    if(aux == NULL){
        printf("Alocacao mal-sucedida\n");
        return(NULL);
    }
    aux->numeroDoc = 0;
    aux->offsetCont = sizeof(super);
    return(aux);
}

void gbvListar(biblioteca *biblio){ 
    if(biblio->sup->numeroDoc == 0){
        printf("Arquivo vazio\n");
        return;
    }
    struct tm *aux = NULL;
    printf("----------ARQUIVOS----------\n");
    for(int i = 0; i < biblio->sup->numeroDoc; i++){
        printf("Nome: %s\n", biblio->dir[i].nome);
        printf("Tamanho: %ld\n", biblio->dir[i].tamanho);
        printf("Offset: %ld\n", biblio->dir[i].offset);
        aux = localtime(&biblio->dir[i].data);
        printf("Data: %02d/%02d/%04d\n", aux->tm_mday, aux->tm_mon + 1, aux->tm_year + 1900);
        printf("Hora: %02d:%02d:%02d\n", aux->tm_hour, aux->tm_min, aux->tm_sec);
        printf("\n");
    }
}

int escreverArquivoNoGBV(FILE *GBV, FILE *novo, super *sup){ //trocar a ordem
    int bytes_lidos = 0, lidos = 0;
    char buffer[BUFFER_SIZE_ESCRITA];

    fseek(GBV, sup->offsetCont, SEEK_SET); //procuro onde escrever os dados
    while ((bytes_lidos = fread(buffer, 1, BUFFER_SIZE_ESCRITA, novo)) > 0){ //fread retorna 0 caso chegou no fim do arquivo
        lidos += bytes_lidos;
        fwrite(buffer, 1, bytes_lidos, GBV); //escrevo no arquivo gbv
    }
    return(lidos);
}

int verificarRepeticao(documento *dir, char nome_procurado[], int tamanhoMaximo, int *repetido){
    for (int j = 0; j < tamanhoMaximo; j++){
        if(strcmp(dir[j].nome, nome_procurado) == 0){
            *repetido = j;
            return(1);
        }
    }
    return(0);
}


//na vdd eh o offset do ultimo arquivo antigo + o tamanho do ultimo novo

documento *adicionarArquivoNoDiretorio(biblioteca *biblio, char *argv[], int posicao, int lidos){
    int repetido = 0;
    documento novo_arq; 

    strcpy(novo_arq.nome, argv[posicao]); 
    novo_arq.tamanho = lidos;
    novo_arq.offset = biblio->sup->offsetCont;
    novo_arq.data = time(NULL);
    biblio->sup->offsetCont += lidos;

    if(verificarRepeticao(biblio->dir, novo_arq.nome, biblio->sup->numeroDoc, &repetido) == 1){ //comparo se ja existe um arquivo com o msm nome
        biblio->dir[repetido] = novo_arq; //caso sim, eu faço com que seja substituido o arquivo antigo pelo novo, nao apagando os dados
    }else{
        biblio->sup->numeroDoc++; 
        biblio->dir = realloc(biblio->dir, biblio->sup->numeroDoc * sizeof(documento));
        biblio->dir[biblio->sup->numeroDoc - 1] = novo_arq;
    }
    return(biblio->dir);

}

void gbvAdicionar(int tam, char *argv[], biblioteca *biblio, FILE *GBV){
    int lidos = 0;
    if(tam <= 3){ //se nao tiver arquivos para adicionar
        printf("Comando invalido, rerun com: ./gbv -a %s <documento que deseja adicionar> \n", argv[2]);
        return;
    }

    for(int i = 3; i < tam; i++){ //se tiver 
        FILE *novo = fopen(argv[i], "rb");
        if(novo != NULL){
            lidos = escreverArquivoNoGBV(GBV, novo, biblio->sup); //a funcao retorna a quantidade de bytes lidos
            biblio->dir = adicionarArquivoNoDiretorio(biblio, argv, i, lidos);
            fclose(novo);
        }else{
            printf("Documento %s inexistente\n", argv[i]);
        }
    }

    fseek(GBV, biblio->sup->offsetCont, SEEK_SET); //procuro onde escrever o diretorio
    fwrite(biblio->dir, sizeof(documento), biblio->sup->numeroDoc, GBV);

    fseek(GBV, 0, SEEK_SET); //procuro onde escrever o superbloco
    fwrite(biblio->sup, sizeof(super), 1, GBV);
}

void gbvRemover(int tam, char *argv[], biblioteca *biblio, FILE *GBV){
    int posicao_repeticao = 0;
    if(tam <= 3){ //se nao tiver arquivos para adicionar
        printf("Comando invalido, rerun com: ./gbv -r %s <documento que deseja remover> \n", argv[2]);
        return;
    }
    for(int i = 3; i < tam; i++){ //se tiver 
        if(verificarRepeticao(biblio->dir, argv[i], biblio->sup->numeroDoc, &posicao_repeticao) == 1){ //procura se existe o nome digitado
            if(posicao_repeticao != biblio->sup->numeroDoc - 1) //caso ele nao seja o ultimo, para nn ficar uma lacuna no diretorio, eu coloco o ultimo no lugar
                biblio->dir[posicao_repeticao] = biblio->dir[biblio->sup->numeroDoc - 1];

            biblio->sup->numeroDoc--;
            biblio->dir = realloc(biblio->dir, biblio->sup->numeroDoc * sizeof(documento));
        }else{
            printf("Documento inexistente na biblioteca\n"); //se nao existe o nome do arquivo nn tem como remover
            return;
        }
    }
    fseek(GBV, biblio->sup->offsetCont, SEEK_SET); //procuro onde escrever o diretorio
    fwrite(biblio->dir, sizeof(documento), biblio->sup->numeroDoc, GBV);

    fseek(GBV, 0, SEEK_SET); //procuro onde escrever o superbloco
    fwrite(biblio->sup, sizeof(super), 1, GBV);
}

void avancarOuVoltar(char teclaEscolhida, long int inicioLeitura, long int fimLeitura, long int tamanhoLeitura, long int *offsetAtual){
    switch(teclaEscolhida){
        case 'p':
            if(*offsetAtual < inicioLeitura)
                printf("Não é possível voltar");
            else{
                if(*offsetAtual == fimLeitura)
                    *offsetAtual -= tamanhoLeitura;
                else
                    *offsetAtual -= BUFFER_SIZE_VISUALIZAR;
                if(*offsetAtual < inicioLeitura)
                    *offsetAtual = inicioLeitura;
            }
            break;
        case 'n':
            if(*offsetAtual > fimLeitura)
                printf("Não é possível avançar");
            else{
                *offsetAtual += BUFFER_SIZE_VISUALIZAR;
                if(*offsetAtual > fimLeitura)
                    *offsetAtual = fimLeitura;
            }
            return;
    }
}

void vizualizacaoArquivo(FILE *GBV, documento *dir, int posicaoRepetida){
    char teclaEscolhida;
    char buffer[BUFFER_SIZE_VISUALIZAR + 1];

    long int inicioLeitura = dir[posicaoRepetida].offset;
    long int fimLeitura = dir[posicaoRepetida].tamanho + inicioLeitura;
    long int bytesLidos = 0;
    long int offsetAtual = inicioLeitura;
    long int tamanhoLeitura = 0;
    long int restante = 0;

    while(1){
        restante = fimLeitura - offsetAtual;
        if(restante <= 0){
            printf("Fim do arquivo\n");
        }
        else{
            if(restante < BUFFER_SIZE_VISUALIZAR)
                tamanhoLeitura = restante;
            else
                tamanhoLeitura = BUFFER_SIZE_VISUALIZAR;
            
            fseek(GBV, offsetAtual, SEEK_SET);
            bytesLidos = fread(buffer, 1, tamanhoLeitura, GBV);
            buffer[bytesLidos] = '\0';
            printf("%s\n", buffer);
        }

        scanf("%c", &teclaEscolhida);
        getchar();
        avancarOuVoltar(teclaEscolhida, inicioLeitura, fimLeitura, tamanhoLeitura, &offsetAtual);
        if(teclaEscolhida == 'q')
            break;
    }
}

void gbvVisualizar(int tam, char *argv[], documento *dir, FILE *GBV){
    if(tam <= 3){ //se nao tiver arquivos para adicionar
        printf("Comando invalido, rerun com: ./gbv -v %s <documento que deseja visualizar>\n", argv[2]);
        return;
    }
    if(tam > 4){
        printf("So eh possivel visualizar um documento por vez, rerun com: ./gbv -v %s <documento que deseja visualizar>\n", argv[2]);
        return;
    }
    int posicaoRepetida = 0;
    if(verificarRepeticao(dir, argv[3], tam, &posicaoRepetida) == 1){
        vizualizacaoArquivo(GBV, dir, posicaoRepetida);
    }
    else{
        printf("Documento inexistente no container\n");
        return;
    }

}

int compararTamanho(const void *a, const void *b){
    documento *aux = (documento*)a;
    documento *temp = (documento*)b;

    if(aux->tamanho < temp->tamanho)
        return(-1);
    if(aux->tamanho > temp->tamanho)
        return(1);
    return(0);
}

int compararDatas(const void *a, const void *b){
    documento *aux = (documento*)a;
    documento *temp = (documento*)b;

    if(aux->data < temp->data)
        return(-1);
    if(aux->data > temp->data)
        return(1);
    return(0);
}

int compararNomes(const void *a, const void *b){
    documento *aux = (documento*)a;
    documento *temp = (documento*)b;

    return(strcmp(aux->nome, temp->nome));
}

void gbvOrdenar(char *argv[], biblioteca *biblio, FILE *GBV){
    if(argv[3] == NULL){
        printf("Comando invalido, rerun com: ./gbv -o %s <tipo de ordenação>\n", argv[2]);
        return;
    }
    char *opcao = argv[3];
    if(strcmp(opcao, "nome") == 0)
        qsort(biblio->dir, biblio->sup->numeroDoc, sizeof(documento), compararNomes);
    else if(strcmp(opcao, "data") == 0)
        qsort(biblio->dir, biblio->sup->numeroDoc, sizeof(documento), compararDatas);
    else if(strcmp(opcao, "tamanho") == 0)
        qsort(biblio->dir, biblio->sup->numeroDoc, sizeof(documento), compararTamanho);
    else{
        printf("Ordenação inválida, rerun com: tamanho, data ou nome\n");
        return;
    }
    fseek(GBV, biblio->sup->offsetCont, SEEK_SET);
    fwrite(biblio->dir, sizeof(documento), biblio->sup->numeroDoc, GBV);
}