#define WIN32_LEAN_AND_MEAN
#define _CHECKERROR	    1
#define	ESC_KEY			27

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "CheckForError.h"

HANDLE hEventKeyT, hEventKeyEsc;
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

int main() {
    int     nTipoEvento = 2, key = 0;
    DWORD   ret;

    HANDLE hEventKeyT = OpenEvent(EVENT_ALL_ACCESS, TRUE, L"KeyT");
    CheckForError(hEventKeyT);
    HANDLE hEventKeyEsc = OpenEvent(EVENT_ALL_ACCESS, TRUE, L"KeyEsc");
    CheckForError(hEventKeyEsc);

    HANDLE Events[2] = { hEventKeyT, hEventKeyEsc };

    while (nTipoEvento != 1) {

        ret = WaitForMultipleObjects(2, Events, FALSE, 1);

        if (ret != WAIT_TIMEOUT) {
            nTipoEvento = ret - WAIT_OBJECT_0;
        }

        if (nTipoEvento == 0) {
            SetConsoleTextAttribute(hConsole, 12);
            printf("BLOQUEADO");
            SetConsoleTextAttribute(hConsole, 15);
            printf(" - Processo de exibicao de dados de otimizacao\n");

            ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);

            nTipoEvento = ret - WAIT_OBJECT_0;

            if (nTipoEvento == 0) {

                SetConsoleTextAttribute(hConsole, 10);
                printf("DESBLOQUEADO");
                SetConsoleTextAttribute(hConsole, 15);
                printf(" - Processo de exibicao de dados de otimizacao\n");

                nTipoEvento = 2;
            }
        }
    }

    CloseHandle(Events);
    CloseHandle(hEventKeyEsc);
    CloseHandle(hEventKeyT);
    CloseHandle(hConsole);

    return EXIT_SUCCESS;
}