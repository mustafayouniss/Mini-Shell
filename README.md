# Mini-Shell 💻 
A simple shell implemented in **C** 
Mini-Shell simulates basic shell behavior and helps understand core Operating System concepts like process management, pipes, and file redirection ⚙️

---

## Features ✨ 
✔️ Command execution using `fork()` & `exec()`  
✔️ Built-in commands:
- `cd` 
- `pwd`
- `exit`
- `history`

✔️ Input / Output Redirection:
- `>` 📤 (output to file)  
- `<` 📥 (input from file)  

✔️ Pipes support:
- `|` 🔗 (connect commands together)  

✔️ Background execution:
- `&` ⚡ (run processes in background)  

✔️ Signal handling:
- `Ctrl + C` 🛑 (terminate foreground process only)

✔️ Basic error handling 🚨

---

## Project Structure 📁
```
📁Mini-Shell
│
├── mini_shell.c       🧩 Main entry point: handles the shell loop, prompt display, and overall program flow 
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
├── make               🏗️ Build the project using make
│
├── README.md          📄 Documentation

```
## How to Run ▶️
### Compile the project then run executable file (mini-shell)
```bash
make
./mini_shell
```

## Team Members 👥
Mustafa Younis (Team Leader)<br>
Youssef Mohammed<br>
Lmiaa<br>
Manar<br>
Maram Ahmed<br>
Seif<br>
Karim <br>
Abdelrahman Mohammed<br>
Youssef Saied<br>

## Author 👤
Mustafa Younis