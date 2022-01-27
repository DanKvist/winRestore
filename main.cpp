#include <iostream>
#include <fstream>
#include <Windows.h>


int main() {
    std::ofstream logFile;
    
    logFile.open("log.txt");
    if (!logFile.is_open()) {
        std::cout << "Error: file not opened." << std::endl;
        return 1;
    }

    while (true) {
        std::cout << "Hello sir!" << std::endl;
        Sleep(2000);
    }
        

    

    logFile.close();
    return 0;
}