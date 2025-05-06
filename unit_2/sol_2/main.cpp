#include <../dependencies/argparse.hpp>
#include <iostream>
#include "./modules.h"

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("threads_images");

    program.add_argument("-f")
        .help("Set the frames number")
        .default_value(50)
        .scan<'i', int>();

    program.add_argument("-m")
        .help("Set program duration´in minutes")
        .default_value(5)
        .scan<'i', int>();

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
        }

    auto frames = program.get<int>("-f");
    auto minutes = program.get<int>("-m");

    std::cout << frames << std::endl;
    
    std::cout << minutes << std::endl;

    generator();
    return 0;
}
