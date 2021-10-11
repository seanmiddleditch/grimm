// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "shell_app.h"

int main(int argc, char* argv[]) {
    up::shell::ShellApp app;

    int rs = app.initialize();
    if (rs != 0) {
        return rs;
    }

    app.run();

    return 0;
}
