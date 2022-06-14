#define HAVE_STRUCT_TIMESPEC
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <errno.h>
#include <signal.h>
#include <conio.h>		                                                        
#include <math.h>                                                               
#include <time.h>
#include <string.h>
#include <locale.h>
#include <pthread.h>
#include "CheckForError.h"   


#define _CHECKERROR	    1				                                        
#define RAM             100                                                     
#define	ESC_KEY			27                                                      
#define FILE_SIZE       200     

//Cabeçalhos de funções
void CriarObjetos();
void FecharHanlders();
void CriarThreadsSecundarias();
void CapturarTeclado();
void CriarProcessosExibicao();

void* GeraDadosOtimizacao(void* arg);
void* GeraDadosProcesso(void* arg);
void* GeraAlarmes(void* arg);

void* RetiraDadosOtimizacao(void* arg);
void* RetiraDadosProcesso(void* arg);
void* RetiraAlarmes(void* arg);

void* ExibeDadosOtimizacao(void* arg);
void* ExibeDadosProcesso(void* arg);
void* ExibeAlarmes(void* arg);

char RamBuffer[RAM][52], key;
int p_ocup = 0, p_livre = 0;
int n_mensagem = 0;

//Objetos do escopo global
HANDLE hMutexBuffer;
HANDLE hSemLivre, hSemOcupado;
HANDLE hEventKeyC, hEventKeyO, hEventKeyP, hEventKeyA, hEventKeyT, hEventKeyR, hEventKeyL, hEventKeyZ, hEventKeyEsc, hTimeOut;

int main() {

    setlocale(LC_ALL, "Portuguese");
    SetConsoleTitle(L"| Terminal Principal |");                               /*Altera nome do terminal da thread primaria*/

    CriarObjetos();            
    CriarThreadsSecundarias(); 
    CriarProcessosExibicao();  
    CapturarTeclado();         // funcionando                               /*Faz a leitura do teclado enquanto a execução está ativa. A tecla ESC encerra a execução*/
    WaitForSingleObject(hTimeOut, 5000);                                    
    FecharHanlders();                                                       /*Aguardando o fim dos processos e threads antes de encerrar*/

    printf("Finalizando - Inputs do teclado\n");

    return EXIT_SUCCESS;                                                   /*fim da funcao main*/
} 

void CriarObjetos() {

    // Mutexes
    hMutexBuffer = CreateMutex(NULL, FALSE, L"MutexBuffer");                /*Para controlar o acesso ao buffer de mensagem*/

    // Semáforos
    hSemLivre = CreateSemaphore(NULL, RAM, RAM, L"SemLivre");               /*Espaço Livre*/
    hSemOcupado = CreateSemaphore(NULL, 0, RAM, L"SemOcupado");             /*Espaço ocupado*/

    // Eventos
    //segurança nula, reset automatico, não inicializado, chaveUnica
    hTimeOut   = CreateEvent(NULL, FALSE, FALSE, L"TimeOut");
    hEventKeyC = CreateEvent(NULL, FALSE, FALSE, L"KeyC");                  /*Tecla C - sinalizador on-off da tarefa de comunicacao de dados */
    hEventKeyO = CreateEvent(NULL, FALSE, FALSE, L"KeyO");                  /*Tecla O - sinalizador on-off da tarefa de retirada de dados de otimizacao  */
    hEventKeyP = CreateEvent(NULL, FALSE, FALSE, L"KeyP");                  /*Tecla P - sinalizador on-off da tarefa de retirada de dados de processo */
    hEventKeyA = CreateEvent(NULL, FALSE, FALSE, L"KeyA");                  /*Tecla A - sinalizador on-off da tarefa de retirada de alarmes */
    hEventKeyT = CreateEvent(NULL, FALSE, FALSE, L"KeyT");                  /*Tecla T - sinalizador on-off da tarefa de exibicao de dados de otimizacao */
    hEventKeyR = CreateEvent(NULL, FALSE, FALSE, L"KeyR");                  /*Tecla R - sinalizador on-off da tarefa de exibicao de dados de processo */
    hEventKeyL = CreateEvent(NULL, FALSE, FALSE, L"KeyL");                  /*Tecla L - sinalizador on-off da tarefa de exibicao de alarmes */
    hEventKeyZ = CreateEvent(NULL, FALSE, FALSE, L"KeyZ");                  /*Tecla Z - sinalizador de limpeza de console da tarefa de  alarmes*/
    //reset manual
    hEventKeyEsc = CreateEvent(NULL, TRUE, FALSE, L"KeyEsc");               /*Tecla Esc - Encerramento de todas as tarefas e programas*/             
}

void CriarProcessosExibicao()
{
    /*Criando processos filhos*/
    STARTUPINFO startup_info;				                                   /*StartUpInformation para novo processo*/
    PROCESS_INFORMATION process_info;	                                       /*Informacoes sobre novo processo criado*/
    bool statusProcess = 0;
    ZeroMemory(&startup_info, sizeof(startup_info));                           /* Zera um bloco de memória localizado em &si passando o comprimento a ser zerado. */
    startup_info.cb = sizeof(startup_info);	                                   /*Tamanho da estrutura em bytes*/

    /*Processo de exibicao de dados do novo sistema de otimizacao - Terminal DadosOtimizacao do Processo*/
    statusProcess = CreateProcess(
        L"..\\x64\\Debug\\ExibicaoDadosOtimizacao.exe",                        /*Caminho relativo do arquivo executavel*/
        NULL,                                                                  /*Apontador para parametros de linha de comando*/
        NULL,                                                                  /*Apontador para descritor de segurança*/
        NULL,                                                                  /*Apontador para threads do processo*/
        FALSE,	                                                               /*Heranca de handles*/
        CREATE_NEW_CONSOLE,	                                                   /*Flags de criação - Criar novo console no caso*/
        NULL,	                                                               /*Heranca do ambiente de execucao*/
        L"..\\x64\\Debug",                                                          /*Diretorio do arquivo executavel*/
        &startup_info,			                                               /*lpStartUpInfo*/
        &process_info);	                                                       /*lpProcessInformation*/

    if (!statusProcess) {
        printf("Erro na criacao do Terminal de Dados do Otimizacao! Codigo = %d\n", GetLastError());
    }
    else {
        printf("Processo de exibicao de Dados de Otimizacao e Terminal criados\n");
    }

    /*Processo de exibicao de dados da plataforma petrolifica- Terminal Dados Scada do Processo*/
    statusProcess = CreateProcess(
        L"..\\x64\\Debug\\ExibicaoDadosProcesso.exe",                        /*Caminho relativo do arquivo executavel*/
        NULL,                                                                  /*Apontador para parametros de linha de comando*/
        NULL,                                                                  /*Apontador para descritor de segurança*/
        NULL,                                                                  /*Apontador para threads do processo*/
        FALSE,	                                                               /*Heranca de handles*/
        CREATE_NEW_CONSOLE,	                                                   /*Flags de criação - Criar novo console no caso*/
        NULL,	                                                               /*Heranca do ambiente de execucao*/
        L"..\\x64\\Debug",                                                     /*Diretorio do arquivo executavel*/
        &startup_info,			                                               /*lpStartUpInfo*/
        &process_info);	                                                       /*lpProcessInformation*/

    if (!statusProcess) {
        printf("Erro na criacao do Terminal de Dados do Processo! Codigo = %d\n", GetLastError());
    }
    else {
        printf("Processo de exibicao de Dados de Processo e Terminal criados\n");
    }

    /*Processo de exibicao de alarmes - Terminal Alarmes*/
    statusProcess = CreateProcess(
        L"..\\x64\\Debug\\ExibicaoAlarmes.exe",                                /*Caminho relativo do arquivo executavel*/
        NULL,                                                                  /*Apontador para parametros de linha de comando*/
        NULL,                                                                  /*Apontador para descritor de segurança*/
        NULL,                                                                  /*Apontador para threads do processo*/
        FALSE,	                                                               /*Heranca de handles*/
        CREATE_NEW_CONSOLE,	                                                   /*Flags de criação - Criar novo console no caso*/
        NULL,	                                                               /*Heranca do ambiente de execucao*/
        L"..\\x64\\Debug",                                                     /*Diretorio do arquivo executavel*/
        &startup_info,			                                               /*lpStartUpInfo*/
        &process_info);	                                                       /*lpProcessInformation*/

    if (!statusProcess) {
        printf("Erro na criacao do Terminal Alarmes! Codigo = %d\n", GetLastError());
    }
    else {
        printf("Processo de exibicao de Alarmes e Terminal criados\n");
    }
}

void CapturarTeclado()
{
    /*------------------------------------------------------------------------------*/
    /*Tratando inputs do teclado*/
    while (key != ESC_KEY) {
        key = _getch();
        switch (key) {
        case 'c':
        case 'C':
            SetEvent(hEventKeyC);
            printf("Tecla C pressionada\n");
            break;
        case 'o':
        case 'O':
            SetEvent(hEventKeyO);
            printf("Tecla O pressionada\n");
            break;
        case 'p':
        case 'P':
            SetEvent(hEventKeyP);
            printf("Tecla P pressionada\n");
            break;
        case 'a':
        case 'A':
            SetEvent(hEventKeyA);
            printf("Tecla A pressionada\n");
            break;
        case 't':
        case 'T':
            SetEvent(hEventKeyT);
            printf("Tecla T pressionada\n");
            break;
        case 'r':
        case 'R':
            SetEvent(hEventKeyR);
            printf("Tecla R pressionada\n");
            break;
        case 'l':
        case 'L':
            SetEvent(hEventKeyL);
            printf("Tecla L pressionada\n");
            break;
        case 'z':
        case 'Z':
            SetEvent(hEventKeyZ);
            printf("Tecla Z pressionada\n");
            break;
        case ESC_KEY:
            printf("\n");
            SetEvent(hEventKeyEsc);
            break;
        default:
            printf("Tecla pressionada não tem uma função!\n");
            break;
        } /*fim do switch*/
    } /*fim do while*/
}

void CriarThreadsSecundarias()
{
    bool status = false; //success
    DWORD ret;

    /*Handles threads*/
    pthread_t hLeituraDadosOtimizacao, hLeituraDadosScada, hLeituraAlarmes, 
              hRetiraDadosOtimizacao, hRetiraDadosScada, hRetiraAlarmes,
              hComunicacaoOtimizacao, hComunicacaoScada, hComunicacaoAlarmes;  /*Tarefas de Comunicacao de dados*/

    /*Criando threads de Comunicacao de Dados*/
    int i = 1;
    status = pthread_create(&hComunicacaoOtimizacao, NULL, GeraDadosOtimizacao, (void*)i);
    if (!status) printf("Thread %d - Comunicacao de dados Otimizacao - criada com Id= %0x \n", i, (int)&hComunicacaoOtimizacao);
    else printf("Erro na criacao da thread %d! Codigo = %d\n", i, GetLastError());

    i = 2;
    status = pthread_create(&hComunicacaoScada, NULL, GeraDadosProcesso, (void*)i);
    if (!status) printf("Thread %d - Comunicacao de dados Scada - criada com Id= %0x \n", i, (int)&hComunicacaoScada);
    else printf("Erro na criacao da thread %d! Codigo = %d\n", i, GetLastError());

    i = 3;
    status = pthread_create(&hComunicacaoAlarmes, NULL, GeraAlarmes, (void*)i);
    if (!status) printf("Thread %d - Comunicacao de dados Alarmes - criada com Id= %0x \n", i, (int)&hComunicacaoAlarmes);
    else printf("Erro na criacao da thread %d! Codigo = %d\n", i, GetLastError());

    /*Criando threads de Retirada de Dados*/
    i = 4;
    status = pthread_create(&hRetiraDadosOtimizacao, NULL, RetiraDadosOtimizacao, (void*)i);
    if (!status) printf("Thread %d - Retirada de dados de Otimizacao - criada com Id= %0x \n", i, (int)&hRetiraDadosOtimizacao);
    else printf("Erro na criacao da thread %d! Codigo = %d\n", i, GetLastError());

    i = 5;
    status = pthread_create(&hRetiraDadosScada, NULL, RetiraDadosProcesso, (void*)i);
    if (!status) printf("Thread %d - Retirada de dados de Scada - criada com Id= %0x \n", i, (int)&hRetiraDadosScada);
    else printf("Erro na criacao da thread %d! Codigo = %d\n", i, GetLastError());

    i = 6;
    status = pthread_create(&hRetiraAlarmes, NULL, RetiraAlarmes, (void*)i);
    if (!status) printf("Thread %d - Retirada de alarmes - criada com Id= %0x \n", i, (int)&hRetiraAlarmes);
    else printf("Erro na criacao da thread %d! Codigo = %d\n", i, GetLastError());

    /*Criando threads de Exibicao de Dados*/
    i = 7;
    status = pthread_create(&hLeituraDadosOtimizacao, NULL, ExibeDadosOtimizacao, (void*)i);
    if (!status) printf("Thread %d - Leitura de dados de Otimizacao - criada com Id= %0x \n", i, (int)&hLeituraDadosOtimizacao);
    else printf("Erro na criacao da thread %d! Codigo = %d\n", i, GetLastError());

    i = 8;
    status = pthread_create(&hLeituraDadosScada, NULL, ExibeDadosProcesso, (void*)i);
    if (!status) printf("Thread %d - Leitura de dados Scada - criada com Id= %0x \n", i, (int)&hLeituraDadosScada);
    else printf("Erro na criacao da thread %d! Codigo = %d\n", i, GetLastError());

    i = 9;
    status = pthread_create(&hLeituraAlarmes, NULL, ExibeAlarmes, (void*)i);
    if (!status) printf("Thread %d - Leitura de alarmes - criada com Id= %0x \n", i, (int)&hLeituraAlarmes);
}

void FecharHanlders()
{
    /*------------------------------------------------------------------------------*/
    /*Fechando todos os handles*/
    CloseHandle(hTimeOut);
    CloseHandle(hEventKeyEsc);
    CloseHandle(hEventKeyC);
    CloseHandle(hEventKeyO);
    CloseHandle(hEventKeyP);
    CloseHandle(hEventKeyA);
    CloseHandle(hEventKeyT);
    CloseHandle(hEventKeyR);
    CloseHandle(hEventKeyL);
    CloseHandle(hEventKeyZ);
    CloseHandle(hSemOcupado);
    CloseHandle(hSemLivre);
    CloseHandle(hMutexBuffer);
    /*------------------------------------------------------------------------------*/
}

/* ======================================================================================================================== */
/*  3 THREADS SECUNDARIA DE ------COMUNICAÇÃO DE DADOS------ */
/*  GERACAO DAS MENSAGENS DO SISTEMA DE OTIMIZACAO E DO SISTEMA SCADA(PROCESSOS INDUSTRIAIS) */
/*  ADICIONA MENSAGENS NA LISTA CIRCULAR*/

void* GeraDadosOtimizacao(void* arg) {
    /*Declarando variaveis locais do LeituraSDCD()*/
    int     index = (int)arg, status, nTipoEvento, k = 0, i = 0, l = 0;

    char    Otimizacao[38], Hora[3], Minuto[3], Segundo[3];

    DWORD   ret;
    BOOL    MemoriaCheia = false;

    /*------------------------------------------------------------------------------*/
    /*Vetor com handles da tarefa*/
    HANDLE  Events[2] = { hEventKeyC, hEventKeyEsc }, SemLivre[2] = { hSemLivre, hEventKeyEsc }, MutexBuffer[2] = { hMutexBuffer, hEventKeyEsc };

    /*------------------------------------------------------------------------------*/
    /*Loop de execucao*/
    while (key != ESC_KEY) {
        for (i = 1; i < 1000000; ++i) {
            /*------------------------------------------------------------------------------*/
            /*Condicao para termino da thread*/
            if (key == ESC_KEY) break;

            /*------------------------------------------------------------------------------*/
            /*Bloqueio e desbloqueio da thread LeituraSDCD*/
            ret = WaitForMultipleObjects(2, Events, FALSE, 1);

            nTipoEvento = ret - WAIT_OBJECT_0;

            if (nTipoEvento == 0) {
                printf("\x1b[31m""BLOQUEADO""\x1b[0m"" - Thread Leitura SDCD\n");

                ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);

                nTipoEvento = ret - WAIT_OBJECT_0;

                printf("\x1b[32m""DESBLOQUEADO""\x1b[0m"" - Thread Leitura SDCD\n");
            }

            /*Condicao para termino do processo*/
            if (nTipoEvento == 1) {
                key = ESC_KEY;
                break;
            }

            /*------------------------------------------------------------------------------*/
            /*Gerando valores aleatorios para os campos referentes ao SDCD*/

            /*Valores de NSEQ - Numero sequencial de 1 ate 999999*/
            for (int j = 0; j < 6; j++) {
                k = i / pow(10, (5 - j));
                k = k % 10;
                Otimizacao[j] = k + '0';
            }
            Otimizacao[6] = '|';

            /*Valores de TIPO - Sempre 11*/
            Otimizacao[7] = '1';
            Otimizacao[8] = '1';

            Otimizacao[9] = '|';

            /*Valores de SP_PRESS - Set point da pressão de injeção de gás (psi)*/
            for (int j = 10; j < 16; j++) {
                if (j == 13) {
                    Otimizacao[j] = '.';
                }
                else {
                    Otimizacao[j] = (rand() % 10) + '0';
                }
            }
            Otimizacao[16] = '|';

            /*Valores de SP_TEMP - Set point da temperatura ddo gás injetado (C)*/
            for (int j = 17; j < 23; j++) {
                if (j == 20) {
                    Otimizacao[j] = '.';
                }
                else {
                    Otimizacao[j] = (rand() % 10) + '0';
                }
            }
            Otimizacao[23] = '|';

            /*Valores de VOL - Volume total do gás a ser injetado*/
            for (int j = 24; j < 29; j++) {
                Otimizacao[j] = (rand() % 10) + '0';
            }
            Otimizacao[29] = '|';

            /*Valores de TIMESTAMP - Com precisao de milisegundos*/
            SYSTEMTIME st;
            GetLocalTime(&st);

            /*Hora*/
            k = 0;
            l = 30;
            for (int j = 0; j < 2; j++) {
                k = st.wHour / pow(10, (1 - j));
                k = k % 10;
                Otimizacao[l] = k + '0';
                l++;
            }
            Otimizacao[32] = ':';

            /*Minuto*/
            k = 0;
            l = 33;
            for (int j = 0; j < 2; j++) {
                k = st.wMinute / pow(10, (1 - j));
                k = k % 10;
                Otimizacao[l] = k + '0';
                l++;
            }
            Otimizacao[35] = ':';

            /*Segundo*/
            k = 0;
            l = 36;
            for (int j = 0; j < 2; j++) {
                k = st.wSecond / pow(10, (1 - j));
                k = k % 10;
                Otimizacao[l] = k + '0';
                l++;
            }

            /*------------------------------------------------------------------------------*/
            /*Condicao para termino da thread*/
            if (key == ESC_KEY) break;

            /*------------------------------------------------------------------------------*/
            /*Gravacao dos dados gerados em memoria*/
            /*Caso memoria esteja cheia a tarefa se bloqueia*/

            /*Esperando o semaforo de espacos livres*/
            ret = WaitForMultipleObjects(2, SemLivre, FALSE, INFINITE);

            nTipoEvento = ret - WAIT_OBJECT_0;

            /*Condição para termino do processo*/
            if (nTipoEvento == 1) {
                key = ESC_KEY;
                break;
            }
            else if (nTipoEvento == 0) {
                /*Conquistando o mutex da secao critica*/
                ret = WaitForMultipleObjects(2, MutexBuffer, FALSE, INFINITE);
                GetLastError();

                nTipoEvento = ret - WAIT_OBJECT_0;

                if (nTipoEvento == 1) {
                    key = ESC_KEY;
                    break;
                }
                else if (nTipoEvento == 0) {
                    /*Gravando dados em memoria RAM*/
                    for (int j = 0; j < 52; j++) {
                        RamBuffer[p_livre][j] = Otimizacao[j];
                    }

                    /*Movendo a posicao de livre para o proximo slot da memoria circular*/
                    p_livre = (p_livre + 1) % RAM;

                    MemoriaCheia = (p_livre == p_ocup);

                    /*Liberando o mutex da secao critica*/
                    status = ReleaseMutex(hMutexBuffer);

                    /*Liberando o semaforo de espacos ocupados*/
                    ReleaseSemaphore(hSemOcupado, 1, NULL);

                    /*Quando a memoria estiver cheia a gravacao de dados e interrompida
                    ate que uma posicao livre apareca - os dados nao escritos sao descartados*/
                    if (MemoriaCheia) {
                        printf("MEMORIA CHEIA\n");

                        printf("\x1b[31m""BLOQUEADO""\x1b[0m"" - Thread Leitura SDCD\n");

                        ret = WaitForMultipleObjects(2, SemLivre, FALSE, INFINITE);
                        GetLastError();

                        nTipoEvento = ret - WAIT_OBJECT_0;

                        if (nTipoEvento == 0) {
                            /*Liberando o semaforo de espacos livres*/
                            ReleaseSemaphore(hSemLivre, 1, NULL);
                            GetLastError();
                        }

                        printf("\x1b[32m""DESBLOQUEADO""\x1b[0m"" - Thread Leitura SDCD\n");
                    }
                }

                nTipoEvento = -1;
            }
            int tempo = rand() % 5;
            int tempoMs = tempo * 1000;
            Sleep(tempoMs);
        } /*fim do for*/
    } /*fim do while*/

    /*------------------------------------------------------------------------------*/
    /*Fechando handles*/
    CloseHandle(MutexBuffer);
    CloseHandle(SemLivre);
    CloseHandle(Events);

    /*------------------------------------------------------------------------------*/
    /*Finalizando a thread leitura do SDCD*/
    printf("Finalizando - Leitura do SDCD\n");
    pthread_exit((void*)index);

    /*Comando nao utilizado, esta aqui apenas para compatibilidade com o Visual Studio da Microsoft*/
    return (void*)index;
}
void* GeraDadosProcesso(void* arg) {
    /*------------------------------------------------------------------------------*/
    /*Declarando variaveis locais da funcao LeituraSCADA()*/
    int     index = (int)arg, status, nTipoEvento, k = 0, i = 0, l = 0;

    char    SCADA[46], Hora[3], Minuto[3], Segundo[3];

    DWORD   ret;
    BOOL    MemoriaCheia;

    /*Vetor com handles esperados */
    HANDLE  Events[2] = { hEventKeyC, hEventKeyEsc }, //BLOQUEIO DE COMUNICAÇÃO DE DADOS E ENCERRAMENTO TOTAL 
            SemLivre[2] = { hSemLivre, hEventKeyEsc }, 
            MutexBuffer[2] = { hMutexBuffer, hEventKeyEsc };

    /*------------------------------------------------------------------------------*/
    /*Loop de execucao*/
    while (key != ESC_KEY) {
        for (i = 1; i < 1000000; ++i) {
            if (key == ESC_KEY) break;
            ret = WaitForMultipleObjects(2, Events, FALSE, 1); //SE C ou ESC FOR SELECIONADO A THREAD NÃO GERA DADOS DE PROCESSO

            nTipoEvento = ret - WAIT_OBJECT_0;

            if (nTipoEvento == 0) {
                printf("\x1b[31m""BLOQUEADO""\x1b[0m"" - Tarefa de comunicação de dados - DadosProcesso\n");

                ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
                GetLastError();

                nTipoEvento = ret - WAIT_OBJECT_0;

                printf("\x1b[32m""DESBLOQUEADO""\x1b[0m"" - Tarefa de comunicação de dados - DadosProcesso\n");
            }

            /*Condicao para termino do processo*/
            if (nTipoEvento == 1) {
                key = ESC_KEY;
                break;
            }

            /*------------------------------------------------------------------------------*/
            /*Gerando valores aleatorios para os campos referentes ao SCADA*/

            /*Valores de NSEQ - Numero sequencial de 1 ate 999999*/
            for (int j = 0; j < 6; j++) {
                k = i / pow(10, (5 - j));
                k = k % 10;
                SCADA[j] = k + '0';
            }
            SCADA[6] = '|';

            /*Valores de TIPO - Sempre 22*/
            SCADA[7] = '2';
            SCADA[8] = '2';

            SCADA[9] = '|';

            /*Valores de PRESSAO_T - Pressão no tubo de extração (psi)*/
            for (int j = 10; j < 16; j++) {
                if (j == 13) {
                    SCADA[j] = '.';
                }
                else {
                    SCADA[j] = (rand() % 10) + '0';
                }
            }
            SCADA[16] = '|';

            /*Valores de TEMP - Temperatura no tubo de extração (C)*/
            for (int j = 17; j < 23; j++) {
                if (j == 20) {
                    SCADA[j] = '.';
                }
                else {
                    SCADA[j] = (rand() % 10) + '0';
                }
            }
            SCADA[23] = '|';

            /*Valores de PRESSAO_G - Pressão no reservatório de gás de injeção (psi)*/
            for (int j = 24; j < 30; j++) {
                if (j == 27) {
                    SCADA[j] = '.';
                }
                else {
                    SCADA[j] = (rand() % 10) + '0';
                }
            }
            SCADA[30] = '|';

            /*Valores de NIVEL - Nível do reservatório de gás de injeção (cm)*/
            for (int j = 31; j < 37; j++) {
                if (j == 33) {
                    SCADA[j] = '.';
                }
                else {
                    SCADA[j] = (rand() % 10) + '0';
                }
            }
            SCADA[37] = '|';

            /*Valores de TIMESTAMP*/
            SYSTEMTIME st;
            GetLocalTime(&st);

            /*Hora*/
            k = 0;
            l = 38;
            for (int j = 0; j < 2; j++) {
                k = st.wHour / pow(10, (1 - j));
                k = k % 10;
                SCADA[l] = k + '0';
                l++;
            }
            SCADA[40] = ':';

            /*Minuto*/
            k = 0;
            l = 41;
            for (int j = 0; j < 2; j++) {
                k = st.wMinute / pow(10, (1 - j));
                k = k % 10;
                SCADA[l] = k + '0';
                l++;
            }
            SCADA[43] = ':';

            /*Segundos*/
            k = 0;
            l = 44;
            for (int j = 0; j < 2; j++) {
                k = st.wSecond / pow(10, (1 - j));
                k = k % 10;
                SCADA[l] = k + '0';
                l++;
            }
            
            /*------------------------------------------------------------------------------*/
            /*Gravacao dos dados gerados em memoria*/
            /*Caso memoria esteja cheia a tarefa se bloqueia*/

            /*Esperando o semaforo de espacos livres*/
            ret = WaitForMultipleObjects(2, SemLivre, FALSE, INFINITE);

            nTipoEvento = ret - WAIT_OBJECT_0;

            /*Condição para termino do processo*/
            if (nTipoEvento == 1) {
                key = ESC_KEY;
                break;
            }
            else if (nTipoEvento == 0) {
                /*Conquistando o mutex da secao critica*/
                ret = WaitForMultipleObjects(2, MutexBuffer, FALSE, INFINITE);

                nTipoEvento = ret - WAIT_OBJECT_0;

                if (nTipoEvento == 1) {
                    key = ESC_KEY;
                    break;
                }
                else if (nTipoEvento == 0) {
                    /*Gravando dados em memoria RAM*/
                    for (int j = 0; j < 52; j++) {
                        RamBuffer[p_livre][j] = SCADA[j];
                    }

                    /*Movendo a posicao de livre para o proximo slot da memoria circular*/
                    p_livre = (p_livre + 1) % RAM;

                    MemoriaCheia = (p_livre == p_ocup);

                    /*Liberando o mutex da secao critica*/
                    status = ReleaseMutex(hMutexBuffer);
                    GetLastError();

                    /*Liberando o semaforo de espacos ocupados*/
                    ReleaseSemaphore(hSemOcupado, 1, NULL);
                    GetLastError();

                    /*Quando a memoria estiver cheia a gravacao de dados e interrompida
                    ate que uma posicao livre apareca - os dados nao escritos sao descartados*/
                    if (MemoriaCheia) {
                        printf("MEMORIA CHEIA\n");

                        printf("\x1b[31m""BLOQUEADO""\x1b[0m"" - Thread Leitura SDCD\n");

                        ret = WaitForMultipleObjects(2, SemLivre, FALSE, INFINITE);
                        GetLastError();

                        nTipoEvento = ret - WAIT_OBJECT_0;

                        if (nTipoEvento == 0) {
                            /*Liberando o semaforo de espacos livres*/
                            ReleaseSemaphore(hSemLivre, 1, NULL);
                            GetLastError();
                        }

                        printf("\x1b[32m""DESBLOQUEADO""\x1b[0m"" - Thread Leitura SDCD\n");
                    }
                }

                nTipoEvento = -1;
            }

            Sleep(500);
        } /*fim do for*/
    } /*fim do while*/

    /*------------------------------------------------------------------------------*/
    /*Fechando handles*/
    CloseHandle(MutexBuffer);
    CloseHandle(SemLivre);
    CloseHandle(Events);

    /*------------------------------------------------------------------------------*/
    /*Finalizando a thread leitura do SDCD*/
    printf("Finalizando - Leitura do SDCD\n");
    pthread_exit((void*)index);

    /*Comando nao utilizado, esta aqui apenas para compatibilidade com o Visual Studio da Microsoft*/
    return (void*)index;
} /*fim do LeituraSCADA*/
void* GeraAlarmes(void* arg) {
    /*------------------------------------------------------------------------------*/
    /*Declarando variaveis locais da funcao LeituraSCADA()*/
    int     index = (int)arg, status, nTipoEvento, k = 0, i = 0, l = 0;

    char    Alarme[27], Hora[3], Minuto[3], Segundo[3];

    DWORD   ret;
    BOOL    MemoriaCheia;

    /*------------------------------------------------------------------------------*/
    /*Vetor com handles da tarefa*/
    HANDLE  Events[2] = { hEventKeyC, hEventKeyEsc }, SemLivre[2] = { hSemLivre, hEventKeyEsc }, MutexBuffer[2] = { hMutexBuffer, hEventKeyEsc };


    /*------------------------------------------------------------------------------*/
    /*Loop de execucao*/
    while (key != ESC_KEY) {
        for (i = 1; i < 1000000; ++i) {
            /*------------------------------------------------------------------------------*/
            /*Condicao para termino da thread*/
            if (key == ESC_KEY) break;

            /*------------------------------------------------------------------------------*/
            /*Bloqueio e desbloqueio da thread LeituraSCADA*/
            ret = WaitForMultipleObjects(2, Events, FALSE, 1);
            GetLastError();

            nTipoEvento = ret - WAIT_OBJECT_0;

            if (nTipoEvento == 0) {
                printf("\x1b[31m""BLOQUEADO""\x1b[0m"" - Thread Leitura SCADA\n");

                ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
                GetLastError();

                nTipoEvento = ret - WAIT_OBJECT_0;

                printf("\x1b[32m""DESBLOQUEADO""\x1b[0m"" - Thread Leitura SCADA\n");
            }

            /*Condicao para termino do processo*/
            if (nTipoEvento == 1) {
                key = ESC_KEY;
                break;
            }

            /*------------------------------------------------------------------------------*/
            /*Gerando valores aleatorios para os campos referentes ao SCADA*/

            /*Valores de NSEQ - Numero sequencial de 1 ate 999999*/
            for (int j = 0; j < 6; j++) {
                k = i / pow(10, (5 - j));
                k = k % 10;
                Alarme[j] = k + '0';
            }
            Alarme[6] = '|';

            /*Valores de TIPO - Sempre 55*/
            Alarme[7] = '5';
            Alarme[8] = '5';

            Alarme[9] = '|';

            /*Valores de ID - Identificador do Alarme*/
            for (int j = 10; j < 14; j++) {
                Alarme[j] = (rand() % 10) + '0';
            }
            Alarme[14] = '|';

            /*Valores de PRIORIDADE - Prioridade do Alarme*/
            for (int j = 15; j < 18; j++) {
                Alarme[j] = (rand() % 10) + '0';
            }
            Alarme[18] = '|';

            /*Valores de TIMESTAMP*/
            SYSTEMTIME st;
            GetLocalTime(&st);

            /*Hora*/
            k = 0;
            l = 19;
            for (int j = 0; j < 2; j++) {
                k = st.wHour / pow(10, (1 - j));
                k = k % 10;
                Alarme[l] = k + '0';
                l++;
            }
            Alarme[21] = ':';

            /*Minuto*/
            k = 0;
            l = 22;
            for (int j = 0; j < 2; j++) {
                k = st.wMinute / pow(10, (1 - j));
                k = k % 10;
                Alarme[l] = k + '0';
                l++;
            }
            Alarme[24] = ':';

            /*Segundos*/
            k = 0;
            l = 25;
            for (int j = 0; j < 2; j++) {
                k = st.wSecond / pow(10, (1 - j));
                k = k % 10;
                Alarme[l] = k + '0';
                l++;
            }

            /*------------------------------------------------------------------------------*/
            /*Condicao para termino da thread*/
            if (key == ESC_KEY) break;

            /*------------------------------------------------------------------------------*/
            /*Temporizador - Mensagens do SDCD se repetem de 500 em 500 ms*/
            /*475 pois o tempo medio da conquista de mutex, semaforo, criacao e gravacao dos dados na lista demora 025 ms*/
            WaitForSingleObject(hTimeOut, 475);
            GetLastError();

            /*------------------------------------------------------------------------------*/
            /*Gravacao dos dados gerados em memoria*/
            /*Caso memoria esteja cheia a tarefa se bloqueia*/

            /*Esperando o semaforo de espacos livres*/
            ret = WaitForMultipleObjects(2, SemLivre, FALSE, INFINITE);

            nTipoEvento = ret - WAIT_OBJECT_0;

            /*Condição para termino do processo*/
            if (nTipoEvento == 1) {
                key = ESC_KEY;
                break;
            }
            else if (nTipoEvento == 0) {
                /*Conquistando o mutex da secao critica*/
                ret = WaitForMultipleObjects(2, MutexBuffer, FALSE, INFINITE);

                nTipoEvento = ret - WAIT_OBJECT_0;

                if (nTipoEvento == 1) {
                    key = ESC_KEY;
                    break;
                }
                else if (nTipoEvento == 0) {
                    /*Gravando dados em memoria RAM*/
                    for (int j = 0; j < 52; j++) {
                        RamBuffer[p_livre][j] = Alarme[j];
                    }

                    /*Movendo a posicao de livre para o proximo slot da memoria circular*/
                    p_livre = (p_livre + 1) % RAM;

                    MemoriaCheia = (p_livre == p_ocup);

                    /*Liberando o mutex da secao critica*/
                    status = ReleaseMutex(hMutexBuffer);
                    GetLastError();

                    /*Liberando o semaforo de espacos ocupados*/
                    ReleaseSemaphore(hSemOcupado, 1, NULL);
                    GetLastError();

                    /*Quando a memoria estiver cheia a gravacao de dados e interrompida
                    ate que uma posicao livre apareca - os dados nao escritos sao descartados*/
                    if (MemoriaCheia) {
                        printf("MEMORIA CHEIA\n");

                        printf("\x1b[31m""BLOQUEADO""\x1b[0m"" - Thread Leitura Alarmes\n");

                        ret = WaitForMultipleObjects(2, SemLivre, FALSE, INFINITE);
                        GetLastError();

                        nTipoEvento = ret - WAIT_OBJECT_0;

                        if (nTipoEvento == 0) {
                            /*Liberando o semaforo de espacos livres*/
                            ReleaseSemaphore(hSemLivre, 1, NULL);
                            GetLastError();
                        }

                        printf("\x1b[32m""DESBLOQUEADO""\x1b[0m"" - Thread Leitura Alarmes\n");
                    }
                }

                nTipoEvento = -1;
            }

            Sleep(500);
        } /*fim do for*/
    } /*fim do while*/

    /*------------------------------------------------------------------------------*/
    /*Fechando handles*/
    CloseHandle(MutexBuffer);
    CloseHandle(SemLivre);
    CloseHandle(Events);

    /*------------------------------------------------------------------------------*/
    /*Finalizando a thread leitura do SDCD*/
    printf("Finalizando - Leitura do SDCD\n");
    pthread_exit((void*)index);

    /*Comando nao utilizado, esta aqui apenas para compatibilidade com o Visual Studio da Microsoft*/
    return (void*)index;
} 

/* ======================================================================================================================== */
/*  3 THREADS SECUNDARIA DE ------RETIRADA DE DADOS------ */
/*  RETIRADA DADOS DA LISTA CIRCULAR EM MEMORIA*/
/*  Dados de otimizacao sao escritos no arquivo circular*/
/*  Processos e Alarmes são enviados para as threads de Exibicao*/

void* RetiraDadosOtimizacao(void* arg) {
    int     index = (int)arg;
    return (void*)index;
} // Parte 2
void* RetiraDadosProcesso(void* arg) {
    int     index = (int)arg;
    return (void*)index;
}
void* RetiraAlarmes(void* arg) {
    /*------------------------------------------------------------------------------*/
    /*Declarando variaveis locais da funcao RetiraDados()*/
    int     index = (int)arg, status, i, nTipoEvento;

    char    SCADA[46];

    DWORD   ret, dwBytesLidos, dwBytesToWrite, dwBytesWritten, dwPos;

    /*------------------------------------------------------------------------------*/
    /*Vetor com handles da tarefa*/
    HANDLE  Events[2] = { hEventKeyA, hEventKeyEsc }, SemOcupado[2] = { hSemOcupado, hEventKeyEsc }, MutexBuffer[2] = { hMutexBuffer, hEventKeyEsc };

    /*------------------------------------------------------------------------------*/
    /*Loop de execucao*/
    while (key != ESC_KEY) {
        /*------------------------------------------------------------------------------*/
        /*Bloqueio e desbloqueio da thread RetiraAlarmes*/
        ret = WaitForMultipleObjects(2, Events, FALSE, 1);
        GetLastError();

        nTipoEvento = ret - WAIT_OBJECT_0;

        if (nTipoEvento == 0) {
            printf("\x1b[31m""BLOQUEADO""\x1b[0m"" - Thread Captura de alarmes\n");

            ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
            GetLastError();

            nTipoEvento = ret - WAIT_OBJECT_0;

            printf("\x1b[32m""DESBLOQUEADO""\x1b[0m"" - Thread Captura alarmes\n");
        }

        /*Condicao para termino do processo*/
        if (nTipoEvento == 1) {
            key = ESC_KEY;
        }

        /*------------------------------------------------------------------------------*/
        /*Leitura dos dados gerados em memoria*/

        /*Esperando o semaforo de espacos ocupados*/
        ret = WaitForMultipleObjects(2, SemOcupado, FALSE, INFINITE);
        GetLastError();

        nTipoEvento = ret - WAIT_OBJECT_0;

        /*Condição para termino do processo*/
        if (nTipoEvento == 1) {
            key = ESC_KEY;
        }
        else if (nTipoEvento == 0) {
            /*Conquistando o mutex da secao critica*/
            ret = WaitForMultipleObjects(2, MutexBuffer, FALSE, INFINITE);
            GetLastError();

            nTipoEvento = ret - WAIT_OBJECT_0;

            if (nTipoEvento == 1) {
                key = ESC_KEY;
            }
            else if (nTipoEvento == 0) {
                /*Selecao dos dados apenas de tipo 1*/
                if (RamBuffer[p_ocup][7] == '2') {
                    /*Lendo dados gravados em memoria*/
                    for (int j = 0; j < 46; j++) {
                        SCADA[j] = RamBuffer[p_ocup][j];
                    }

                    /*Movendo a posicao de livre para o proximo slot da memoria circular*/
                    p_ocup = (p_ocup + 1) % RAM;

                    /*Liberando o semaforo de espacos livres*/
                    ReleaseSemaphore(hSemLivre, 1, NULL);
                    GetLastError();
                }
                else {
                    /*Liberando o semaforo de espacos ocupados*/
                    ReleaseSemaphore(hSemOcupado, 1, NULL);
                    GetLastError();
                }

                /*Liberando o mutex da secao critica*/
                status = ReleaseMutex(hMutexBuffer);
                GetLastError();
            }
        }
    } /*fim do while*/

    /*------------------------------------------------------------------------------*/
    /*Fechando handles*/
    CloseHandle(MutexBuffer);
    CloseHandle(SemOcupado);
    CloseHandle(Events);

    /*------------------------------------------------------------------------------*/
    /*Finalizando a thread de captura de alarmes*/
    printf("Finalizando - Captura de alarmes\n");
    pthread_exit((void*)index);

    /*Comando nao utilizado, esta aqui apenas para compatibilidade com o Visual Studio da Microsoft*/
    return (void*)index;
}


/* ======================================================================================================================== */
/*  3 THREAD SECUNDARIA DE EXIBIÇÃO DE DADOS E ALARMES*/
/*  Dados de otimizacao sao buscados no arquivo circular*/
/*  Processos e Alarmes são recebidos das threads de Retirada de dados*/

void* ExibeDadosOtimizacao(void* arg) {
    int     index = (int)arg;
    return (void*)index;
} // Parte 2
void* ExibeDadosProcesso(void* arg) {
    int     index = (int)arg;
    return (void*)index;
}
void* ExibeAlarmes(void* arg) {
    int     index = (int)arg;
    return (void*)index;
}

void* RetiraDados(void* arg) {
    /*------------------------------------------------------------------------------*/
    /*Declarando variaveis locais da funcao de RetiraDados()*/
    int     index = (int)arg, status, i, nTipoEvento;

    char    SDCD[38];

    DWORD   ret, dwBytesToWrite, dwBytesWritten, dwPos;

    /*------------------------------------------------------------------------------*/
    /*Vetor com handles da tarefa*/
    HANDLE Events[2] = { hEventKeyP, hEventKeyEsc }, SemOcupado[2] = { hSemOcupado, hEventKeyEsc }, MutexBuffer[2] = { hMutexBuffer, hEventKeyEsc };

    /*------------------------------------------------------------------------------*/
    /*Loop de execucao*/
    while (key != ESC_KEY) {
        nTipoEvento = -1;

        /*------------------------------------------------------------------------------*/
        /*Bloqueio e desbloqueio da thread RetiraDados*/
        ret = WaitForMultipleObjects(2, Events, FALSE, 1);
        GetLastError();

        nTipoEvento = ret - WAIT_OBJECT_0;

        if (nTipoEvento == 0) {
            printf("\x1b[31m""BLOQUEADO""\x1b[0m"" - Thread Captura de dados do processo\n");

            ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
            GetLastError();

            nTipoEvento = ret - WAIT_OBJECT_0;

            printf("\x1b[32m""DESBLOQUEADO""\x1b[0m"" - Thread Captura de dados do processo\n");
        }

        /*Condicao para termino do processo*/
        if (nTipoEvento == 1) {
            key = ESC_KEY;
        }

        /*------------------------------------------------------------------------------*/
        /*Leitura dos dados gerados e gravados em memoria do SDCD*/

        /*Esperando o semaforo de espacos ocupados*/
        ret = WaitForMultipleObjects(2, SemOcupado, FALSE, INFINITE);
        GetLastError();

        nTipoEvento = ret - WAIT_OBJECT_0;

        /*Condição para termino do processo*/
        if (nTipoEvento == 1) {
            key = ESC_KEY;
        }
        else if (nTipoEvento == 0) {
            /*Conquistando o mutex da secao critica*/
            ret = WaitForMultipleObjects(2, MutexBuffer, FALSE, INFINITE);
            GetLastError();

            nTipoEvento = ret - WAIT_OBJECT_0;

            if (nTipoEvento == 1) {
                key = ESC_KEY;
            }
            else if (nTipoEvento == 0) {
                /*Selecao dos dados apenas de tipo 1*/
                if (RamBuffer[p_ocup][7] == '1') {
                    /*Lendo dados gravados em memoria*/
                    char aux[40];
                    for (int j = 0; j < 40; j++) {
                        if (j < 38)
                        {
                            SDCD[j] = RamBuffer[p_ocup][j];
                            aux[j] = SDCD[j];
                        }
                        else if (j == 38)
                        {
                            aux[j] = '\r';
                        }
                        else if (j == 39)
                        {
                            aux[j] = '\n';
                        }
                    }                   
                }
                else {
                    /*Liberando o semaforo de espacos ocupados*/
                    ReleaseSemaphore(hSemOcupado, 1, NULL);
                    GetLastError();
                }

                /*Liberando o mutex da secao critica*/
                status = ReleaseMutex(hMutexBuffer);
                GetLastError();
            }
        }
    } /*fim do while*/

    /*------------------------------------------------------------------------------*/
    /*Fechando handles*/
    CloseHandle(MutexBuffer);
    CloseHandle(SemOcupado);
    CloseHandle(Events);

    /*------------------------------------------------------------------------------*/
    /*Finalizando a thread de captura de dados do processo*/
    printf("Finalizando - Captura de dados do processo\n");
    pthread_exit((void*)index);

    /*Comando nao utilizado, esta aqui apenas para compatibilidade com o Visual Studio da Microsoft*/
    return (void*)index;
}
