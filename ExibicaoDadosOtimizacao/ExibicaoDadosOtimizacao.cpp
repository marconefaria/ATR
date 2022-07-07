#define WIN32_LEAN_AND_MEAN
#define _CHECKERROR	    1
#define	ESC_KEY			27
#define	FILE_SIZE	    200

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "CheckForError.h"

HANDLE hEventKeyT, hEventKeyEsc;
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
HANDLE hFile, hArquivo, hArquivoCheio;

int main() {
    int     nTipoEvento = 2, key = 0, cont = 0;
    DWORD   ret, status, dwPos, dwBytesToWrite, dwBytesWritten;
    DWORD  dwBytesToRead, dwBytesRead;
    bool    desbloqueado = true;

    HANDLE hEventKeyT = OpenEvent(EVENT_ALL_ACCESS, TRUE, L"KeyT");
    CheckForError(hEventKeyT);
    HANDLE hEventKeyEsc = OpenEvent(EVENT_ALL_ACCESS, TRUE, L"KeyEsc");
    CheckForError(hEventKeyEsc);
    hArquivoCheio = OpenEvent(EVENT_ALL_ACCESS, TRUE, L"ArquivoCheio");
    CheckForError(hArquivoCheio);
    hArquivo = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, L"SemArquivo");
    CheckForError(hArquivo);

    HANDLE Events[2] = { hEventKeyT, hEventKeyEsc };

    while (nTipoEvento != 1) {

        nTipoEvento = WaitForMultipleObjects(2, Events, FALSE, 1);

        if (nTipoEvento == 1) break;

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
            strcpy(DadosOtimizacao, LerDadosArquivo(cont));
            cont++;
            ReleaseSemaphore(hArquivo, 1, NULL);
            SetEvent(hArquivoCheio);
        }
    }

    CloseHandle(Events);
    CloseHandle(hEventKeyEsc);
    CloseHandle(hEventKeyT);
    CloseHandle(hConsole);

    return EXIT_SUCCESS;
}

char*  LerDadosArquivo(int cont) {
    /*Abrindo arquivo*/
    hFile = CreateFile(
        L"..\\ArquivoCircular.txt",
        GENERIC_WRITE | GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        (LPSECURITY_ATTRIBUTES)NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        (HANDLE)NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Falha ao abrir o arquivo - Codigo %d. \n", GetLastError());
    }

    else {
        printf("Arquivo aberto com sucesso\n");

        Sleep(300);
        char DadosOtimizacao[38];

        cont = cont % FILE_SIZE;
        DWORD dwPos = cont * 38;

        /*Leitura de arquivo*/
        LockFile(hFile, 0, 38 * FILE_SIZE, 38 * FILE_SIZE, NULL);
        DWORD dwBytesRead = 0;
        dwPos = SetFilePointer(hFile, dwPos, NULL, FILE_BEGIN);
        int status = ReadFile(hFile, DadosOtimizacao, 38, &dwBytesRead, NULL);

        if (FALSE == status) {
            printf("Nao foi possivel habilitar o arquivo para leitura. Codigo %d\n", GetLastError());
        }

        UnlockFile(hFile, 0, 38 * FILE_SIZE, 38 * FILE_SIZE, NULL);

        return DadosOtimizacao;
    }
}