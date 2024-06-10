#include <iostream>
#include <cstdlib>
#include <unistd.h>

void create_virtual_interface(const char* if_name, const char* mac_address) {
    std::string cmd = "ip link add " + std::string(if_name) + " type dummy";
    system(cmd.c_str());
    cmd = "ip link set dev " + std::string(if_name) + " address " + mac_address;
    system(cmd.c_str());
    cmd = "ip link set " + std::string(if_name) + " up";
    system(cmd.c_str());
}

void delete_virtual_interface(const char* if_name) {
    std::string cmd = "ip link delete " + std::string(if_name);
    system(cmd.c_str());
}

void run_in_sandbox(const char* mac_address, const char* executable, char* const argv[]) {
    const char* if_name = "dummy0";

    // Create a new network namespace
    if (unshare(CLONE_NEWNET) == -1) {
        perror("unshare");
        exit(EXIT_FAILURE);
    }

    // Create a virtual network interface and set its MAC address
    create_virtual_interface(if_name, mac_address);

    // Execute the given program
    execvp(executable, argv);

    // Cleanup if execvp fails
    perror("execvp");
    delete_virtual_interface(if_name);
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <MAC_ADDRESS> <EXECUTABLE> [ARGS...]" << std::endl;
        return EXIT_FAILURE;
    }

    const char* mac_address = argv[1];
    const char* executable = argv[2];

    // Prepare arguments for execvp
    char** exec_args = new char*[argc];
    for (int i = 2; i < argc; ++i) {
        exec_args[i-2] = argv[i];
    }
    exec_args[argc-2] = nullptr;

    // Run the executable in a sandbox with the specified MAC address
    run_in_sandbox(mac_address, executable, exec_args);

    // Cleanup
    delete[] exec_args;

    return EXIT_SUCCESS;
}