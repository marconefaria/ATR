#define WIN32_LEAN_AND_MEAN
#define _CHECKERROR	    1
#define	ESC_KEY			27

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <locale.h>
#include "CheckForError.h"

HANDLE hEventKeyL, hEventKeyEsc, hEventKeyZ, hEventMailslotAlarme, hMailslotServerAlarme;
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

char descricaoAlarme[10][30] =
{
	"10|Pressão baixa gás injeção ",
	"01|Derramamento de óleo PF-1 ",
	"02|Parada total de produção  ",
	"03|Nível crítico reservatório",
	"04|Parada produção por 60 min",
	"05|Danos leves a equipamentos",
	"06|Temperatura alta extração ",
	"07|Danos críticos equipamento",
	"08|Temperatura baixa extração",
	"09|Pressão baixa gás extração"
};

int main() {
	setlocale(LC_ALL, "Portuguese");
	int     nTipoEvento = 3, key = 0, index = 0;

	bool status;

	char    buffer[27];

	DWORD   ret, dwBytesLidos, MessageCount;

	HANDLE hEventKeyL = OpenEvent(EVENT_ALL_ACCESS, TRUE, L"KeyL");
	CheckForError(hEventKeyL);

	HANDLE hEventKeyEsc = OpenEvent(EVENT_ALL_ACCESS, TRUE, L"KeyEsc");
	CheckForError(hEventKeyEsc);

	HANDLE hEventKeyZ = OpenEvent(EVENT_ALL_ACCESS, TRUE, L"KeyZ");
	CheckForError(hEventKeyZ);

	hEventMailslotAlarme = OpenEvent(EVENT_ALL_ACCESS, TRUE, L"MailslotAlarme");
	CheckForError(hEventMailslotAlarme);

	HANDLE Events[3] = { hEventKeyL, hEventKeyEsc, hEventKeyZ };

	hMailslotServerAlarme = CreateMailslot(L"\\\\.\\mailslot\\MailslotAlarme", 0, MAILSLOT_WAIT_FOREVER, NULL);
	if (hMailslotServerAlarme == INVALID_HANDLE_VALUE)
	{
		printf("CreateMailslot failed: %d\n", GetLastError());
	}
	SetEvent(hEventMailslotAlarme);

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

		if (nTipoEvento == 3) {
			status = GetMailslotInfo(hMailslotServerAlarme, 0, &MessageCount, 0, 0);
			if (!status)
			{
				printf("GetMailslotInfo failed: %d\n", GetLastError());
			}

			DWORD    numRead;

			/* Read the record */
			status = ReadFile(hMailslotServerAlarme, &buffer, sizeof(buffer), &numRead, 0);

			/* See if an error */
			if (!status)
			{
				printf("ReadFile error: %d\n", GetLastError());
			}
			else
			{
				/*TIMESTAMP*/
				for (int j = 0; j < 8; j++) {
					printf("%c", buffer[(j + 19)]);
				}

				printf(" NSEQ: ");

				/*NSEQ*/
				for (int j = 8; j < 14; j++) {
					printf("%c", buffer[(j - 8)]);
				}

				index = (int)buffer[13] - 48;
				printf(" %.*s PRI: ", 30, descricaoAlarme[index]);

				/*PRIORIDADE*/
				for (int j = 18; j < 21; j++) {
					printf("%c", buffer[(j - 3)]);
				}
			}

			printf("\n");

		}
	}

	CloseHandle(Events);
	CloseHandle(hMailslotServerAlarme);
	CloseHandle(hEventMailslotAlarme);
	CloseHandle(hEventKeyEsc);
	CloseHandle(hEventKeyL);
	CloseHandle(hConsole);

	return EXIT_SUCCESS;
}