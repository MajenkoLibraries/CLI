#include <CLI.h>

CLI_COMMAND(addFunc);
CLI_COMMAND(helpFunc);
CLI_COMMAND(connectFunc);

void setup() {
    Serial.begin(9600);
    CLI.setDefaultPrompt("> ");
    CLI.onConnect(connectFunc);
    
    CLI.addCommand("add", addFunc);
    CLI.addCommand("help", helpFunc);

    CLI.addClient(Serial);
}

void loop() {
    CLI.process();
}

CLI_COMMAND(addFunc) {
    if (argc != 3) {
        dev->println("Usage: add <number 1> <number 2>");
        return 10;
    }
    int n1 = atoi(argv[1]);
    int n2 = atoi(argv[2]);
    dev->print(n1);
    dev->print(" + ");
    dev->print(n2);
    dev->print(" = ");
    dev->println(n1 + n2);
    return 0;
}

CLI_COMMAND(helpFunc) {
    dev->println("add <number 1> <number 2> - Add two numbers together");
    return 0;
}

CLI_COMMAND(connectFunc) {
    dev->println("Welcome to the CLI test.");
    dev->println("Type 'help' to list commands.");
    dev->println();
    dev->printPrompt();
}

