/*
 * Copyright (c) 2013, Majenko Technologies
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <CLI.h>

CLIServer CLI;

CLIServer::CLIServer() {
    clients = NULL;
    commands = NULL;
}

void CLIServer::addClient(Stream *dev) {
    CLIClient *scan;
    CLIClient *newClient;

    newClient = (CLIClient *)malloc(sizeof(CLIClient));
    newClient->dev = dev;
    newClient->pos = 0;
    bzero(newClient->input, CLI_BUFFER);
    newClient->next = NULL;
    if (clients == NULL) {
        clients = newClient;
        return;
    }
    for (scan = clients; scan->next; scan = scan->next);
    scan->next = newClient;
}

void CLIServer::addCommand(const char *command, int (*function)(Stream *, int, char **)) {
    CLICommand *scan;
    CLICommand *newCommand;

    newCommand = (CLICommand *)malloc(sizeof(CLICommand));
    newCommand->command = strdup(command);
    newCommand->function = function;
    newCommand->next = NULL;

    if (commands == NULL) {
        commands = newCommand;
        return;
    }
    for (scan = commands; scan->next; scan = scan->next);
    scan->next = newCommand;
}

char *CLIServer::getWord(char *buf) {
    static char *ptr = NULL;
    char *start, *scan;
    char term = ' ';

    if (buf != NULL) {
        ptr = buf;
    }

    while (*ptr == ' ' || *ptr == '\t' && *ptr != '\0') {
        ptr++;
    }
    if (*ptr == '\0') {
        return NULL;
    }

    if (*ptr == '"' || *ptr == '\'') {
        term = *ptr;
        ptr++;
    }
    start = ptr;

    while (*ptr != '\0') {
        if (*ptr == '\\') {
            for (scan = ptr; *scan != '\0'; scan++) {
                *scan = *(scan+1);
            }
            ptr++;
            continue;
        }
        if (*ptr == term || (term == ' ' && *ptr == '\t')) {
            *ptr = '\0';
            ptr++;
            return start;
        }
        ptr++;
    }
    if (ptr == start) {
        return NULL;
    }
    return start;
}

int CLIServer::readline(CLIClient *client) {
	int rpos;

	char readch = client->dev->read();
	
	if (readch > 0) {
		switch (readch) {
			case '\n': // Ignore new-lines
				break;
			case '\r': // Return on CR
				rpos = client->pos;
				client->pos = 0;  // Reset position index ready for next time
				client->dev->println();
				return rpos;
			case 8:
			case 127:
				if (client->pos > 0) {
					client->pos--;
					client->input[client->pos] = 0;
					client->dev->print("\b \b");
				}
				break;
			default:
				if (client->pos < CLI_BUFFER-1) {
					client->dev->print(readch);	
					client->input[client->pos++] = readch;
					client->input[client->pos] = 0;
				}
		}
	}
	// No end of line has been found, so return -1.
	return -1;
}

int CLIServer::parseCommand(CLIClient *client) {
    CLICommand *scan;
	char *argv[20];
	int argc;
	char *w;

	argc = 0;
	w = getWord(client->input);
	while ((argc < 20) && (w != NULL)) {
		argv[argc++] = w;
		w = getWord(NULL);
	}
	for (scan = commands; scan; scan = scan->next) {
		if (strcmp(scan->command, argv[0]) == 0) {
			return scan->function(client->dev, argc, argv);
		}
	}
	return -1;
}

void CLIServer::process() {
    CLIClient *scan;
    for (scan = clients; scan; scan = scan->next) {
        if (readline(scan) > 0) {
            int rv = parseCommand(scan);
            if (rv == -1) {
                scan->dev->println("Unknown command");
            }
        }
    }
}

void CLIServer::write(uint8_t c) {
    CLIClient *scan;
    for (scan = clients; scan; scan = scan->next) {
        scan->dev->write(c);
    }
}
