#include <iostream>
#include <fstream>

int main() {
    std::ofstream logFile;
    
    logFile.open("log.txt");
    if (!logFile.is_open()) {
        std::cout << "Error: file not opened." << std::endl;
        return 1;
    }
        

    std::cout << "Hello sir!" << std::endl;

    logFile.close();
    return 0;
}