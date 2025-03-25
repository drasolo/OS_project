
# MyShell – Custom Command Line Shell

This project is a custom-built Unix-like shell written in C, designed for educational purposes as part of an Operating Systems course. It provides basic shell functionalities and demonstrates the inner workings of command execution, process management, and file handling.

## 🛠 Features

- Command parsing and execution
- Built-in commands (e.g., `cd`, `exit`)
- Support for background processes (`&`)
- Redirection operators (`<`, `>`)
- Pipelining with `|`
- Error handling and messaging
- Screenshot demos in `/rdme/`

### Prerequisites:
- GCC or Clang
- CMake
- Linux/Unix-based system or WSL

### Build Instructions:
```bash
mkdir build
cd build
cmake ..
make
./myshell
```

## 🖼 Demo

Screenshots of shell functionality are located in `MyShell/rdme/`.

## 📚 Educational Purpose

This project was developed for learning about:
- Process creation with `fork()`
- Command execution with `execvp()`
- File descriptors and redirection
- Handling signals and process statuses

---

*Developed as part of an Operating Systems course assignment.*
