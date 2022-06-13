#define HAVE_STRUCT_TIMESPEC
#define WIN32_LEAN_AND_MEAN

#define _CHECKERROR	    1				                                        

#define RAM             100                                                     
#define	ESC_KEY			27                                                      
#define FILE_SIZE       200  

#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <conio.h>		                                                        
#include <math.h>                                                               
#include <time.h>
#include <string.h>
#include "CheckForError.h"        

void* LeituraSDCD(void* arg);
void* LeituraSCADA(void* arg);
void* LeituraAlarmes(void* arg);
void* RetiraDados(void* arg);
void* RetiraAlarmes(void* arg);

char    RamBuffer[RAM][52], key;
int     p_ocup = 0, p_livre = 0;
int     n_mensagem = 0;

HANDLE hMutexBuffer;
HANDLE hSemLivre, hSemOcupado, hArquivo;
HANDLE hEventKeyC, hEventKeyO, hEventKeyP, hEventKeyA, hEventKeyT, hEventKeyR, hEventKeyL, hEventKeyZ, hEventKeyEsc, hEventMailslotAlarmeA, hTimeOut;

HANDLE hArquivoCheio;
// HANDLE hMailslotClienteAlarmeA;
// HANDLE hFile;

int main() {
    int     i, status;

    DWORD   ret;

    hMutexBuffer = CreateMutex(NULL, FALSE, L"MutexBuffer");

    hSemLivre = CreateSemaphore(NULL, RAM, RAM, L"SemLivre");

    hSemOcupado = CreateSemaphore(NULL, 0, RAM, L"SemOcupado");

    hArquivo = CreateSemaphore(NULL, FILE_SIZE, FILE_SIZE, L"SemArquivo");

    /*Criando objetos do tipo eventos*/
    hEventKeyC = CreateEvent(NULL, FALSE, FALSE, L"KeyC");

    hEventKeyO = CreateEvent(NULL, FALSE, FALSE, L"KeyO");

    hEventKeyP = CreateEvent(NULL, FALSE, FALSE, L"KeyP");

    hEventKeyA = CreateEvent(NULL, FALSE, FALSE, L"KeyA");

    hEventKeyT = CreateEvent(NULL, FALSE, FALSE, L"KeyT");

    hEventKeyR = CreateEvent(NULL, FALSE, FALSE, L"KeyR");

    hEventKeyL = CreateEvent(NULL, FALSE, FALSE, L"KeyL");

    hEventKeyZ = CreateEvent(NULL, FALSE, FALSE, L"KeyZ");

    hEventKeyEsc = CreateEvent(NULL, TRUE, FALSE, L"KeyEsc");

    // hEventMailslotAlarmeA = CreateEvent(NULL, FALSE, FALSE, L"MailslotAlarme");

    hTimeOut = CreateEvent(NULL, FALSE, FALSE, L"TimeOut");

    hArquivoCheio = CreateEvent(NULL, FALSE, TRUE, L"ArquivoCheio");

    /*Threads*/

    /*Handles threads*/
    pthread_t hLeituraSDCD, hLeituraSCADA, hLeituraAlarmes, hRetiraDados, hRetiraAlarmes;

    /*Criando threads secundarias*/
    i = 1;
    status = pthread_create(&hLeituraSDCD, NULL, LeituraSDCD, (void*)i);
    if (!status) printf("Thread %d - Leitura SDCD - criada com Id= %0x \n", i, (int)&hLeituraSDCD);
    else printf("Erro na criacao da thread %d! Codigo = %d\n", i, GetLastError());

    i = 2;
    status = pthread_create(&hLeituraSCADA, NULL, LeituraSCADA, (void*)i);
    if (!status) printf("Thread %d - Leitura SCADA - criada com Id= %0x \n", i, (int)&hLeituraSCADA);
    else printf("Erro na criacao da thread %d! Codigo = %d\n", i, GetLastError());

    i = 3;
    status = pthread_create(&hLeituraAlarmes, NULL, LeituraAlarmes, (void*)i);
    if (!status) printf("Thread %d - Leitura Alarmes - criada com Id= %0x \n", i, (int)&hLeituraAlarmes);
    else printf("Erro na criacao da thread %d! Codigo = %d\n", i, GetLastError());

    i = 4;
    status = pthread_create(&hRetiraDados, NULL, RetiraDados, (void*)i);
    if (!status) printf("Thread %d - Retirada de dados do processo - criada com Id= %0x \n", i, (int)&hRetiraDados);
    else printf("Erro na criacao da thread %d! Codigo = %d\n", i, GetLastError());

    i = 5;
    status = pthread_create(&hRetiraAlarmes, NULL, RetiraAlarmes, (void*)i);
    if (!status) printf("Thread %d - Retirada de alarmes - criada com Id= %0x \n\n", i, (int)&hRetiraAlarmes);
    else printf("Erro na criacao da thread %d! Codigo = %d\n", i, GetLastError());

    /*Processos*/

    /*Nomeando terminal da thread primaria*/
    SetConsoleTitle(L"| Terminal Principal |");

    /*Criando processos filhos*/
    STARTUPINFO si;				                                               /*StartUpInformation para novo processo*/
    PROCESS_INFORMATION NewProcess;	                                           /*Informacoes sobre novo processo criado*/

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);	                                                       /*Tamanho da estrutura em bytes*/

    /*Processo de exibicao de dados - Terminal Dados do Processo*/
    status = CreateProcess(
        L"..\\Debug\\ExibicaoDados.exe",                                       /*Caminho relativo do arquivo executavel*/
        NULL,                                                                  /*Apontador para parametros de linha de comando*/
        NULL,                                                                  /*Apontador para descritor de segurança*/
        NULL,                                                                  /*Apontador para threads do processo*/
        FALSE,	                                                               /*Heranca de handles*/
        CREATE_NEW_CONSOLE,	                                                   /*Flags de criação - Criar novo console no caso*/
        NULL,	                                                               /*Heranca do ambiente de execucao*/
        L"..\\Debug",                                                          /*Diretorio do arquivo executavel*/
        &si,			                                                       /*lpStartUpInfo*/
        &NewProcess);	                                                       /*lpProcessInformation*/
    if (!status) {
        printf("Erro na criacao do Terminal de Dados do Processo! Codigo = %d\n", GetLastError());
    }
    else {
        printf("Processo de exibicao de dados e Terminal de Dados do Processo criados\n", GetLastError());
    }

    /*Processo de exibicao de alarmes - Terminal Alarmes*/
    status = CreateProcess(
        L"..\\Debug\\ExibicaoAlarmes.exe",                                     /*Caminho relativo do arquivo executavel*/
        NULL,                                                                  /*Apontador para parametros de linha de comando*/
        NULL,                                                                  /*Apontador para descritor de segurança*/
        NULL,                                                                  /*Apontador para threads do processo*/
        FALSE,	                                                               /*Heranca de handles*/
        CREATE_NEW_CONSOLE,	                                                   /*Flags de criação - Criar novo console no caso*/
        NULL,	                                                               /*Heranca do ambiente de execucao*/
        L"..\\Debug",                                                          /*Diretorio do arquivo executavel*/
        &si,			                                                       /*lpStartUpInfo*/
        &NewProcess);	                                                       /*lpProcessInformation*/
    if (!status) {
        printf("Erro na criacao do Terminal Alarmes! Codigo = %d\n", GetLastError());
    }
    else {
        printf("Processo de exibicao de alarmes e Terminal Alarmes criados\n\n", GetLastError());
    }

    /*------------------------------------------------------------------------------*/
    /*Tratando inputs do teclado*/
    while (key != ESC_KEY) {
        key = _getch();
        switch (key) {
        case 'c':
        case 'C':
            SetEvent(hEventKeyC);
            GetLastError();
            printf("Tecla C pressionada\n");
            break;
        case 'o':
        case 'O':
            SetEvent(hEventKeyO);
            GetLastError();
            printf("Tecla O pressionada\n");
            break;
        case 'p':
        case 'P':
            SetEvent(hEventKeyP);
            GetLastError();
            printf("Tecla P pressionada\n");
            break;
        case 'a':
        case 'A':
            SetEvent(hEventKeyA);
            GetLastError();
            printf("Tecla A pressionada\n");
            break;
        case 't':
        case 'T':
            SetEvent(hEventKeyT);
            GetLastError();
            printf("Tecla T pressionada\n");
            break;
        case 'r':
        case 'R':
            SetEvent(hEventKeyR);
            GetLastError();
            printf("Tecla R pressionada\n");
            break;
        case 'l':
        case 'L':
            SetEvent(hEventKeyL);
            GetLastError();
            printf("Tecla L pressionada\n");
            break;
        case 'z':
        case 'Z':
            SetEvent(hEventKeyZ);
            GetLastError();
            printf("Tecla Z pressionada\n");
            break;
        case ESC_KEY:
            printf("\n");
            SetEvent(hEventKeyEsc);
            GetLastError();
            break;
        default:
            printf("Tecla pressionada não tem uma função!\n");
            break;
        } /*fim do switch*/
    } /*fim do while*/

    /*------------------------------------------------------------------------------*/
    /*Aguardando o fim dos processos e threads antes de encerrar a main*/
    WaitForSingleObject(hTimeOut, 5000);
    GetLastError();

    /*------------------------------------------------------------------------------*/
    /*Fechando handles*/
    CloseHandle(hTimeOut);
    CloseHandle(hEventMailslotAlarmeA);
    CloseHandle(hEventKeyEsc);
    CloseHandle(hEventKeyC);
    CloseHandle(hEventKeyO);
    CloseHandle(hEventKeyP);
    CloseHandle(hEventKeyA);
    CloseHandle(hEventKeyT);
    CloseHandle(hEventKeyR);
    CloseHandle(hEventKeyL);
    CloseHandle(hEventKeyZ);
    CloseHandle(hArquivoCheio);
    CloseHandle(hSemOcupado);
    CloseHandle(hSemLivre);
    CloseHandle(hArquivo);
    CloseHandle(hMutexBuffer);

    /*------------------------------------------------------------------------------*/
    printf("Finalizando - Inputs do teclado\n");
    return EXIT_SUCCESS;

} /*fim da funcao main*/

void* LeituraSDCD(void* arg) {
    /*Declarando variaveis locais do LeituraSDCD()*/
    int     index = (int)arg, status, nTipoEvento, k = 0, i = 0, l = 0;

    char    SDCD[38], Hora[3], Minuto[3], Segundo[3];

    DWORD   ret;

    BOOL    Memory;

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
            GetLastError();

            nTipoEvento = ret - WAIT_OBJECT_0;

            if (nTipoEvento == 0) {
                printf("\x1b[31m""BLOQUEADO""\x1b[0m"" - Thread Leitura SDCD\n");

                ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
                GetLastError();

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
                SDCD[j] = k + '0';
            }
            SDCD[6] = '|';

            /*Valores de TIPO - Sempre 11*/
            SDCD[7] = '1';
            SDCD[8] = '1';

            SDCD[9] = '|';

            /*Valores de SP_PRESS - Set point da pressão de injeção de gás (psi)*/
            for (int j = 10; j < 16; j++) {
                if (j == 13) {
                    SDCD[j] = '.';
                }
                else {
                    SDCD[j] = (rand() % 10) + '0';
                }
            }
            SDCD[16] = '|';

            /*Valores de SP_TEMP - Set point da temperatura ddo gás injetado (C)*/
            for (int j = 17; j < 23; j++) {
                if (j == 20) {
                    SDCD[j] = '.';
                }
                else {
                    SDCD[j] = (rand() % 10) + '0';
                }
            }
            SDCD[23] = '|';

            /*Valores de VOL - Volume total do gás a ser injetado*/
            for (int j = 24; j < 29; j++) {
                SDCD[j] = (rand() % 10) + '0';
            }
            SDCD[29] = '|';

            /*Valores de TIMESTAMP - Com precisao de milisegundos*/
            SYSTEMTIME st;
            GetLocalTime(&st);

            /*Hora*/
            k = 0;
            l = 30;
            for (int j = 0; j < 2; j++) {
                k = st.wHour / pow(10, (1 - j));
                k = k % 10;
                SDCD[l] = k + '0';
                l++;
            }
            SDCD[32] = ':';

            /*Minuto*/
            k = 0;
            l = 33;
            for (int j = 0; j < 2; j++) {
                k = st.wMinute / pow(10, (1 - j));
                k = k % 10;
                SDCD[l] = k + '0';
                l++;
            }
            SDCD[35] = ':';

            /*Segundo*/
            k = 0;
            l = 36;
            for (int j = 0; j < 2; j++) {
                k = st.wSecond / pow(10, (1 - j));
                k = k % 10;
                SDCD[l] = k + '0';
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
            GetLastError();

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
                        RamBuffer[p_livre][j] = SDCD[j];
                    }

                    /*Movendo a posicao de livre para o proximo slot da memoria circular*/
                    p_livre = (p_livre + 1) % RAM;

                    Memory = (p_livre == p_ocup);

                    /*Liberando o mutex da secao critica*/
                    status = ReleaseMutex(hMutexBuffer);
                    GetLastError();

                    /*Liberando o semaforo de espacos ocupados*/
                    ReleaseSemaphore(hSemOcupado, 1, NULL);
                    GetLastError();

                    /*Quando a memoria estiver cheia a gravacao de dados e interrompida
                    ate que uma posicao livre apareca - os dados nao escritos sao descartados*/
                    if (Memory) {
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
/*  THREAD SECUNDARIA DE LEITURA SCADA*/
/*  GERACAO DE VALORES DE MENSAGENS DO SCADA*/

void* LeituraSCADA(void* arg) {
    /*------------------------------------------------------------------------------*/
    /*Declarando variaveis locais da funcao LeituraSCADA()*/
    int     index = (int)arg, status, nTipoEvento, k = 0, i = 0, l = 0;

    char    SCADA[46], Hora[3], Minuto[3], Segundo[3];

    DWORD   ret;

    BOOL    Memory;

    /*------------------------------------------------------------------------------*/
    /*Vetor com handles da tarefa*/
    HANDLE  Events[2] = { hEventKeyO, hEventKeyEsc }, SemLivre[2] = { hSemLivre, hEventKeyEsc }, MutexBuffer[2] = { hMutexBuffer, hEventKeyEsc };

    /*------------------------------------------------------------------------------*/
    /*Criando arquivo para cliente mailslot
    WaitForSingleObject(hEventMailslotAlarmeA, INFINITE);
    GetLastError();*/

    /*hMailslotClienteAlarmeA = CreateFile(L"\\\\.\\mailslot\\MailslotAlarmeA", GENERIC_WRITE, FILE_SHARE_READ,
        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    CheckForError(hMailslotClienteAlarmeA != INVALID_HANDLE_VALUE);*/

    /*------------------------------------------------------------------------------*/
    /*Tempo de inicio
    ticks1 = GetTickCount();
    ticksA = GetTickCount();*/

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
            GetLastError();

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
                        RamBuffer[p_livre][j] = SCADA[j];
                    }

                    /*Movendo a posicao de livre para o proximo slot da memoria circular*/
                    p_livre = (p_livre + 1) % RAM;

                    Memory = (p_livre == p_ocup);

                    /*Liberando o mutex da secao critica*/
                    status = ReleaseMutex(hMutexBuffer);
                    GetLastError();

                    /*Liberando o semaforo de espacos ocupados*/
                    ReleaseSemaphore(hSemOcupado, 1, NULL);
                    GetLastError();

                    /*Quando a memoria estiver cheia a gravacao de dados e interrompida
                    ate que uma posicao livre apareca - os dados nao escritos sao descartados*/
                    if (Memory) {
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

/* ======================================================================================================================== */
/*  THREAD SECUNDARIA DE LEITURA ALARMES*/
/*  GERACAO DE VALORES DE ALARMES*/

void* LeituraAlarmes(void* arg) {
    /*------------------------------------------------------------------------------*/
    /*Declarando variaveis locais da funcao LeituraSCADA()*/
    int     index = (int)arg, status, nTipoEvento, k = 0, i = 0, l = 0;

    char    Alarme[27], Hora[3], Minuto[3], Segundo[3];

    DWORD   ret;

    BOOL    Memory;

    /*------------------------------------------------------------------------------*/
    /*Vetor com handles da tarefa*/
    HANDLE  Events[2] = { hEventKeyA, hEventKeyEsc }, SemLivre[2] = { hSemLivre, hEventKeyEsc }, MutexBuffer[2] = { hMutexBuffer, hEventKeyEsc };

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
            GetLastError();

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
                        RamBuffer[p_livre][j] = Alarme[j];
                    }

                    /*Movendo a posicao de livre para o proximo slot da memoria circular*/
                    p_livre = (p_livre + 1) % RAM;

                    Memory = (p_livre == p_ocup);

                    /*Liberando o mutex da secao critica*/
                    status = ReleaseMutex(hMutexBuffer);
                    GetLastError();

                    /*Liberando o semaforo de espacos ocupados*/
                    ReleaseSemaphore(hSemOcupado, 1, NULL);
                    GetLastError();

                    /*Quando a memoria estiver cheia a gravacao de dados e interrompida
                    ate que uma posicao livre apareca - os dados nao escritos sao descartados*/
                    if (Memory) {
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

/* ======================================================================================================================== */
/*  THREAD SECUNDARIA DE CAPTURA DE DADOS DO PROCESSO*/
/*  CAPTURA DE DADOS EM MEMORIA PARA GRAVACAO EM ARQUIVO*/
/*  THREAD COSUMIDORA, RETIRA AS MENSAGENS PRODUZIDAS PELA TAREFA DE LEITURA DO SDCD*/
/*  SINALIZACAO DA GRAVACAO A TAREFA DE EXIBICAO DE DADOS DE PROCESSO*/

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

/* ======================================================================================================================== */
/*  THREAD SECUNDARIA DE CAPTURA DE ALARMES*/
/*  RETIRA AS MENSAGENS DE ALARMES NAO CRITICOS DA MEMORIA*/
/*  REPASSAGEM DAS MESMAS PARA A TAREFA DE EXIBICAO DE ALARMES*/

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