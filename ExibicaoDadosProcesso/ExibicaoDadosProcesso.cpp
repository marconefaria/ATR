#define WIN32_LEAN_AND_MEAN
#define _CHECKERROR	    1
#define	ESC_KEY			27

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "CheckForError.h"

HANDLE hEventKeyR, hEventKeyEsc, hEventMailslotProcesso, hMailslotServerProcesso;
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

int main() {
    int     nTipoEvento = 2, key = 0;
    bool status;

    char    buffer[46];

    DWORD   ret, dwBytesLidos, MessageCount;

    HANDLE hEventKeyR = OpenEvent(EVENT_ALL_ACCESS, TRUE, L"KeyR");
    CheckForError(hEventKeyR);

    HANDLE hEventKeyEsc = OpenEvent(EVENT_ALL_ACCESS, TRUE, L"KeyEsc");
    CheckForError(hEventKeyEsc);

    hEventMailslotProcesso = OpenEvent(EVENT_ALL_ACCESS, TRUE, L"MailslotProcesso");
    CheckForError(hEventMailslotProcesso);

    HANDLE Events[2] = { hEventKeyR, hEventKeyEsc };

    hMailslotServerProcesso = CreateMailslot(L"\\\\.\\mailslot\\MailslotProcesso", 0, MAILSLOT_WAIT_FOREVER, NULL);
    if (hMailslotServerProcesso == INVALID_HANDLE_VALUE)
    {
        printf("CreateMailslot failed: %d\n", GetLastError());
    }
    SetEvent(hEventMailslotProcesso);

    while (nTipoEvento != 1) {

        ret = WaitForMultipleObjects(2, Events, FALSE, 1);

        if (ret != WAIT_TIMEOUT) {
            nTipoEvento = ret - WAIT_OBJECT_0;
        }

        if (nTipoEvento == 0) {
            SetConsoleTextAttribute(hConsole, 12);
            printf("BLOQUEADO");
            SetConsoleTextAttribute(hConsole, 15);
            printf(" - Processo de exibicao de dados de processo\n");

            ret = WaitForMultipleObjects(2, Events, FALSE, INFINITE);
            nTipoEvento = ret - WAIT_OBJECT_0;

            if (nTipoEvento == 0) {

                SetConsoleTextAttribute(hConsole, 10);
                printf("DESBLOQUEADO");
                SetConsoleTextAttribute(hConsole, 15);
                printf(" - Processo de exibicao de dados de processo\n");

                nTipoEvento = 2;
            }
        }
        if (nTipoEvento == 2) {
            status = GetMailslotInfo(hMailslotServerProcesso, 0, &MessageCount, 0, 0);
            if (!status)
            {
                printf("GetMailslotInfo failed: %d\n", GetLastError());
            }

            DWORD    numRead;

            /* Read the record */
            status = ReadFile(hMailslotServerProcesso, &buffer, sizeof(buffer), &numRead, 0);

            /* See if an error */
            if (!status)
            {
                printf("ReadFile error: %d\n", GetLastError());
            }
            else
            {
                /*TIMESTAMP*/
                for (int j = 0; j < 8; j++) {
                    printf("%c", buffer[(j + 38)]);
                }

                printf(" NSEQ:");

                /*NSEQ*/
                for (int j = 8; j < 14; j++) {
                    printf("%c", buffer[(j - 8)]);
                }

                printf(" PR (T):");

                /*PRESSAO_T*/
                for (int j = 14; j < 20; j++) {
                    printf("%c", buffer[(j - 4)]);
                }

                printf("psi");
                printf(" TEMP:");
                
                /*TEMP*/
                for (int j = 20; j < 26; j++) {
                    printf("%c", buffer[(j - 3)]);
                }

                printf("C");
                printf(" PR (G):");

                /*PRESSAO_G*/
                for (int j = 26; j < 31; j++) {
                    printf("%c", buffer[(j - 2)]);
                }

                printf("psi");
                printf(" NIVEL:");

                /*NIVEL*/
                for (int j = 31; j < 37; j++) {
                    printf("%c", buffer[j]);
                }

                printf("cm");
            }

            printf("\n");

        }
    }

    CloseHandle(Events);
    CloseHandle(hMailslotServerProcesso);
    CloseHandle(hEventKeyEsc);
    CloseHandle(hEventKeyR);
    CloseHandle(hConsole);

    return EXIT_SUCCESS;
}