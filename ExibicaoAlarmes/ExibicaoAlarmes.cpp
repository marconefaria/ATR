#define WIN32_LEAN_AND_MEAN
#define _CHECKERROR	    1
#define	ESC_KEY			27

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "CheckForError.h"

HANDLE hEventKeyL, hEventKeyEsc, hEventKeyZ;
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

int main() {
    int     nTipoEvento = 3, key = 0;
    DWORD   ret;

    HANDLE hEventKeyL = OpenEvent(EVENT_ALL_ACCESS, TRUE, L"KeyL");
    CheckForError(hEventKeyL);
    HANDLE hEventKeyEsc = OpenEvent(EVENT_ALL_ACCESS, TRUE, L"KeyEsc");
    CheckForError(hEventKeyEsc);
    HANDLE hEventKeyZ = OpenEvent(EVENT_ALL_ACCESS, TRUE, L"KeyZ");
    CheckForError(hEventKeyZ);

    HANDLE Events[3] = { hEventKeyL, hEventKeyEsc, hEventKeyZ};

    while (nTipoEvento != 1) {

        ret = WaitForMultipleObjects(3, Events, FALSE, 1);

        if (ret != WAIT_TIMEOUT) {
            nTipoEvento = ret - WAIT_OBJECT_0;
        }

        if (nTipoEvento == 0) {
            SetConsoleTextAttribute(hConsole, 12);
            printf("BLOQUEADO");
            SetConsoleTextAttribute(hConsole, 15);
            printf(" - Processo de exibicao de alarmes\n");

            ret = WaitForMultipleObjects(3, Events, FALSE, INFINITE);
            nTipoEvento = ret - WAIT_OBJECT_0;

            if (nTipoEvento == 0) {

                SetConsoleTextAttribute(hConsole, 10);
                printf("DESBLOQUEADO");
                SetConsoleTextAttribute(hConsole, 15);
                printf(" - Processo de exibicao de alarmes\n");

                nTipoEvento = 3;
            }
        }
        else if (nTipoEvento == 2) {
             system("cls");
        }
    } 

    CloseHandle(Events);
    CloseHandle(hEventKeyEsc);
    CloseHandle(hEventKeyL);
    CloseHandle(hConsole);

    return EXIT_SUCCESS;
}