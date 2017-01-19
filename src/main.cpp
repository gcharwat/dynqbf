/*
Copyright 2016-2017, Guenther Charwat
WWW: <http://dbai.tuwien.ac.at/proj/decodyn/dynqbf>.

This file is part of dynQBF.

dynQBF is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

dynQBF is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with dynQBF.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <iostream>
#include <csignal>


#include "Utils.h"
#include "Application.h"

Application* app;

void signalHandler(int signum) {
    std::cout << "Terminated (signal): " << signum << std::endl;
    if (app != NULL) {
        delete app;
    }
    exit(signum);
}

int main(int argc, char** argv) {
    signal(SIGINT, signalHandler);
    
    int ret = 0;
    try {
        app = new Application(argv[0]);
        ret = app->run(argc - 1, argv + 1);
    } catch (const std::exception& e) {
        std::cerr << std::endl << "Error: " << e.what() << std::endl;
        ret = 2;
    }
    if (app != NULL) {
        delete app;
    }
    return ret;
}
