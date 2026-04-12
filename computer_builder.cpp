#include <iostream>
#include <vector>
#include <string>

class Computer
{
public:
    std::string motherboard;
    std::string cpu;
    int ram = 0;
    std::string storage;
    std::string power_supply;
    std::string gpu;
};

class Builder
{
private:
    Computer pc;

public:
    
    Builder &setMotherboard(std::string val)
    {
        pc.motherboard = val;
        return *this;
    }

    Builder &setCPU(std::string val)
    {
        pc.cpu = val;
        return *this;
    }

    Builder &setRAM(int val)
    {
        pc.ram = val;
        return *this;
    }

    Builder &setStorage(std::string val)
    {
        pc.storage = val;
        return *this;
    }

    Builder &setPowerSupply(std::string val)
    {
        pc.power_supply = val;
        return *this;
    }

    Builder &setGPU(std::string val)
    {
        pc.gpu = val;
        return *this;
    }

    Computer build()
    {
        return pc;
    }
};