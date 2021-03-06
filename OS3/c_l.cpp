/*

Program Name: Project OS3

Purpose: The purpose of this project is ...

Input: 
1) Input from keyboard: 
- Input filename
- The total simulation time (in integer seconds)
- The quantum size (in integer milliseconds; usually between 10 and 100)
- The number of processes allowed in the system (degree of multiprogramming - how many jobs are in the system)
2) An input text file contains incoming jobs. The first line is an integer that represents the total number of Lines (jobs) in the file. Each subsequent line has four integers: start time of the job, PID, the probability of I/O requests, and the job length.

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
void IO_Process(int quantum, queue<jobs> &ready, queue<jobs> &IO, int &Throughput, int &JobsInSystem);
int CPU(int quantum, queue<jobs> &ready, queue<jobs> &IO, int &Throughput, int &JobsInSystem, int CurrentSystemTime);
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
	ifstream fin;
	string FileName;
	int SimulationTime;
	int QuantumSize;
	int NumProcesses;
	int Lines;
	srand(time(NULL));
	cout << "Please enter the file name: " << endl;
	// cin >> FileName;
	FileName = "input.txt";
	fin.open(FileName.c_str());

	if(fin.is_open())
	{
		//incoming incomingqueue;
		//ready readyqueue;
		//IO ioqueue;
		int Throughput = 0;
		int JobsInSystem = 0;
		int CurrentSystemTime = 0;
		int JobsSkipped = 0;

		cout << "Please enter the desired simulation time (in seconds): ";
		cin >> SimulationTime;
		SimulationTime = SimulationTime * 1000;
		cout << endl;

		cout << "Please enter the desired quantum size (in milliseconds): ";
		cin >> QuantumSize;
		cout << endl;

		cout << "Please enter the number of processes allowed in the system: ";
		cin >> NumProcesses;
		cout << endl;

		jobs NextJob;
		fin >> Lines;
		for(int i=0; i<Lines; i++)
		{
			fin >> NextJob.StartTime 
			>> NextJob.PID 
			>> NextJob.ProbIORequest 
			>> NextJob.Length;
			incoming.push(NextJob);

			cout << NextJob.StartTime << "\t"
			<< NextJob.PID << "\t"
			<< NextJob.ProbIORequest << "\t"
			<< NextJob.Length << "\n";
		}

		for(int j=0; j<NumProcesses; j++)
		{
			ready.push(incoming.front());
			incoming.pop();
		}

		// remember to remove below
		// cout << "ready queue: ";
		// for(int i=0; i<NumProcesses; i++)
		// {
		// jobs temp;
		// temp = ready.front();
		// cout << temp.PID << " ";
		// ready.pop();
		// }
		// cout << "incoming queue: ";
		// for(int i=0; i<(Lines-NumProcesses); i++)
		// {
		// jobs temp;
		// temp = incoming.front();
		// cout << temp.PID << " ";
		// incoming.pop();
		// }
		// remember to remove above
		// cout << "before while " << endl;
		while(CurrentSystemTime <= SimulationTime && (!ready.empty() || !IO.empty()))
		{
			// cout << "inwhile" << endl;
			// cout << "before " << CurrentSystemTime;
			CurrentSystemTime = cpu(QuantumSize, ready, IO, Throughput, JobsInSystem, CurrentSystemTime);
			IO_Process(QuantumSize, ready, IO, Throughput, JobsInSystem);
			cout << "SimulationTime\t" << SimulationTime
			<< "\ncurrentsystemtime\t" << CurrentSystemTime;
			// cout << "  after " << CurrentSystemTime << endl;
			cout << "Ready empty: " << ready.empty() << endl 
			<< "IO empty: " << IO.empty() << endl;
			cout << endl << endl;
		}
		cout << "afterwhile "<< endl;
		JobsInSystem = JobsStillInSystem(ready, IO);
		JobsSkipped = TotalJobsSkip(incoming);

		// Need to change the output placeholders
		cout << "Throughput (number of jobs completed during the simulation):\t" << Throughput << endl
		<< "Number of jobs still in system:\t" << JobsInSystem << endl
		<< "Number of jobs skipped:\t" << JobsSkipped << endl
		<< "Average job length excluding I/O time:\t" << Throughput << " (ms)" << endl
		<< "Average turnaround time:\t" << Throughput << " (ms)" << endl
		<< "Average waiting time per process:\t" << Throughput << " (ms)" << endl
		<< "CPU utilization (percentage of time CPU is busy):\t" << Throughput << '%' << endl;

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

void IO_Process(int quantum, queue<jobs> &ready, queue<jobs> &IO, int &Throughput, int &JobsInSystem){
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

int CPU(int quantum, queue<jobs> &ready, queue<jobs> &IO, int &Throughput, int &JobsInSystem, int CurrentSystemTime)
{
	if(!ready.empty()){
		jobs incpu;
		incpu = ready.front();
		CurrentSystemTime = CurrentSystemTime + PENALTYENTERCPU;
		ready.pop();

		cout << "-------- cpu -------\n"
		<< incpu.StartTime << "\t"
		<< incpu.PID << "\t"
		<< incpu.ProbIORequest << "\t"
		<< incpu.Length << "\n"
		<< "-------- cpu -------\n";

		incpu.Length = incpu.Length - quantum;
		CurrentSystemTime = CurrentSystemTime + quantum;

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
			Throughput++;
			if(!incoming.empty())
			{
				cout << "------ else if -> if --------\n";
				ready.push(incoming.front());
				ready.pop();
			}
		}
	}

	return CurrentSystemTime;
}

int JobsStillInSystem(queue<jobs> ready, queue<jobs> IO)
{
	int JobsInSystem = 0;
	while(!ready.empty())
	{
		JobsInSystem++;
		ready.pop();
	}
	while(!IO.empty())
	{
		JobsInSystem++;
		IO.pop();
	}

	return JobsInSystem;
}

int TotalJobsSkip(queue<jobs> incoming)
{
	int JobsSkipped;
	while(!incoming.empty())
	{
		JobsSkipped++;
		incoming.pop();
	}
	return JobsSkipped;
}