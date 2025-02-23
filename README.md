# Mayoware

Mayoware is a modular Command and Control (C2) system designed for remote client management and execution of commands. It consists of the following components:

* C2 Server: Manages client connections and executes remote commands.

* Agent: A lightweight client that connects to the C2 server and executes received instructions.

## Installation

### Prerequisites

Ensure you have the following installed:

* GCC (for compiling C-based components)

* Linux (recommended) or Windows with MinGW

## Setup

Clone the repository:

```git clone https://github.com/yourusername/mayoware.git```

```cd mayoware```

Compile the C2 Server:

```gcc -o c2_server c2_server.c -pthread```

Compile the Agent:

```i686-w64-mingw32-gcc -o agent.exe agent.c -lwininet -lwsock32```

Start the C2 Server:

```./c2_server```

Deploy the Agent on a target system and execute it

# Usage

## C2 Server Commands

```clients``` - List connected clients.

```select <index>``` - Interact with a specific client.

```deselect``` - Stop interacting with the selected client.

```exit``` - Shut down the server.

```help``` - Display available commands.

## Commands for Selected Clients

```<command>``` - Execute a shell command.

```exit``` - Exit client interaction mode.

```q``` - Disconnect the selected client.

```cd <dir>``` - Change directory on the client.

```keylog_start``` - Start keylogging.

```persist``` - Enable persistence on the client.

```help``` - Display available commands.

## Disclaimer

This project is intended for educational and research purposes only. The author is not responsible for any misuse or illegal activity involving this software.

## License

This project is licensed under MIT License - see the LICENSE file for details.
