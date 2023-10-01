# _myShell

This is my own personal shell, these are the commands that you can currently do on the shell. 

1. Compile with `make`.
2. Run `./shell`

Commands that work:

1.`echo`
2. `ls`, `ls -l -a`, `ls -l /usr/bin`
3. `cd` <directory>, `cd ../../`, `cd -`, `cd /home`
4. `pwd`
5. Input Output Redirection (ex. `ps aux > a`, `grep /init < a`, `grep /init < a > b`)
6. Single Pipes (ex. 'ls -l | grep "shell.cpp"')
7. Multiple Pipes (ex. `ps aux | awk '/usr/{print $1}' | sort -r`)
8. Multiple Pipes Redirection (ex. `ps aux > test.txt`, `awk '{print $1$11}' < test.txt | head -10 | tr a-z A-Z | sort > output.txt`, `cat output.txt`)
9. `exit`
