#include <iostream>
#include <fstream>
#include <string>
#pragma once

class Logger
{
    public:
        Logger(std::string filename) 
        {
            logFile.open(filename, std::ios::app);     
        }
        ~Logger() { logFile.close();};

        void log(std::string text)
        {
            logFile << text << std::endl;
            logFile.flush();

        }

    private:
        std::ofstream logFile;

};
