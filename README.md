# Mini-Shell 💻 
A simple shell implemented in **C** 
Mini-Shell simulates basic shell behavior and helps understand core Operating System concepts like process management, pipes, and file redirection ⚙️

---

## Features ✨ 
### Parsing 🔍
✔️ Command parsing and tokenization  
✔️ Detection of operators:
- pipes `|`
- redirection `>` `<`
- background execution `&`

---

### Execution & Error Handling ⚙️ 
✔️ Command execution using `fork()` & `exec()`  
✔️ Process creation and management (Foreground & Background)
✔️ Redirection & Pipes execution
✔️ Error handling:
- invalid commands  
- fork/exec failures  
- file/directory errors  

---

### Built-in Commands 🛠️ 
✔️ Internal shell commands (no exec required):
- `cd` 📂  
- `pwd` 📍  
- `exit` 🚪  
- `history` 🧠  

---

### Signal Handling 📡 
✔️ Handle system signals:
- `Ctrl + C` (SIGINT)  → terminates foreground process only  
- `Ctrl + Z` (SIGTSTP) → temporary pause
- Prevents shell from exiting  

---

### Input / Output Redirection 📤📥
✔️ Redirect input/output streams:
- `>` → write output to file  
- `<` → read input from file  

---

### Pipes 🔗 
✔️ Command chaining using pipes:
- `|` connects output of one process to input of another  

---

### Background Execution ⚡
✔️ Run processes in background using:
- `&`  
✔️ Shell continues accepting new commands

---

## Project Structure 📁
```
📁Mini-Shell
│
├── myShell.c          🧩 Main entry point: handles the shell loop, prompt display, and overall program flow 
│
├── parser.c           🔍 Parse input & detect commands/operators
├── parser.h           📄 Parser declarations
│
├── executor.c         ⚙️ Execute commands (fork, exec, pipes, redirection)
├── executor.h         📄 Executor declarations
│
├── built_ins.c        🛠️ Built-in commands (cd, pwd, exit, history)
├── built_ins.h        📄 Built-ins declarations
│
├── signals.c          📡 Signal handling (Ctrl+C, etc.)
├── signals.h          📄 Signal declarations
│
├── Makefile           🏗️ Build the project using make
│
├── README.md          📄 Documentation

```
## How to Run ▶️
### Compile the project then run executable file (myShell)
```bash
make
./myShell
```

## Team Members 👥
Mustafa Younis (Team Leader)<br>
Youssef Mohammed<br>
Lamia<br>
Manar<br>
Maram Ahmed<br>
Seif<br>
Karim <br>
Abdelrahman Mohammed<br>
Youssef Saiaed<br>

## Author 👤
Mustafa Younis
