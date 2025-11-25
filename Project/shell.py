#!/usr/bin/env python3
import os
import shlex
import subprocess

def minishell():
    while True:
        try:
            line = input("$ ").strip()
        except EOFError:
            print()
            break
        if not line:
            continue
        if line == "exit":
            break
        if line == "help":
            print("Comandos: cd <ruta>, exit, tuberías |, redirecciones < > >>, background &")
            print("Ejemplos: ls | grep py > out.txt &")
            continue
        if line.startswith("cd "):
            path = line[3:].strip() or os.path.expanduser("~")
            try:
                os.chdir(path)
            except Exception as e:
                print(str(e))
            continue

        # Soporte de background
        background = False
        if line.endswith("&"):
            background = True
            line = line[:-1].rstrip()

        # Para pipes y redirecciones, delegar al shell
        if any(sym in line for sym in ["|", ">", "<"]):
            proc = subprocess.Popen(line, shell=True)
            if not background:
                proc.wait()
            continue

        # Ejecutar comando simple
        try:
            args = shlex.split(line)
            if not args:
                continue
            proc = subprocess.Popen(args)
            if not background:
                proc.wait()
        except Exception as e:
            print(str(e))

if __name__ == "__main__":
    minishell()