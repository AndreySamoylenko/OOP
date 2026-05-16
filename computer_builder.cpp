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

    void print_info(){
        std::cout << "MB : " << motherboard << std::endl 
                  << "CPU : " << cpu << std::endl 
                  << "RAM amount : " << ram << "GB" << std::endl 
                  << "Storage : " << storage << std::endl 
                  << "PSU : " << power_supply << std::endl 
                  << "GPU : " << gpu << std::endl ;
    }
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
        Computer result = pc;
        pc = Computer();
        return result;
    }
};


int main(){
    Builder b;
    Computer pc = b.setCPU("ryzen 9000")
                   .setMotherboard("Asus tuf 2027 se")
                   .setPowerSupply("KZAS 900w")
                   .setRAM(32)
                   .setGPU("amd 9080")
                   .setStorage("SSD : 2TB, HDD : 2PB")
                   .build();

    pc.print_info();
    return 0;
}