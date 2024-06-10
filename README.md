# Sandbox Program

This project is a C++ program that creates a sandbox environment with a specified MAC address for running executable programs. The sandbox uses Linux network namespaces to ensure that the changes in the network configuration do not affect the host system.

## Features

- Creates a new network namespace
- Sets a custom MAC address for a virtual network interface within the namespace
- Executes specified programs within the sandbox environment

## Requirements

- Linux operating system
- `iproute2` package (for the `ip` command)

## Installation

1. Clone the repository:
    ```sh
    git clone <repository_url>
    cd <repository_directory>
    ```

2. Compile the program:
    ```sh
    g++ -o sandbox sandbox.cpp
    ```

## Usage

Run the program with root privileges to ensure that it can create and manage network namespaces:

```sh
sudo ./sandbox <MAC_ADDRESS> <EXECUTABLE> [ARGS...]