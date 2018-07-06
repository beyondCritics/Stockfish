#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char string[32767];
char fen[32767];
char epd[32767];
char bareepd[32767];
char topEpd[32767];
char apv[32767];

static const char * bestmove = "Best move: " ;// Be2
static const char * pondermove = "Ponder move: "; // Re8

char *getsafe(char *buffer, int count)
{
    char *result = buffer, *np;
    if ((buffer == NULL) || (count < 1))
        result = NULL;
    else if (count == 1)
        *result = '\0';
    else if ((result = fgets(buffer, count, stdin)) != NULL)
        if (np = strchr(buffer, '\n'))
            *np = '\0';
    return result;
}

static const char *searching = "Searching: ";

//32   -0.16   05:46   2897M  Nc3 Nc6 Bb5 Nh6 h5 cxd4 Qxd4 Nf5 Qc5 Bd7 Nf3 Nxe5 Nxe5 Bxe5 Qxd5 Bxc3+ bxc3 Qc7 Qxd7+ Qxd7 Bxd7+ Kxd7 Ba3 Kc6 O-O-O Rad8 Rxd8 Rxd8 hxg6 hxg6 Rh7 Rf8 Kb2 Kd5 Kb3 b6 Bb4 Ke6 g4 Nd6 Bxd6 Kxd6

int dmscore(int N)
{
    return (32767 - ((2 * N) - 1));
}

int dmnscore(int N)
{
    return (-32767 + (2 * N));
}

int main(int argc, char **argv)
{
    int depth=0;
    int ce;
    int mindepth = 25;
    bool bTopOnly = false;
    char finalBm[64] = { 0 };
    char finalResponse[64] = { 0 };
    if (argc > 1)
    {
        if (strcasecmp(argv[1], "top") == 0)
        {
            bTopOnly = 1;
            mindepth = 1;
        }
        else
        {
            mindepth = atoi(argv[1]);
            if (mindepth == 0) mindepth = 25;
        }
    }

    while (getsafe(string, sizeof string))
    {
        int oldDepth = 0;
        if (strncmp(string, bestmove, strlen(bestmove)) == 0)
        {
            strcpy(finalBm, string + strlen(bestmove));
        }

        if (strncmp(string, pondermove, strlen(pondermove)) == 0)
        {
            strcpy(finalResponse, string + strlen(pondermove));
        }

        if (strstr(string, searching))
        {
            static const char *tail = " 0 1";
            char *where = strstr(string, tail);
            if (where)
            {
                *where = 0;
            }
            strcpy(bareepd, string + strlen(searching));
            memset(finalResponse, 0, sizeof finalResponse);
            memset(finalBm, 0, sizeof finalBm);
        }
        oldDepth = depth;
        depth = atoi(string);
        if (depth > mindepth || strlen(finalResponse))
        {
            char *wheremat = 0;
            char *wherenmat = 0;
            if ((wheremat = strchr(string + 3, '#')) != 0)
            {
                if ((wherenmat = strstr(string + 3, "-#")) != 0)
                {
                    ce = dmnscore(atoi(wherenmat + 2));
                }
                else
                {
                    ce = dmscore(atoi(wheremat + 1));
                }
            }
            else
            {

                double x = atof(string + 3)*100.0;
                ce = (int)(x < 0 ? x - 0.5 : x + 0.5);
            }
            int hours = 0;
            int minutes = atoi(string+11);
            int seconds = 0;
            char *where = strchr(string+11, ':');
            if (where)
            {
                seconds = atoi(where+1);
                where = strchr(where+1, ':');

                if (where) // Then it wasn't mm:ss but rather hh:mm:ss
                {
                    hours = minutes;
                    minutes = seconds;
                    seconds = atoi(where);
                }
            }
            int acs = hours * 3600 + minutes * 60 + seconds;
            unsigned long long nodes = atoi(string+19);
            //nodes *= 1000;
            if (string[25] == 'M') nodes *= 1000;
            char *pv = string+28;
            strcpy(apv, pv);
            if (strchr(apv, ' ') == NULL) {
                strcat(apv, " ");
                strcat(apv, finalResponse);
            }
            sprintf(fen, "%s bm %s;", epd, pv);
            where = strchr(pv + 1, ' ');
            if (where) *where = 0;
            sprintf(epd, "%s acd %d; ce %d; acs %d; acn %llu; pv %s;", bareepd, depth, ce, acs, nodes, apv);
            if (bTopOnly)
            {
                if (strstr(topEpd, bareepd) == 0 && strlen(topEpd)) puts(topEpd);
                strcpy(topEpd, fen);
            }
            else
            {
                if (strncmp(fen, " bm ", 4) != 0  && depth > 0)
                    puts(fen);
            }

            if (pv && strlen(finalBm) > 0 && (strcmp(pv, finalBm) != 0) &&  strlen(finalResponse) > 0)
            {
                where = strstr(fen, "; pv ");
                if (where)
                {
                    where += strlen("; pv ");
                    sprintf(where, "%s %s", finalBm, finalResponse);
                }

                puts(fen);
                memset(finalResponse, 0, sizeof finalResponse);
            }
        }
    }
    return 0;
}
