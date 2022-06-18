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
#include <string>

#define _CHECKERROR	    1				                                        
#define RAM             100                                                     
#define	ESC_KEY			27                                                      
#define FILE_SIZE       200     

//Cabeçalhos de funções
void CriarObjetos();
void FecharHandlers();
void CriarThreadsSecundarias();
void CapturarTeclado();
void CriarProcessosExibicao();

void GeraDadosOtimizacao(int i);
void GeraDadosProcesso(int i);
void GeraAlarmes(int i);

void* GeraDados(void* arg);
void* RetiraDadosOtimizacao(void* arg);
void* RetiraDadosProcesso(void* arg);
void* RetiraAlarmes(void* arg);

char CircularList[RAM][47], key;
int p_ocup = 0, p_livre = 0;
int n_mensagem = 0;

//Objetos do escopo global
HANDLE hMutexBuffer, hMutexConsole;
HANDLE hSemLivre, hSemOcupado;
HANDLE hEventKeyC, hEventKeyO, hEventKeyP, hEventKeyA, hEventKeyT, hEventKeyR, hEventKeyL, hEventKeyZ, hEventKeyEsc, hTimeOut;
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

int main() {

	WaitForSingleObject(hMutexConsole, INFINITE);
	setlocale(LC_ALL, "Portuguese");
	SetConsoleTitle(L"| Terminal Principal |");                              /*Altera nome do terminal da thread primaria*/
	ReleaseMutex(hMutexConsole);

	CriarObjetos();
	CriarProcessosExibicao();
	CriarThreadsSecundarias();
	CapturarTeclado();                                                      /*Faz a leitura do teclado enquanto a execução está ativa. A tecla ESC encerra a execução*/
	WaitForSingleObject(hTimeOut, 5000);
	FecharHandlers();                                                       /*Aguardando o fim dos processos e threads antes de encerrar*/

	WaitForSingleObject(hMutexConsole, INFINITE);
	printf("Finalizando - Inputs do teclado\n");
	ReleaseMutex(hMutexConsole);

	WaitForSingleObject(hMutexConsole, INFINITE);
	ReleaseMutex(hMutexConsole);
	return EXIT_SUCCESS;                                                   /*fim da funcao main*/
}

void CriarObjetos() {

	// Mutexes
	hMutexBuffer = CreateMutex(NULL, FALSE, L"MutexBuffer");                /*Para controlar o acesso ao buffer de mensagem*/
	hMutexConsole = CreateMutex(NULL, FALSE, L"MutexConsole");              /*Para controlar o acesso as escritas do console*/

	// Semáforos
	hSemLivre = CreateSemaphore(NULL, RAM, RAM, L"SemLivre");               /*Espaço Livre*/
	hSemOcupado = CreateSemaphore(NULL, 0, RAM, L"SemOcupado");             /*Espaço ocupado*/

	// Eventos
	//segurança nula, reset automatico, não inicializado, chaveUnica
	hTimeOut = CreateEvent(NULL, FALSE, FALSE, L"TimeOut");
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
	bool statusProcess;
	ZeroMemory(&startup_info, sizeof(startup_info));                           /* Zera um bloco de memória localizado em &si passando o comprimento a ser zerado. */
	startup_info.cb = sizeof(startup_info);	                                   /*Tamanho da estrutura em bytes*/

	/*Processo de exibicao de dados da plataforma petrolifica- Terminal Dados Scada do Processo*/
	startup_info.lpTitle = L"TERMINAL A - Exibicao de Dados do Processo";
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

	WaitForSingleObject(hMutexConsole, INFINITE);
	if (!statusProcess) {
		printf("Erro na criacao do Terminal de Dados do Processo! Codigo = %d\n", GetLastError());
	}
	else {
		printf("Processo de exibicao de Dados de Processo e Terminal criados\n");
	}
	ReleaseMutex(hMutexConsole);


	/*Processo de exibicao de dados do novo sistema de otimizacao - Terminal DadosOtimizacao do Processo*/
	startup_info.lpTitle = L"TERMINAL B - Exibicao de Dados de Otimizacao";
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

	WaitForSingleObject(hMutexConsole, INFINITE);
	if (!statusProcess) {
		printf("Erro na criacao do Terminal de Dados do Otimizacao! Codigo = %d\n", GetLastError());
	}
	else {
		printf("Processo de exibicao de Dados de Otimizacao e Terminal criados\n");
	}
	ReleaseMutex(hMutexConsole);

	/*Processo de exibicao de alarmes - Terminal Alarmes*/
	startup_info.lpTitle = L"TERMINAL C - Exibicao de Alarmes";
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

	WaitForSingleObject(hMutexConsole, INFINITE);
	if (!statusProcess) {
		printf("Erro na criacao do Terminal Alarmes! Codigo = %d\n", GetLastError());
	}
	else {
		printf("Processo de exibicao de Alarmes e Terminal criados\n");
	}
	ReleaseMutex(hMutexConsole);
}

void CapturarTeclado()
{
	/*Tratando inputs do teclado*/
	SetConsoleTextAttribute(hConsole, 15);
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
		}
	}
}

void CriarThreadsSecundarias()
{
	bool status = false; //success
	DWORD ret;

	/*Handles threads*/
	pthread_t hLeituraDadosOtimizacao, hLeituraDadosScada, hLeituraAlarmes,
		hRetiraDadosOtimizacao, hRetiraDadosScada, hRetiraAlarmes,
		hComunicacao;  /*Tarefas de Comunicacao de dados*/

/*Criando threads de Comunicacao de Dados*/

	WaitForSingleObject(hMutexConsole, INFINITE);
	int i = 1;
	status = pthread_create(&hComunicacao, NULL, GeraDados, (void*)i);
	if (!status) printf("Thread %d - Comunicacao de dados Alarmes - criada com Id= %0x \n", i, (int)&hComunicacao);
	else printf("Erro na criacao da thread %d! Codigo = %d\n", i, GetLastError());

	/*Criando threads de Retirada de Dados*/
	i = 2;
	status = pthread_create(&hRetiraDadosOtimizacao, NULL, RetiraDadosOtimizacao, (void*)i);
	if (!status) printf("Thread %d - Retirada de dados de Otimizacao - criada com Id= %0x \n", i, (int)&hRetiraDadosOtimizacao);
	else printf("Erro na criacao da thread %d! Codigo = %d\n", i, GetLastError());

	i = 3;
	status = pthread_create(&hRetiraDadosScada, NULL, RetiraDadosProcesso, (void*)i);
	if (!status) printf("Thread %d - Retirada de dados de Scada - criada com Id= %0x \n", i, (int)&hRetiraDadosScada);
	else printf("Erro na criacao da thread %d! Codigo = %d\n", i, GetLastError());

	i = 4;
	status = pthread_create(&hRetiraAlarmes, NULL, RetiraAlarmes, (void*)i);
	if (!status) printf("Thread %d - Retirada de alarmes - criada com Id= %0x \n", i, (int)&hRetiraAlarmes);
	else printf("Erro na criacao da thread %d! Codigo = %d\n", i, GetLastError());

	ReleaseMutex(hMutexConsole);
}

void FecharHandlers()
{
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
}

/* ======================================================================================================================== */
/*  3 THREADS SECUNDARIA DE ------COMUNICAÇÃO DE DADOS------ */
/*  GERACAO DAS MENSAGENS DO SISTEMA DE OTIMIZACAO E DO SISTEMA SCADA(PROCESSOS INDUSTRIAIS) */
/*  ADICIONA MENSAGENS NA LISTA CIRCULAR*/

void GeraDadosOtimizacao(int i) {
	int     status, nTipoEvento = 0, k = 0, l = 0;

	char    Otimizacao[38], Hora[3], Minuto[3], Segundo[3];

	DWORD   ret;
	BOOL    MemoriaCheia = false;

	/*Vetor com handles esperados */
	HANDLE SemLivre[2] = { hSemLivre, hEventKeyEsc },
		MutexBuffer[2] = { hMutexBuffer, hEventKeyEsc };

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

	/*Valores de SP_TEMP - Set point da temperatura do gás injetado (C)*/
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

	nTipoEvento = WaitForMultipleObjects(2, SemLivre, FALSE, INFINITE);     /*Esperando o espaço na memoria pra escrever*/

	if (nTipoEvento == 1) {                                                 /*Condição para termino do processo*/
		return;
	}
	else if (nTipoEvento == 0) {                                            /*Tem permissao pra escrever pois ninguem mais está escrevendo*/

		nTipoEvento = WaitForMultipleObjects(2, MutexBuffer, FALSE, INFINITE);      /*Conquistando o mutex da lista circular*/

		if (nTipoEvento == 0) {                                             /*Permissao pra iniciar a gravação na lista*/

			for (int j = 0; j < 38; j++) {
				CircularList[p_livre][j] = Otimizacao[j];
			}

			p_livre = (p_livre + 1) % RAM;                                  /*Movendo a posicao de livre para o proximo slot da memoria circular*/

			status = ReleaseMutex(hMutexBuffer);                             /*Liberando o mutex da secao critica*/

			MemoriaCheia = (p_livre == p_ocup);

			ReleaseSemaphore(hSemOcupado, 1, NULL);                         /*Liberando o semaforo de espacos ocupados*/

			if (MemoriaCheia) {
				WaitForSingleObject(hMutexConsole, INFINITE);
				SetConsoleTextAttribute(hConsole, 12);
				printf("MEMORIA CHEIA\n");
				SetConsoleTextAttribute(hConsole, 12);
				printf("BLOQUEADO");
				SetConsoleTextAttribute(hConsole, 15);
				printf(" - Thread Gera Dados\n");
				ReleaseMutex(hMutexConsole);

				nTipoEvento = WaitForMultipleObjects(2, SemLivre, FALSE, INFINITE);

				if (nTipoEvento == 0) {
					/*Liberando o semaforo de espacos livres*/
					ReleaseSemaphore(hSemLivre, 1, NULL);
					WaitForSingleObject(hMutexConsole, INFINITE);
					SetConsoleTextAttribute(hConsole, 10);
					printf("DESBLOQUEADO");
					SetConsoleTextAttribute(hConsole, 15);
					printf(" - Thread Gera Dados\n");
					ReleaseMutex(hMutexConsole);
				}
				else if (nTipoEvento == 1) {
					return;
				}
			}
			else {
				ReleaseSemaphore(hSemLivre, 1, NULL);
			}
		}
	}
	Sleep(1000);
}

void GeraDadosProcesso(int i) {
	int     status, nTipoEvento = 0, k = 0, l = 0;

	char    Processo[46], Hora[3], Minuto[3], Segundo[3];

	DWORD   ret;
	BOOL    MemoriaCheia = false;

	/*Vetor com handles esperados */
	HANDLE  SemLivre[2] = { hSemLivre, hEventKeyEsc },
		MutexBuffer[2] = { hMutexBuffer, hEventKeyEsc };

	/*Loop de execucao*/
		/*Valores de NSEQ - Numero sequencial de 1 ate 999999*/
	for (int j = 0; j < 6; j++) {
		k = i / pow(10, (5 - j));
		k = k % 10;
		Processo[j] = k + '0';
	}
	Processo[6] = '|';

	/*Valores de TIPO - Sempre 22*/
	Processo[7] = '2';
	Processo[8] = '2';

	Processo[9] = '|';

	/*Valores de PRESSAO_T - Pressão no tubo de extração (psi)*/
	for (int j = 10; j < 16; j++) {
		if (j == 13) {
			Processo[j] = '.';
		}
		else {
			Processo[j] = (rand() % 10) + '0';
		}
	}
	Processo[16] = '|';

	/*Valores de TEMP - Temperatura no tubo de extração (C)*/
	for (int j = 17; j < 23; j++) {
		if (j == 20) {
			Processo[j] = '.';
		}
		else {
			Processo[j] = (rand() % 10) + '0';
		}
	}
	Processo[23] = '|';

	/*Valores de PRESSAO_G - Pressão no reservatório de gás de injeção (psi)*/
	for (int j = 24; j < 30; j++) {
		if (j == 27) {
			Processo[j] = '.';
		}
		else {
			Processo[j] = (rand() % 10) + '0';
		}
	}
	Processo[30] = '|';

	/*Valores de NIVEL - Nível do reservatório de gás de injeção (cm)*/
	for (int j = 31; j < 37; j++) {
		if (j == 33) {
			Processo[j] = '.';
		}
		else {
			Processo[j] = (rand() % 10) + '0';
		}
	}
	Processo[37] = '|';

	/*Valores de TIMESTAMP*/
	SYSTEMTIME st;
	GetLocalTime(&st);

	/*Hora*/
	k = 0;
	l = 38;
	for (int j = 0; j < 2; j++) {
		k = st.wHour / pow(10, (1 - j));
		k = k % 10;
		Processo[l] = k + '0';
		l++;
	}
	Processo[40] = ':';

	/*Minuto*/
	k = 0;
	l = 41;
	for (int j = 0; j < 2; j++) {
		k = st.wMinute / pow(10, (1 - j));
		k = k % 10;
		Processo[l] = k + '0';
		l++;
	}
	Processo[43] = ':';

	/*Segundos*/
	k = 0;
	l = 44;
	for (int j = 0; j < 2; j++) {
		k = st.wSecond / pow(10, (1 - j));
		k = k % 10;
		Processo[l] = k + '0';
		l++;
	}

	/*Esperando o semaforo de espacos livres*/
	nTipoEvento = WaitForMultipleObjects(2, SemLivre, FALSE, INFINITE);


	/*Condição para termino do processo*/
	if (nTipoEvento == 1) {
		return;
	}
	else if (nTipoEvento == 0) {
		/*Conquistando o mutex da secao critica*/
		nTipoEvento = WaitForMultipleObjects(2, MutexBuffer, FALSE, INFINITE);

		if (nTipoEvento == 1) return;
		else if (nTipoEvento == 0) {
			/*Gravando dados em memoria RAM*/
			for (int j = 0; j < 46; j++) {
				CircularList[p_livre][j] = Processo[j];
			}
			status = ReleaseMutex(hMutexBuffer);                      /*Liberando o mutex da secao critica*/

			p_livre = (p_livre + 1) % RAM;
			MemoriaCheia = (p_livre == p_ocup);

			ReleaseSemaphore(hSemOcupado, 1, NULL);                    /*Liberando o semaforo de espacos ocupados*/


			if (MemoriaCheia) {
				WaitForSingleObject(hMutexConsole, INFINITE);
				SetConsoleTextAttribute(hConsole, 12);
				printf("MEMORIA CHEIA\n");
				SetConsoleTextAttribute(hConsole, 12);
				printf("BLOQUEADO");
				SetConsoleTextAttribute(hConsole, 15);
				printf(" - Thread Gera Dados\n");
				ReleaseMutex(hMutexConsole);

				nTipoEvento = WaitForMultipleObjects(2, SemLivre, FALSE, INFINITE);

				if (nTipoEvento == 0) {
					WaitForSingleObject(hMutexConsole, INFINITE);
					SetConsoleTextAttribute(hConsole, 10);
					printf("DESBLOQUEADO");
					SetConsoleTextAttribute(hConsole, 15);
					printf(" - Thread Gera Dados\n");
					ReleaseMutex(hMutexConsole);
					/*Liberando o semaforo de espacos livres*/
					ReleaseSemaphore(hSemLivre, 1, NULL);
				}
				else if (nTipoEvento == 1) {
					return;
				}
			}
			else {
				ReleaseSemaphore(hSemLivre, 1, NULL);
			}
		}
	}
	Sleep(1000);
}

void GeraAlarmes(int i) {
	/*Declarando variaveis locais da funcao LeituraSCADA()*/
	int     status, nTipoEvento = 0, k = 0, l = 0;

	char    Alarme[27], Hora[3], Minuto[3], Segundo[3];

	DWORD   ret;
	BOOL    MemoriaCheia;

	/*Vetor com handles da tarefa*/
	HANDLE  SemLivre[2] = { hSemLivre, hEventKeyEsc },
		MutexBuffer[2] = { hMutexBuffer, hEventKeyEsc };

	/*Loop de execucao*/
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

	/*Esperando o semaforo de espacos livres*/
	nTipoEvento = WaitForMultipleObjects(2, SemLivre, FALSE, INFINITE);

	/*Condição para termino do processo*/
	if (nTipoEvento == 1) return;
	else if (nTipoEvento == 0) {
		/*Conquistando o mutex da secao critica*/
		nTipoEvento = WaitForMultipleObjects(2, MutexBuffer, FALSE, INFINITE);

		if (nTipoEvento == 1) return;
		else if (nTipoEvento == 0) {
			/*Gravando dados em memoria RAM*/
			for (int j = 0; j < 27; j++) {
				CircularList[p_livre][j] = Alarme[j];
			}
			/*Liberando o mutex da secao critica*/
			ReleaseMutex(hMutexBuffer);

			/*Movendo a posicao de livre para o proximo slot da memoria circular*/
			p_livre = (p_livre + 1) % RAM;
			MemoriaCheia = (p_livre == p_ocup);
			/*Liberando o semaforo de espacos ocupados*/
			ReleaseSemaphore(hSemOcupado, 1, NULL);

			if (MemoriaCheia) {
				WaitForSingleObject(hMutexConsole, INFINITE);
				SetConsoleTextAttribute(hConsole, 12);
				printf("MEMORIA CHEIA\n");
				SetConsoleTextAttribute(hConsole, 12);
				printf("BLOQUEADO");
				SetConsoleTextAttribute(hConsole, 15);
				printf(" - Thread Gera Alarmes\n");
				ReleaseMutex(hMutexConsole);

				nTipoEvento = WaitForMultipleObjects(2, SemLivre, FALSE, INFINITE);

				if (nTipoEvento == 0) {
					/*Liberando o semaforo de espacos livres*/
					WaitForSingleObject(hMutexConsole, INFINITE);
					SetConsoleTextAttribute(hConsole, 10);
					printf("DESBLOQUEADO");
					SetConsoleTextAttribute(hConsole, 15);
					printf(" - Thread Gera Alarmes\n");
					ReleaseMutex(hMutexConsole);

					ReleaseSemaphore(hSemLivre, 1, NULL);
				}
				else if (nTipoEvento == 1) {
					return;
				}
			}
			else {
				ReleaseSemaphore(hSemLivre, 1, NULL);
			}
		}
	}

	Sleep(1000);
}

void* GeraDados(void* arg) {
	/*Declarando variaveis locais da funcao LeituraSCADA()*/
	int     index = (int)arg, status, i, nTipoEvento = 0;

	char    Hora[3], Minuto[3], Segundo[3];

	DWORD   ret;
	BOOL    MemoriaCheia;

	/*Vetor com handles da tarefa*/
	HANDLE  Events[2] = { hEventKeyC, hEventKeyEsc };

	/*Loop de execucao*/
	while (nTipoEvento != 1) {
		for (i = 1; i < 1000000; ++i) {
			GeraDadosOtimizacao(i);
			GeraDadosProcesso(i);
			GeraAlarmes(i);
			/*Condicao para termino da thread*/
			if (nTipoEvento == 1) break;

			/*Bloqueio e desbloqueio da thread GeraAlarmes*/
			ret = WaitForMultipleObjects(2, Events, FALSE, 1);
			nTipoEvento = ret - WAIT_OBJECT_0;

			if (nTipoEvento == 0) {
				WaitForSingleObject(hMutexConsole, INFINITE);
				SetConsoleTextAttribute(hConsole, 12);
				printf("BLOQUEADO");
				SetConsoleTextAttribute(hConsole, 15);
				printf(" - Thread Gera Dados\n");
				ReleaseMutex(hMutexConsole);

				nTipoEvento = WaitForMultipleObjects(2, Events, FALSE, INFINITE);

				if (nTipoEvento == 0) {
					WaitForSingleObject(hMutexConsole, INFINITE);
					SetConsoleTextAttribute(hConsole, 10);
					printf("DESBLOQUEADO");
					SetConsoleTextAttribute(hConsole, 15);
					printf(" - Thread Gera Dados\n");
					ReleaseMutex(hMutexConsole);
				}
				if (nTipoEvento == 1) {
					break;
				}
			}
			else if (nTipoEvento == 1) {
				break;
			}
		}

		/*Condição para termino do processo*/
		if (nTipoEvento == 1) break;
	}

	/*Fechando handles*/
	CloseHandle(Events);

	WaitForSingleObject(hMutexConsole, INFINITE);
	printf("Finalizando - Gera Dados\n");
	ReleaseMutex(hMutexConsole);
	pthread_exit((void*)index);

	return (void*)index;
}

/* ======================================================================================================================== */
/*  3 THREADS SECUNDARIA DE ------RETIRADA DE DADOS------ */
/*  RETIRADA DADOS DA LISTA CIRCULAR EM MEMORIA*/
/*  Dados de otimizacao sao escritos no arquivo circular*/
/*  Processos e Alarmes são enviados para as threads de Exibicao*/

void* RetiraDadosOtimizacao(void* arg) {
	int     index = (int)arg, status, i, nTipoEvento = 0;
	char    DadosOtimizacao[38];
	DWORD   ret;

	HANDLE  Events[2] = { hEventKeyO, hEventKeyEsc },
		SemOcupado[2] = { hSemOcupado, hEventKeyEsc },
		MutexBuffer[2] = { hMutexBuffer, hEventKeyEsc };

	while (nTipoEvento != 1) {
		nTipoEvento = WaitForMultipleObjects(2, Events, FALSE, 1);

		if (nTipoEvento == 0) {
			WaitForSingleObject(hMutexConsole, INFINITE);
			SetConsoleTextAttribute(hConsole, 12);
			printf("BLOQUEADO");
			SetConsoleTextAttribute(hConsole, 15);
			printf(" - Thread Retira Dados Otimizacao\n");
			ReleaseMutex(hMutexConsole);

			nTipoEvento = WaitForMultipleObjects(2, Events, FALSE, INFINITE);

			WaitForSingleObject(hMutexConsole, INFINITE);
			SetConsoleTextAttribute(hConsole, 10);
			printf("DESBLOQUEADO");
			SetConsoleTextAttribute(hConsole, 15);
			printf(" - Thread Retira Dados Otimizacao\n");
			ReleaseMutex(hMutexConsole);
		}

		if (nTipoEvento == 1) break;

		nTipoEvento = WaitForMultipleObjects(2, SemOcupado, FALSE, INFINITE);

		if (nTipoEvento == 1) break;

		else if (nTipoEvento == 0) {
			nTipoEvento = WaitForMultipleObjects(2, MutexBuffer, FALSE, INFINITE);                                /*Esperando o semaforo de espacos ocupados - permissao pra leitura*/

			if (nTipoEvento == 1) break;
			else if (nTipoEvento == 0) {                                                                 /*Semafoto de espacos ocupados Conquistado*/
				if (CircularList[p_ocup][7] == '1' && CircularList[p_ocup][8] == '1') {         /*Selecao dos dados apenas de tipo  55 = Alarme */
					for (int i = 0; i < 38; i++) {
						DadosOtimizacao[i] = CircularList[p_ocup][i];
					}

					WaitForSingleObject(hMutexConsole, INFINITE);
					printf("%.*s\n\n", 38, DadosOtimizacao);
					ReleaseMutex(hMutexConsole);

					p_ocup = (p_ocup + 1) % RAM;

					ReleaseSemaphore(hSemLivre, 1, NULL);
				}
				else {
					ReleaseSemaphore(hSemOcupado, 1, NULL);
				}

				ReleaseMutex(hMutexBuffer);                                                     /*Liberando o mutex da secao critica*/

			}
		}
	}

	CloseHandle(MutexBuffer);
	CloseHandle(SemOcupado);
	CloseHandle(Events);

	WaitForSingleObject(hMutexConsole, INFINITE);
	printf("Finalizando - Captura de Dados de Otimizacao\n");
	ReleaseMutex(hMutexConsole);
	pthread_exit((void*)index);
	return (void*)index;
}

void* RetiraDadosProcesso(void* arg) {
	int     index = (int)arg, status, i, nTipoEvento = 0;
	char    DadosProcesso[46];
	DWORD   ret;

	HANDLE  Events[2] = { hEventKeyP, hEventKeyEsc },
		SemOcupado[2] = { hSemOcupado, hEventKeyEsc },
		MutexBuffer[2] = { hMutexBuffer, hEventKeyEsc };


	while (nTipoEvento != 1) {
		nTipoEvento = WaitForMultipleObjects(2, Events, FALSE, 1);

		if (nTipoEvento == 0) {
			WaitForSingleObject(hMutexConsole, INFINITE);
			SetConsoleTextAttribute(hConsole, 12);
			printf("BLOQUEADO");
			SetConsoleTextAttribute(hConsole, 15);
			printf(" - Thread Retira Dados Processo\n");
			ReleaseMutex(hMutexConsole);

			nTipoEvento = WaitForMultipleObjects(2, Events, FALSE, INFINITE);

			WaitForSingleObject(hMutexConsole, INFINITE);
			SetConsoleTextAttribute(hConsole, 10);
			printf("DESBLOQUEADO");
			SetConsoleTextAttribute(hConsole, 15);
			printf(" - Thread Retira Dados Processo\n");
			ReleaseMutex(hMutexConsole);
		}

		if (nTipoEvento == 1) break;

		nTipoEvento = WaitForMultipleObjects(2, SemOcupado, FALSE, INFINITE);

		if (nTipoEvento == 1) break;

		else if (nTipoEvento == 0) {
			nTipoEvento = WaitForMultipleObjects(2, MutexBuffer, FALSE, INFINITE);                                /*Esperando o semaforo de espacos ocupados - permissao pra leitura*/

			if (nTipoEvento == 1) break;
			else if (nTipoEvento == 0) {                                                                 /*Semafoto de espacos ocupados Conquistado*/
				if (CircularList[p_ocup][7] == '2' && CircularList[p_ocup][8] == '2') {         /*Selecao dos dados apenas de tipo  55 = Alarme */
					for (int i = 0; i < 46; i++) {
						DadosProcesso[i] = CircularList[p_ocup][i];
					}

					WaitForSingleObject(hMutexConsole, INFINITE);
					printf("%.*s\n\n", 46, DadosProcesso);
					ReleaseMutex(hMutexConsole);

					p_ocup = (p_ocup + 1) % RAM;

					ReleaseSemaphore(hSemLivre, 1, NULL);
				}
				else {
					ReleaseSemaphore(hSemOcupado, 1, NULL);
				}

				ReleaseMutex(hMutexBuffer);                                                     /*Liberando o mutex da secao critica*/

			}
		}
	}


	CloseHandle(MutexBuffer);
	CloseHandle(SemOcupado);
	CloseHandle(Events);

	WaitForSingleObject(hMutexConsole, INFINITE);
	printf("Finalizando - Captura de dados de processo\n");
	ReleaseMutex(hMutexConsole);
	pthread_exit((void*)index);
	return (void*)index;
}

void* RetiraAlarmes(void* arg) {
	int     index = (int)arg, status, i, nTipoEvento = 0;
	char    Alarmes[27];
	DWORD   ret;

	HANDLE  Events[2] = { hEventKeyA, hEventKeyEsc },
		SemOcupado[2] = { hSemOcupado, hEventKeyEsc },
		MutexBuffer[2] = { hMutexBuffer, hEventKeyEsc };

	while (nTipoEvento != 1) {
		nTipoEvento = WaitForMultipleObjects(2, Events, FALSE, 1);

		if (nTipoEvento == 0) {
			WaitForSingleObject(hMutexConsole, INFINITE);
			SetConsoleTextAttribute(hConsole, 12);
			printf("BLOQUEADO");
			SetConsoleTextAttribute(hConsole, 15);
			printf(" - Thread Retira Alarmes\n");
			ReleaseMutex(hMutexConsole);

			nTipoEvento = WaitForMultipleObjects(2, Events, FALSE, INFINITE);

			WaitForSingleObject(hMutexConsole, INFINITE);
			SetConsoleTextAttribute(hConsole, 10);
			printf("DESBLOQUEADO");
			SetConsoleTextAttribute(hConsole, 15);
			printf(" - Thread Retira Alarmes\n");
			ReleaseMutex(hMutexConsole);
		}

		if (nTipoEvento == 1) break;

		nTipoEvento = WaitForMultipleObjects(2, SemOcupado, FALSE, INFINITE);

		if (nTipoEvento == 1) break;

		else if (nTipoEvento == 0) {
			nTipoEvento = WaitForMultipleObjects(2, MutexBuffer, FALSE, INFINITE);                                /*Esperando o semaforo de espacos ocupados - permissao pra leitura*/

			if (nTipoEvento == 1) break;
			else if (nTipoEvento == 0) {                                                                 /*Semafoto de espacos ocupados Conquistado*/
				if (CircularList[p_ocup][7] == '5' && CircularList[p_ocup][8] == '5') {         /*Selecao dos dados apenas de tipo  55 = Alarme */
					for (int i = 0; i < 27; i++) {
						Alarmes[i] = CircularList[p_ocup][i];
					}

					WaitForSingleObject(hMutexConsole, INFINITE);
					printf("%.*s\n\n", 27, Alarmes);
					ReleaseMutex(hMutexConsole);

					p_ocup = (p_ocup + 1) % RAM;

					ReleaseSemaphore(hSemLivre, 1, NULL);
				}
				else {
					ReleaseSemaphore(hSemOcupado, 1, NULL);
				}

				ReleaseMutex(hMutexBuffer);                                                     /*Liberando o mutex da secao critica*/

			}
		}
	}

	CloseHandle(MutexBuffer);
	CloseHandle(SemOcupado);
	CloseHandle(Events);

	WaitForSingleObject(hMutexConsole, INFINITE);
	printf("Finalizando - Captura de alarmes\n");
	ReleaseMutex(hMutexConsole);
	pthread_exit((void*)index);
	return (void*)index;
}
