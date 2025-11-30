if [ ! -d "bin" ]; then
    mkdir bin
else
	rm bin/*
fi
echo "Compiling"
g++ -g -O0 -I . -o bin/interrupts_EP interrupts_101306866_101302780_EP.cpp
# g++ -g -O0 -I . -o bin/interrupts_RR interrupts_101306866_101302780_RR.cpp
# g++ -g -O0 -I . -o bin/interrupts_EP_RR interrupts_101306866_101302780_EP_RR.cpp
echo "Running"
./bin/interrupts_EP ./trace.txt
# ./bin/interrupts_RR ./trace.txt
# ./bin/interrupts_EP_RR ./trace.txt