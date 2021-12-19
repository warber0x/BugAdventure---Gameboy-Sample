del main.o
del %2
"Gameboy developpment\GB Dev 2020 + Debuggers\gbdk2020\bin\lcc" -Wa-l -Wl-m -Wf--debug -Wl-y -Wl-w -c -o main.o %1
"Gameboy developpment\GB Dev 2020 + Debuggers\gbdk2020\bin\lcc" -Wa-l -Wl-m -Wf--debug -Wl-y -Wl-w -o %2 main.o
