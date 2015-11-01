/*

Program Name: Project OS3

Purpose: The purpose of this project is ...

Input: 
1) Input from keyboard: 
- Input filename
- The total simulation time (in integer seconds)
- The quantum size (in integer milliseconds; usually between 10 and 100)
- The number of processes allowed in the system (degree of multiprogramming - how many jobs are in the system)
2) An input text file contains incoming jobs. The first line is an integer that represents the total number of lines (jobs) in the file. Each subsequent line has four integers: start time of the job, PID, the probability of I/O requests, and the job length.

Output:
1) Output on console:
- Prompt to enter input filename, simulation time, quantum size and the degree of multiprogramming
- Error messages while file not found
- Throughput (number of jobs completed during the simulation)
- Number of jobs still in system
- Number of jobs skipped
- Average job length excluding I/O time
- Average turnaround time
- Average waiting time per process
- Percentage of time CPU is busy (CPU utilization)

Question:
- Are we assuming that the user inputs are in correct form, do we need to handle exceptions?
- 

- We have abided by the Wheaton College honor code in this work.
*/

#include<iostream>
#include<fstream>
#include<stdlib.h>
#include<time.h>
#include<queue>


using namespace std;

//global int PROBABILITY;
time_t IOTimeStart = 0;
bool EnterIO = true;
int IOLength;
const int PENALTYENTERCPU = 4;

int RandomNumber(int start, int endtime);
void IO_Process(int quantum, queue<jobs> &ready, queue<jobs> &IO, int &throughput, int &jobsinsystem);
int CPU(int quantum, queue<jobs> &ready, queue<jobs> &IO, int &throughput, int &jobsinsystem, int systemtimeat);
int JobsStillInSystem(queue<jobs> ready, queue<jobs> IO);
int TotalJobsSkip(queue<jobs> incoming);

queue<jobs> incoming;
queue<jobs> ready;
queue<jobs> IO;

struct jobs
{

	int StartTime;
	int PID;
	int ProbIORequest;
	int Length;

};

int main()
{
	ifstream file;
	string filename;
	int simulationtime;
	int quantumsize;
	int numprocesses;
	int lines;
	srand(time(NULL));
	cout << "Please enter the file name: ";
	// cin >> filename;
	filename = "input.txt";
	file.open(filename.c_str());

	if(file.is_open())
	{
		//incoming incomingqueue;
		//ready readyqueue;
		//IO ioqueue;
		int throughput = 0;
		int jobsinsystem = 0;
		int systemtimeat = 0;
		int jobsskipped = 0;

		cout << "What is the desired simulation time (in seconds)? ";
		cin >> simulationtime;
		simulationtime = simulationtime * 1000;

		cout << "What is the desired quantum size (in milliseconds)? ";
		cin >> quantumsize;

		cout << "What is the number of processes allowed in the system? ";
		cin >> numprocesses;

		jobs nextjob;
		file >> lines;
		for(int i=0; i<lines; i++)
		{
			file >> nextjob.StartTime 
			>> nextjob.PID 
			>> nextjob.ProbIORequest 
			>> nextjob.Length;
			incoming.push(nextjob);

			cout << nextjob.StartTime << "\t"
			<< nextjob.PID << "\t"
			<< nextjob.ProbIORequest << "\t"
			<< nextjob.Length << "\n";
		}

		for(int j=0; j<numprocesses; j++)
		{
			ready.push(incoming.front());
			incoming.pop();
		}

		// remember to remove below
		// cout << "ready queue: ";
		// for(int i=0; i<numprocesses; i++)
		// {
		// jobs temp;
		// temp = ready.front();
		// cout << temp.PID << " ";
		// ready.pop();
		// }
		// cout << "incoming queue: ";
		// for(int i=0; i<(lines-numprocesses); i++)
		// {
		// jobs temp;
		// temp = incoming.front();
		// cout << temp.PID << " ";
		// incoming.pop();
		// }
		// remember to remove above
		// cout << "before while " << endl;
		while(systemtimeat <= simulationtime && (!ready.empty() || !IO.empty()))
		{
			// cout << "inwhile" << endl;
			// cout << "before " << systemtimeat;
			systemtimeat = cpu(quantumsize, ready, IO, throughput, jobsinsystem, systemtimeat);
			IO_Process(quantumsize, ready, IO, throughput, jobsinsystem);
			cout << "simulationtime\t" << simulationtime
			<< "\nsystemtimeat\t" << systemtimeat;
			// cout << "  after " << systemtimeat << endl;
			cout << "Ready empty: " << ready.empty() << endl 
			<< "IO empty: " << IO.empty() << endl;
			cout << endl << endl;
		}
		cout << "afterwhile "<< endl;
		jobsinsystem = JobsStillInSystem(ready, IO);
		jobsskipped = TotalJobsSkip(incoming);

		// Need to change the output placeholders
		cout << "Throughput (number of jobs completed during the simulation):\t" << throughput << endl
		<< "Number of jobs still in system:\t" << jobsinsystem << endl
		<< "Number of jobs skipped:\t" << jobsskipped << endl
		<< "Average job length excluding I/O time:\t" << throughput << " (ms)" << endl
		<< "Average turnaround time:\t" << throughput << " (ms)" << endl
		<< "Average waiting time per process:\t" << throughput << " (ms)" << endl
		<< "CPU utilization (percentage of time CPU is busy):\t" << throughput << '%' << endl;

	}
	else
		cout << "sorry not a valid file" << endl;


	return 0;
}


int RandomNumber(int start, int endtime)
{
	int RandomNumber;
	RandomNumber = rand() % endtime + start;
	return RandomNumber;
}

void IO_Process(int quantum, queue<jobs> &ready, queue<jobs> &IO, int &throughput, int &jobsinsystem){
	if(IOTimeStart != 0 ){
		if ((IOLength < (time(NULL) - IOTimeStart))) {
			EnterIO = false;
		} else {
			EnterIO = true;
		}
	}

	if(!IO.empty() && EnterIO){
		jobs inIO;
		inIO = IO.front();

		IOTimeStart = time(NULL);

		// Pop the job after finished the IO
		IO.pop();

		// Generate initial random value for new process and put it at the end of the ready queue
		IOLength = RandomNumber(10,25);
		ready.push(inIO);
	}
}

int CPU(int quantum, queue<jobs> &ready, queue<jobs> &IO, int &throughput, int &jobsinsystem, int systemtimeat)
{
	if(!ready.empty()){
		jobs incpu;
		incpu = ready.front();
		systemtimeat = systemtimeat + PENALTYENTERCPU;
		ready.pop();

		cout << "-------- cpu -------\n"
		<< incpu.StartTime << "\t"
		<< incpu.PID << "\t"
		<< incpu.ProbIORequest << "\t"
		<< incpu.Length << "\n"
		<< "-------- cpu -------\n";

		incpu.Length = incpu.Length - quantum;
		systemtimeat = systemtimeat + quantum;

		if(incpu.Length > quantum)
		{
			cout << "------ if --------\n";

			int rrandomnumber;
			rrandomnumber = RandomNumber(1,100);
			if(incpu.ProbIORequest <= rrandomnumber)
			{
				cout << "------ if -> if--------\n";
				IO.push(incpu);
			}
			else if(incpu.ProbIORequest > rrandomnumber)
			{
				cout << "------ if -> else if--------\n";
				ready.push(incpu);
			}

		}
		else if(incpu.Length <= quantum)
		{
			cout << "------ else if --------\n";
			throughput++;
			if(!incoming.empty())
			{
				cout << "------ else if -> if --------\n";
				ready.push(incoming.front());
				ready.pop();
			}
		}
	}

	return systemtimeat;
}

int JobsStillInSystem(queue<jobs> ready, queue<jobs> IO)
{
	int jobsinsystem = 0;
	while(!ready.empty())
	{
		jobsinsystem++;
		ready.pop();
	}
	while(!IO.empty())
	{
		jobsinsystem++;
		IO.pop();
	}

	return jobsinsystem;
}

int TotalJobsSkip(queue<jobs> incoming)
{
	int jobsskipped;
	while(!incoming.empty())
	{
		jobsskipped++;
		incoming.pop();
	}
	return jobsskipped;
}