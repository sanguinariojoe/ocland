/*
 *  This file is part of ocland, a free cloud OpenCL interface.
 *  Copyright (C) 2012  Jose Luis Cercos Pita <jl.cercos@upm.es>
 *
 *  ocland is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  ocland is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include <ocland/server/log.h>
#include <ocland/server/validator.h>
#include <ocland/server/dispatcher.h>

/** Maximum number of client connections
 * accepted by server. Variable must be
 * defined by autotools.
 */
#ifndef OCLAND_PORT
    #define OCLAND_PORT 51000u
#endif

/** Buffer size. Variable must be
 * defined by autotools.
 */
#ifndef BUFF_SIZE
    #define BUFF_SIZE 1025u
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
    printf("  -l, --log-file=LOG           Output log file. If unset /var/log/ocland.log\n");
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
    int index;
    int opt = getopt_long( argc, argv, opts, longOpts, &index );
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
        opt = getopt_long( argc, argv, opts, longOpts, &index );
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
    int serverfd = 0, *clientfd = NULL;
    validator *v = NULL;
    unsigned int n_clientfd = 0, i,j;
    struct sockaddr_in serv_addr;

    char buffer[BUFF_SIZE];

    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(buffer, '0', sizeof(buffer));

    serverfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if(serverfd < 0){
        printf("Socket can be registered!\n");
        return EXIT_FAILURE;
    }
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port        = htons(OCLAND_PORT);
    if(bind(serverfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))){
        printf("Can't bind on port %u!\n", OCLAND_PORT);
        return EXIT_FAILURE;
    }
    if(listen(serverfd, MAX_CLIENTS)){
        printf("Can't listen on port %u!\n", OCLAND_PORT);
        return EXIT_FAILURE;
    }
    printf("Server ready on port %u.\n", OCLAND_PORT);
    printf("%u connections will be accepted...\n", MAX_CLIENTS);
    fflush(stdout);
    // ------------------------------
    // Start serving
    // ------------------------------
    clientfd = (int*) malloc(MAX_CLIENTS*sizeof(int));
    v = (validator*) malloc(MAX_CLIENTS*sizeof(validator));
    for(i=0;i<MAX_CLIENTS;i++){
        clientfd[i] = -1;
    }
    while(1)
    {
        // Accepts new connection if possible
        int fd = accept(serverfd, (struct sockaddr*)NULL, NULL);
        if(fd >= 0){
            clientfd[n_clientfd] = fd;
            initValidator(&(v[n_clientfd]));
            struct sockaddr_in adr_inet;
            socklen_t len_inet;
            len_inet = sizeof(adr_inet);
            getsockname(fd, (struct sockaddr*)&adr_inet, &len_inet);
            n_clientfd++;
            printf("%s connected, hello!\n", inet_ntoa(adr_inet.sin_addr)); fflush(stdout);
            printf("%u connection slots free.\n", MAX_CLIENTS - n_clientfd); fflush(stdout);
        }
        // Count new number of clients (to manage lost ones)
        unsigned int n = n_clientfd;
        n_clientfd = 0;
        for(i=0;i<MAX_CLIENTS;i++){
            if(clientfd[i] >= 0){
                n_clientfd++;
            }
        }
        if(n != n_clientfd){
            printf("%u connection slots free.\n", MAX_CLIENTS - n_clientfd); fflush(stdout);
            if(!(MAX_CLIENTS - n_clientfd)){
                printf("NO MORE CLIENTS WILL BE ACCEPTED\n"); fflush(stdout);
            }
            // Sort the clients at the start of the array
            for(i=0;i<n_clientfd;i++){
                if(clientfd[i] < 0){
                    // Look for next non-negative
                    j = i+1;
                    while(clientfd[j] < 0)
                        j++;
                    // Swap them
                    clientfd[i] = clientfd[j];
                    clientfd[j] = -1;
                    v[i]        = v[j];
                }
            }
        }
        // Serve to the clients
        for(i=0;i<n_clientfd;i++){
            dispatch(&(clientfd[i]), buffer, v[i]);
            if(clientfd[i] < 0){
                // Client disconnected
                closeValidator(&(v[i]));
            }
        }
        if(!n_clientfd){
            // We can wait a little bit more if not any
            // client is connected.
            usleep(1000);
        }
    }
    free(clientfd); clientfd=0;
    free(v); v = NULL;
    return EXIT_SUCCESS;







    /*
    // ------------------------------
    // Initialize
    // ------------------------------
    parseOptions(argc, argv);
    validator v = NULL;
    initValidator(&v);

    // ------------------------------
    // Build server
    // ------------------------------
    int serverfd = 0, *clientfd = NULL;
    unsigned int n_clientfd = 0, i,j;
    struct sockaddr_in serv_addr;

    char buffer[BUFF_SIZE];

    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(buffer, '0', sizeof(buffer));

    serverfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if(serverfd < 0){
        printf("Socket can be registered!\n");
        return EXIT_FAILURE;
    }
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port        = htons(OCLAND_PORT);
    if(bind(serverfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))){
        printf("Can't bind on port %u!\n", OCLAND_PORT);
        return EXIT_FAILURE;
    }
    if(listen(serverfd, MAX_CLIENTS)){
        printf("Can't listen on port %u!\n", OCLAND_PORT);
        return EXIT_FAILURE;
    }
    printf("Server ready on port %u.\n", OCLAND_PORT);
    printf("%u connections will be accepted...\n", MAX_CLIENTS);
    fflush(stdout);
    // ------------------------------
    // Start serving
    // ------------------------------
    clientfd = (int*) malloc(MAX_CLIENTS*sizeof(int));
    for(i=0;i<MAX_CLIENTS;i++) clientfd[i] = -1;
    pthread_t threads[MAX_CLIENTS];
    while(1)
    {
        // Accepts new connection if possible
        int fd = accept(serverfd, (struct sockaddr*)NULL, NULL);
        if(fd >= 0){
            clientfd[n_clientfd] = fd;
            struct sockaddr_in adr_inet;
            socklen_t len_inet;
            len_inet = sizeof(adr_inet);
            getsockname(fd, (struct sockaddr*)&adr_inet, &len_inet);
            // Create the thread for the client
            int rc = pthread_create(&threads[n_clientfd], NULL, client_thread, (void *)(&clientfd[n_clientfd]));
            if(rc){
                printf("ERROR: Thread creation has failed with the return code %d\n", rc); fflush(stdout);
            }
            n_clientfd++;
            printf("%s connected, hello!\n", inet_ntoa(adr_inet.sin_addr)); fflush(stdout);
            printf("%u connection slots free.\n", MAX_CLIENTS - n_clientfd); fflush(stdout);
        }
        // Count new number of clients (to manage lost ones)
        unsigned int n = n_clientfd;
        n_clientfd = 0;
        for(i=0;i<MAX_CLIENTS;i++){
            if(clientfd[i] >= 0){
                if(pthread_kill(threads[i],0)){
                    // Client is not anymore valid, probably due to a segmentation fault
                    clientfd[i] = -1;
                    continue;
                }
                n_clientfd++;
            }
        }
        if(n != n_clientfd){
            printf("%u connection slots free.\n", MAX_CLIENTS - n_clientfd); fflush(stdout);
            if(!(MAX_CLIENTS - n_clientfd)){
                printf("NO MORE CLIENTS WILL BE ACCEPTED\n"); fflush(stdout);
            }
        }
        // Sort clients at the start of the array
        for(i=0;i<n_clientfd;i++){
            if(clientfd[i] < 0){
                // Look for next non-negative
                j = i+1;
                while(clientfd[j] < 0)
                    j++;
                // Swap them
                clientfd[i] = clientfd[j];
                threads[i]  = threads[j];
                clientfd[j] = -1;
            }
        }
        usleep(1000);
    }
    if(clientfd) free(clientfd); clientfd=0;
    closeValidator(&v);
    return EXIT_SUCCESS;
    */
}
