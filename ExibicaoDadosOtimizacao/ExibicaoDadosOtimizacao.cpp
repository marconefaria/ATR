#define WIN32_LEAN_AND_MEAN
#define	FILE_SIZE	    200

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "CheckForError.h"

HANDLE hEventKeyT;
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
HANDLE hFile, hArquivo, hArquivoCheio;
void LerDadosArquivo();
int cont = 0;

int main() {
    int     nTipoEvento = 2;
    bool    desbloqueado = true;

    hEventKeyT = OpenEvent(EVENT_ALL_ACCESS, TRUE, L"KeyT");
    CheckForError(hEventKeyT);
    hArquivoCheio = OpenEvent(EVENT_ALL_ACCESS, TRUE, L"ArquivoCheio");
    CheckForError(hArquivoCheio);
    hArquivo = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, L"SemArquivo");
    CheckForError(hArquivo);

    /*Abrindo arquivo*/
    hFile = CreateFile(
        L"..\\..\\ArquivoCircular.txt",
        GENERIC_WRITE | GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        (LPSECURITY_ATTRIBUTES)NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        (HANDLE)NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Falha ao abrir o arquivo - Codigo %d. \n", GetLastError());
    }

    while (true) {

        nTipoEvento = WaitForSingleObject(hEventKeyT, 1);

        if (nTipoEvento == 0 && desbloqueado) {
            desbloqueado = false;
            SetConsoleTextAttribute(hConsole, 12);
            printf("BLOQUEADO");
            SetConsoleTextAttribute(hConsole, 15);
            printf(" - Processo de exibicao de dados de otimizacao\n");
        }
        else if (nTipoEvento == 0 && !desbloqueado)
        {
            desbloqueado = true;
            SetConsoleTextAttribute(hConsole, 10);
            printf("DESBLOQUEADO");
            SetConsoleTextAttribute(hConsole, 15);
            printf(" - Processo de exibicao de dados de otimizacao\n");
        }

        if (desbloqueado)
        {
            char DadosOtimizacao[38];
            
            LerDadosArquivo();
            
            ReleaseSemaphore(hArquivo, 1, NULL);
            SetEvent(hArquivoCheio);
        }
        Sleep(1000);
    }

    return EXIT_SUCCESS;
}

void  LerDadosArquivo() {

    char DadosOtimizacao[38];
    cont = cont % FILE_SIZE;
    DWORD dwPos = cont * 39;
    DWORD dwBytesRead = 0;

    int status = LockFile(hFile, 0, NULL, 39 * FILE_SIZE, NULL);
    if (status != 0)
    {
        dwPos = SetFilePointer(hFile, dwPos, NULL, FILE_BEGIN);
        status = ReadFile(hFile, DadosOtimizacao, 38, &dwBytesRead, NULL);

        if (FALSE == status) {
            printf("ReadFile error: %d\n", GetLastError());
        }
        else if (dwBytesRead == 38) {
            /*NSEQ*/
            printf("NSEQ: ");

            for (int j = 0; j < 6; j++) {
                printf("%c", DadosOtimizacao[j]);
            }

            printf(" SP (TEMP) :");

            /*SP_TEMP*/
            for (int j = 17; j < 23; j++) {
                printf("%c", DadosOtimizacao[j]);
            }

            printf("C SP (PRE) :");

            /*SP_PRESS*/
            for (int j = 10; j < 16; j++) {
                printf("%c", DadosOtimizacao[j]);
            }

            printf("psi VOL:");

            /*VOL*/
            for (int j = 24; j < 29; j++) {
                printf("%c", DadosOtimizacao[j]);
            }
            printf("m3 PROD:");

            /*TIPO*/
            for (int j = 7; j < 9; j++) {
                printf("%c", DadosOtimizacao[j]);
            }
            printf(" ");

            /*TIMESTAMP*/
            for (int j = 30; j < 38; j++) {
                printf("%c", DadosOtimizacao[j]);
            }
            printf("\n");

            cont++;
        }
        status = UnlockFile(hFile, 0, NULL, 39 * FILE_SIZE, NULL);
    }
}