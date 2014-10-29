#include <CLI.h>

CLI_COMMAND(addFunc);
CLI_COMMAND(helpFunc);

void setup() {
    Serial.begin(9600);
    CLI.addClient(Serial);
    CLI.addCommand("add", addFunc);
    CLI.addCommand("help", helpFunc);
}

void loop() {
    CLI.process();
}

CLI_COMMAND(addFunc) {
    if (argc != 2) {
        dev->println("Usage: add <number 1> <number 2>");
        return 10;
    }
    int n1 = atoi(argv[1]);
    int n2 = atoi(argv[2]);
    dev->println(n1 + n2);
    return 0;
}

CLI_COMMAND(helpFunc) {
    dev->println("add <number 1> <number 2> - Add two numbers together");
    return 0;
}
