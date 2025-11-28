if [ ! -d "bin" ]; then
    mkdir bin
else
	rm bin/*
fi
h
g++ -g -O0 -I . -o bin/interrupts_EP interrupts_101306866_101302780_EP.cpp
g++ -g -O0 -I . -o bin/interrupts_RR interrupts_101306866_101302780_RR.cpp
g++ -g -O0 -I . -o bin/interrupts_EP_RR interrupts_101306866_101302780_EP_RR.cpp