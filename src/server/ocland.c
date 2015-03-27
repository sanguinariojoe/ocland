/*
 *  This file is part of ocland, a free cloud OpenCL interface.
 *  Copyright (C) 2015  Jose Luis Cercos Pita <jl.cercos@upm.es>
 *
 *  ocland is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ocland is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser Public License
 *  along with ocland.  If not, see <http://www.gnu.org/licenses/>.
 */

/** \mainpage ocland (OpenCL land).
 * In Spanish ocland phonetics is similar to Oakland one,
 * an EEUU western city, Area code 510. \n
 * ocland is a free software to transparent remote devices
 * virtualization tool in order to use it with any OpenCL
 * based program. So ocland is the first OpenCL cloud
 * computing tookit. \n
 * ocland needs 510XX TCP ports open (Area code). ocland
 * server will bind to port 51000 listening for clients,
 * but since the clients can perform asynchronous memory
 * transfers, additional ports will be binded, so please
 * open TCP ports at least 2 times the maximum number of
 * clients allowed (to 32 default clients TCP Ports from
 * 51000 to 51064 must be open). \n
 * Please, read README file and user manual in order to
 * know how to use ocland. \n
 * Ocland have two main components, the server, that is an
 * executable running on the machines that host remote
 * desired devices, and the client, that is a library
 * installed as an OpenCL ICD (Installable Client Driver),
 * providing platforms on selected servers.
 * @remarks Ocland client requires an operative OpenCL-1.2
 * ICD loader.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <ocland/common/sockets.h>
#include <ocland/common/usleep.h>
#include <ocland/server/log.h>
#include <ocland/server/validator.h>
#include <ocland/server/dispatcher.h>

#ifdef _MSC_VER
    // own getopt implementation
    // only necessary functionality is implemented
    static int optind = 1;
    static char * optarg = NULL;
    struct option {
        const char *name;
        int has_arg;
        int *flag;
        int val;
    };
    enum {
        no_argument = 0,
        required_argument = 1,
        optional_argument = 2
    };
    /** Getopt interface implementation
     * @note not all getopt features are supported
       @note "--argument=value" syntax is not supported, only "--argument value"
     */
    int getopt_long(int argc, char *const argv[], const char *optString, const struct option *longOpts, int *longIndex)
    {
        if (optind >= argc)
        {
            return -1;
        }
        int ret = '?';
        if (argv[optind][0] == '-'){
            // this is an option
            struct option emptyOption = { 0 };
            int isLongOption = 0;
            if (argv[optind][1] == '-'){
                // this is a long option starting with "--"
                isLongOption = 1;
            }
            int i;
            for (i = 0; longOpts[i].val != NULL; i++){
                if ((isLongOption && !strcmp(argv[optind]+2, longOpts[i].name)) || ((!isLongOption) && argv[optind][1] == longOpts[i].val)){
                    // found an option in list
                    ret = longOpts[i].val;
                    if (longOpts[i].has_arg == required_argument){
                        if (optind + 1 < argc){
                            optind++;
                            optarg = argv[optind];
                        }
                        else{
                            // missed argument
                            ret = ':';
                        }
                    }
                    break; // option found
                }
            }
        }

        optind++;
        return ret;
    }
#else
    #include <getopt.h>
#endif
/** Maximum number of client connections
 * accepted by server. Variable must be
 * defined by autotools.
 */
#ifndef OCLAND_PORT
    #define OCLAND_PORT 51000u
#endif

/** Maximum number of client connections
 * accepted by server. Variable must be
 * defined by autotools.
 */
#ifndef MAX_CLIENTS
    #define MAX_CLIENTS 32u
#endif

/** ocland name and version. Variable must be
 * defined by autotools.
 */
#ifndef PACKAGE_STRING
    #define PACKAGE_STRING "ocland 0.0.00"
#endif

/// Valid command line sort options.
static const char *opts = "l:vh?";
/// Valid command line long options.
static const struct option longOpts[] = {
    { "log-file", required_argument, NULL, 'l' },
    { "version", no_argument, NULL, 'v' },
    { "help", no_argument, NULL, 'h' },
    { NULL, no_argument, NULL, 0 }
};
/// Option argument
extern char *optarg;

/** Show usage/help page and stops ocland server execution.
 */
void displayUsage()
{
    printf("Usage:\tocland [Option]...\n");
    printf("Launch ocland server.\n");
    printf("\n");
    printf("Required arguments for long options are also required for the short ones.\n");
#ifndef _MSC_VER
    printf("  -l, --log-file=LOG           Output log file. Command line output is used if unset.\n");
#else
    printf("  -l, --log-file LOG           Output log file. Command line output is used if unset.\n");
#endif
    printf("                                 will used\n");
    printf("  -v, --version                Show ocland name and version\n");
    printf("  -h, --help                   Show this help page\n");
}

/** Parse command line options. Execute ocland --help to see
 * valid command line options.
 * @param argc Number of command line arguments.
 * @param argv List of command line arguments.
 */
void parseOptions(int argc, char *argv[])
{
    int opt = getopt_long( argc, argv, opts, longOpts, NULL );
    while( opt != -1 ) {
        switch( opt ) {
            case 'l':
                if(!setLogFile(optarg)){
                    printf("File \"%s\" could not be opened!\n", optarg);
                    exit(EXIT_FAILURE);
                }
                break;

            case 'v':
                printf(PACKAGE_STRING);
                printf("\n");
                exit(EXIT_SUCCESS);

            case ':':
            case '?':
                printf("\n");
                displayUsage();
                exit(EXIT_FAILURE);
            case 'h':
                displayUsage();
                exit(EXIT_SUCCESS);

            default:
                displayUsage();
                exit(EXIT_FAILURE);
        }
        opt = getopt_long( argc, argv, opts, longOpts, NULL );
    }
}

/** Server entry point. ocland-server is an executable called ocland
 * that runs on machines that must serve computing devices remotely.
 * @param argc Number of command line arguments.
 * @param argv List of command line arguments.
 * @return EXIT_SUCCESS if server ran right, EXIT_FAILURE otherwise.
 */
int main(int argc, char *argv[])
{
    // ------------------------------
    // Initialize
    // ------------------------------
    parseOptions(argc, argv);

    // ------------------------------
    // Build server
    // ------------------------------
    int flag;
    int switch_on=1;
    // int switch_off = 0;
    int server[2], *clientfd=NULL, *clientcb=NULL;
    validator *v=NULL;
    unsigned int n_clients=0, i, j;
    struct sockaddr_in serv_addr;
#ifdef WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        printf("Winsock DLL initialization failed with error %d\n", err);
        return EXIT_SUCCESS;
    }
#endif

    memset(&serv_addr, '0', sizeof(serv_addr));
    for(i = 0; i < 2; i++){
#ifndef WIN32
        if(i == 0)
            server[i] = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        else
            server[i] = socket(AF_INET, SOCK_STREAM, 0);
        if(server[i] < 0){
            printf("Socket cannot be registered!\n");
            return EXIT_FAILURE;
        }
        flag = setsockopt(server[i],
                          IPPROTO_TCP,
                          TCP_QUICKACK,
                          (void *) &switch_on,
                          sizeof(int));
        if(flag){
            printf("Failure enabling TCP_QUICKACK: %s\n", strerror(errno));
            printf("\tThe socket is still considered valid\n");
        }
#else
        server[i] = socket(AF_INET, SOCK_STREAM, 0);
        if(server[i] < 0){
            printf("Socket cannot be registered! Error %d.\n", WSAGetLastError());
            return EXIT_FAILURE;
        }
        if(i == 0){
            // make it non-blocking
            // for non-blocking accept()
            u_long arg = 1;
            ioctlsocket(server[i], FIONBIO, &arg);
        }
#endif

        flag = setsockopt(server[i],
                          IPPROTO_TCP,
                          TCP_NODELAY,
                          (void *) &switch_on,
                          sizeof(int));
        if(flag){
            printf("Failure enabling TCP_NODELAY: %s\n", strerror(errno));
            printf("\tThe socket is still considered valid\n");
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.sin_port = htons(OCLAND_PORT + i);
        if(bind(server[i], (struct sockaddr*)&serv_addr, sizeof(serv_addr))){
            printf("Can't bind on port %u!\n", OCLAND_PORT);
            return EXIT_FAILURE;
        }
        if(listen(server[i], MAX_CLIENTS)){
            printf("Can't listen on port %u!\n", OCLAND_PORT);
            return EXIT_FAILURE;
        }
    }

    printf("Server ready on port %u.\n", OCLAND_PORT);
    printf("%u connections will be accepted...\n", MAX_CLIENTS);
    fflush(stdout);
    // ------------------------------
    // Start serving
    // ------------------------------
    clientfd = (int *)malloc(MAX_CLIENTS * sizeof(int));
    clientcb = (int *)malloc(MAX_CLIENTS * sizeof(int));
    v = (validator *)malloc(MAX_CLIENTS * sizeof(validator));
    if((!clientfd) || (!clientcb) || (!v)){
        printf("Failure allocating memory!\n");
        free(clientfd); clientfd=NULL;
        free(clientcb); clientcb=NULL;
        free(v); v=NULL;
    }
    for(i = 0; i < MAX_CLIENTS; i++){
        clientfd[i] = -1;
        clientcb[i] = -1;
        v[i] = (validator)malloc(sizeof(struct validator_st));
        if(!v[i]){
            printf("Failure allocating memory!\n");
            for(j = 0; j < i; j++){
                free(v[j]); v[j] = NULL;
            }
            free(clientfd); clientfd=NULL;
            free(clientcb); clientcb=NULL;
            free(v); v=NULL;
            return EXIT_FAILURE;
        }
    }
    while(1)
    {
        // Accepts new connection if possible
        int fd = accept(server[0], (struct sockaddr*)NULL, NULL);
        if(fd >= 0){
            clientfd[n_clients] = fd;
#ifdef WIN32
            BOOL bool_on = TRUE;
            setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&bool_on, sizeof(BOOL));
            // make it blocking because MSG_WAITALL recv flag is not supported for non-blocking socket on Windows
            u_long arg = 0;
            ioctlsocket(fd, FIONBIO, &arg);
#else
            setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&switch_on, sizeof(int));
            setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, (char *)&switch_on, sizeof(int));
#endif
            struct sockaddr_in adr_inet;
            socklen_t len_inet;
            len_inet = sizeof(adr_inet);
            getsockname(fd, (struct sockaddr*)&adr_inet, &len_inet);
            // Connect the additional data streams
            int *clients[2] = {clientfd, clientcb};
            for(j = 1; j < 2; j++){
                fd = accept(server[1], (struct sockaddr*)NULL, NULL);
                if(fd < 0){
                    printf("%s connected to port %u, but not to %u\n",
                           inet_ntoa(adr_inet.sin_addr),
                           OCLAND_PORT,
                           OCLAND_PORT + 1);
                    fflush(stdout);
                    clientfd[n_clients] = -1;
                    break;
                }
                clients[j][n_clients] = fd;
            }
            if(clientfd[n_clients] < 0){
                // Forgive this connection try and restart the loop
                for(j = 0; j < 2; j++){
                    shutdown(clients[j][n_clients], 2);
                    clients[j][n_clients] = -1;
                }
                continue;
            }

            initValidator(v[n_clients]);
            v[n_clients]->socket = &(clientfd[n_clients]);
            v[n_clients]->callbacks_socket = &(clientcb[n_clients]);
            n_clients++;
            printf("%s connected, hello!\n", inet_ntoa(adr_inet.sin_addr)); fflush(stdout);
            printf("%u connection slots free.\n", MAX_CLIENTS - n_clients); fflush(stdout);
        }
        // Count new number of clients (to manage lost ones)
        unsigned int n = n_clients;
        n_clients = 0;
        for(i = 0; i < MAX_CLIENTS; i++){
            if(clientfd[i] >= 0){
                n_clients++;
            }
        }
        if(n != n_clients){
            printf("%u connection slots free.\n", MAX_CLIENTS - n_clients); fflush(stdout);
            if(!(MAX_CLIENTS - n_clients)){
                printf("NO MORE CLIENTS WILL BE ACCEPTED\n"); fflush(stdout);
            }
            // Sort the active clients at the start of the array
            for(i = 0; i < n_clients; i++){
                if(clientfd[i] < 0){
                    // Look for next non-negative
                    j = i+1;
                    while(clientfd[j] < 0)
                        j++;
                    // Swap them
                    clientfd[i] = clientfd[j];
                    clientfd[j] = -1;
                    clientcb[i] = clientcb[j];
                    clientcb[j] = -1;
                    v[i] = v[j];
                }
            }
        }
        // Serve to the clients
        for(i = 0; i < n_clients; i++){
            dispatch(&(clientfd[i]), v[i]);
            if((clientfd[i] < 0) || (clientcb[i] < 0)){
                // Client disconnected
                if (clientfd[i] != -1) {
                    shutdown(clientfd[i], 2);
                    clientfd[i] = -1;
                }
                if (clientcb[i] != -1) {
                    shutdown(clientcb[i], 2);
                    clientcb[i] = -1;
                }
                closeValidator(v[i]);
            }
        }
        if(!n_clients){
            // We can wait a little bit more if not any client is connected.
            usleep(1000);
        }
    }
    free(clientfd); clientfd = NULL;
    free(clientcb); clientcb = NULL;
    for(i = 0; i < MAX_CLIENTS; i++){
        free(v[i]); v[i] = NULL;
    }
    free(v); v = NULL;
#ifdef WIN32
    WSACleanup();
#endif
    return EXIT_SUCCESS;
}
