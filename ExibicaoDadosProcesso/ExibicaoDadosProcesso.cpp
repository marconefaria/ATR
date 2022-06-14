#define WIN32_LEAN_AND_MEAN
#define _CHECKERROR	    1
#define	ESC_KEY			27

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "CheckForError.h"

HANDLE hEventKeyL, hEventKeyEsc;
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

int main() {
    SetConsoleTitle(L"TERMINAL C - Exibicao de dados SCADA");

    int     nTipoEvento = 2, key = 0;

    bool    status = false;

    char    PIMS[27] = { 'N', 'S', 'E', 'Q',
                         'T', 'I', 'P', 'O',
                         'I', 'D',
                         'P', 'R', 'I', 'O', 'R', 'I', 'D', 'A', 'D', 'E',
                         'H', 'H', ':', 'M', 'M', ':', 'S',
    },
        MsgBuffer[38];

    DWORD   ret, dwBytesLidos, MenssageCount{};

    hEventKeyL = OpenEvent(EVENT_ALL_ACCESS, TRUE, L"KeyL");
    CheckForError(hEventKeyL);

    hEventKeyEsc = OpenEvent(EVENT_ALL_ACCESS, TRUE, L"KeyEsc");
    CheckForError(hEventKeyEsc);

    HANDLE Events[2] = { hEventKeyL, hEventKeyEsc };

    while (key != ESC_KEY) {
        ret = WaitForMultipleObjects(2, Events, FALSE, 1);
        GetLastError();

        if (ret != WAIT_TIMEOUT) {
            nTipoEvento = ret - WAIT_OBJECT_0;
        }

        if (nTipoEvento == 0) {
            SetConsoleTextAttribute(hConsole, 12);
            printf("BLOQUEADO");
            SetConsoleTextAttribute(hConsole, 15);
            printf(" - Processo de exibicao de dados\n");

            ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);

            nTipoEvento = ret - WAIT_OBJECT_0;

            if (nTipoEvento == 0) {
                nTipoEvento = 2;
            }

            SetConsoleTextAttribute(hConsole, 10);
            printf("DESBLOQUEADO");
            SetConsoleTextAttribute(hConsole, 15);
            printf(" - Processo de exibicao de dados\n");
        }

        /*Condicao para termino do processo*/
        if (nTipoEvento == 1) {
            key = ESC_KEY;
        }

        /*Caso nTipoEvento nao tenha sido alterado -> leitura do mailslot*/
        if (nTipoEvento == 2 && (int)MenssageCount > 0) {
            CheckForError(status);

            /*TIMESTAMP*/
            for (int j = 0; j < 8; j++) {
                PIMS[j] = MsgBuffer[(j + 23)];
            }

            /*NSEQ*/
            for (int j = 14; j < 20; j++) {
                PIMS[j] = MsgBuffer[(j - 14)];
            }

            /*ID ALARME*/
            for (int j = 31; j < 35; j++) {
                PIMS[j] = MsgBuffer[(j - 22)];
            }

            /*GRAU*/
            for (int j = 41; j < 43; j++) {
                PIMS[j] = MsgBuffer[(j - 27)];
            }

            /*PREV*/
            for (int j = 49; j < 54; j++) {
                PIMS[j] = MsgBuffer[(j - 32)];
            }

            if (MsgBuffer[7] == '9') {
                /*Exibe alarmes criticos em vermelho*/
                SetConsoleTextAttribute(hConsole, 12);
                for (int j = 0; j < 54; j++) {
                    printf("%c", PIMS[j]);
                }
                SetConsoleTextAttribute(hConsole, 15);
                printf("\n");
            }
            else if (MsgBuffer[7] == '2') {
                /*Exibe alarmes nao criticos*/
                for (int j = 0; j < 54; j++) {
                    printf("%c", PIMS[j]);
                }
                printf("\n");
            }
        }
    } /*fim do while*/

    /*------------------------------------------------------------------------------*/
    /*Fechando handles*/
    CloseHandle(Events);
    CloseHandle(hEventKeyEsc);
    CloseHandle(hEventKeyL);
    CloseHandle(hConsole);

    /*------------------------------------------------------------------------------*/
    /*Finalizando o processo de exibicao de alarmes*/
    return EXIT_SUCCESS;
}