/**
 * @file interrupts.cpp
 * @author Sasisekhar Govind
 * @brief template main.cpp file for Assignment 3 Part 1 of SYSC4001
 * 
 */

#include "interrupts_101306866_101302780.hpp"

void ExternalPriority(std::vector<PCB> &ready_queue) {
    std::sort(
        ready_queue.begin(),
        ready_queue.end(),
        [](const PCB &a, const PCB &b){
            return a.PID < b.PID; // lower number = higher priority
        }
    );
}


std::tuple<std::string /* add std::string for bonus mark */ > run_simulation(std::vector<PCB> list_processes) {

    std::vector<PCB> ready_queue;   //The ready queue of processes
    std::vector<PCB> wait_queue;    //The wait queue of processes
    std::vector<PCB> job_list;      //A list to keep track of all the processes. This is similar
                                    //to the "Process, Arrival time, Burst time" table that you
                                    //see in questions. You don't need to use it, I put it here
                                    //to make the code easier :).

    unsigned int current_time = 0;
    const int context_time = 10; // context switch time
    const int isr_time = 50; // ISR time
    bool cpu_idle = true;
    PCB running;
    unsigned int time_since_io = 0; // Track time since last I/O

    //Initialize an empty running process
    idle_CPU(running);

    std::string execution_status;

    //make the output table (the header row)
    execution_status = print_exec_header();

    //Loop while till there are no ready or waiting processes.
    //This is the main reason I have job_list, you don't have to use it.
    while(!all_process_terminated(job_list) || job_list.empty()) {

        //Inside this loop, there are three things you must do:
        // 1) Populate the ready queue with processes as they arrive
        // 2) Manage the wait queue
        // 3) Schedule processes from the ready queue

        //Population of ready queue is given to you as an example.
        //Go through the list of proceeses
        for(auto &process : list_processes) {
            if(process.arrival_time == current_time) {//check if the AT = current time
                //if so, assign memory and put the process into the ready queue
                assign_memory(process);

                process.state = READY;  //Set the process state to READY
                ready_queue.push_back(process); //Add the process to the ready queue
                job_list.push_back(process); //Add it to the list of processes

                execution_status += print_exec_status(current_time, process.PID, NEW, READY);
                
                sync_queue(job_list, process);
            }
        }

        //////////////////////////SCHEDULER//////////////////////////////
        ExternalPriority(ready_queue); 
        
        //Run the process if the ready queue is not empty and CPU is idle
        if (cpu_idle && !ready_queue.empty()) {
            run_process(running, job_list, ready_queue, current_time);
            execution_status += print_exec_status(current_time, running.PID, READY, RUNNING);
            cpu_idle = false;
            time_since_io = 0; //Reset I/O counter when process starts/resumes
        }
        
        // Decrementing the remaining time on process execution
        if (!cpu_idle && running.remaining_time > 0) {
            running.remaining_time--;
            time_since_io++;
            sync_queue(job_list, running);
        }
        
        // Decrementing I/O duration for processes already in wait queue
        for(auto &waiting : wait_queue) {
            waiting.io_duration--;
            sync_queue(job_list, waiting);
        }
        
        current_time++; //Increment the sim time
        
        // Check if process needs I/O after time increment
        if (!cpu_idle && running.io_freq != 0 && time_since_io == running.io_freq && running.remaining_time > 0) {
            // Save original io_duration from PCB in process list
            for(auto &p : list_processes) {
                if(p.PID == running.PID) {
                    running.io_duration = p.io_duration;
                    break;
                }
            }
            
            execution_status += print_exec_status(current_time, running.PID, RUNNING, WAITING);
            running.state = WAITING;
            sync_queue(job_list, running);
            wait_queue.push_back(running);  // Add AFTER decrementing existing ones
            cpu_idle = true;
            idle_CPU(running);
        }
        
        // Check if running process has completed after sim time increment
        if (!cpu_idle && running.remaining_time == 0) {
            execution_status += print_exec_status(current_time, running.PID, RUNNING, TERMINATED);
            terminate_process(running, job_list);
            cpu_idle = true;
        }
        
        ///////////////////////MANAGE WAIT QUEUE/////////////////////////
        // Check if any processes in wait queue have completed I/O
        for(auto it = wait_queue.begin(); it != wait_queue.end(); ) {
            if(it->io_duration == 0) {
                it->state = READY;
                execution_status += print_exec_status(current_time, it->PID, WAITING, READY);
                ready_queue.push_back(*it);
                sync_queue(job_list, *it);
                it = wait_queue.erase(it);
            } else {
                ++it;
            }
        }
        /////////////////////////////////////////////////////////////////
    }
    
    //Close the output table
    execution_status += print_exec_footer();

    return std::make_tuple(execution_status);
}


int main(int argc, char** argv) {

    //Get the input file from the user
    if(argc != 2) {
        std::cout << "ERROR!\nExpected 1 argument, received " << argc - 1 << std::endl;
        std::cout << "To run the program, do: ./interrutps <your_input_file.txt>" << std::endl;
        return -1;
    }

    //Open the input file
    auto file_name = argv[1];
    std::ifstream input_file;
    input_file.open(file_name);

    //Ensure that the file actually opens
    if (!input_file.is_open()) {
        std::cerr << "Error: Unable to open file: " << file_name << std::endl;
        return -1;
    }

    //Parse the entire input file and populate a vector of PCBs.
    //To do so, the add_process() helper function is used (see include file).
    std::string line;
    std::vector<PCB> list_process;
    while(std::getline(input_file, line)) {
        auto input_tokens = split_delim(line, ", ");
        auto new_process = add_process(input_tokens);
        list_process.push_back(new_process);
    }
    input_file.close();

    //With the list of processes, run the simulation
    auto [exec] = run_simulation(list_process);

    write_output(exec, "execution.txt");

    return 0;
}