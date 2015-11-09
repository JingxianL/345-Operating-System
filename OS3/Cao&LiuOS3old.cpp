/*

Program Name: Project OS3

Purpose: The purpose of this project is to implement the operating system: NBGOS, it use only one CPU and includes a ready queue and I/O queue.

Input: 
1) Input from keyboard: 
- Input filename
- The total simulation time (in integer seconds)
- The quantum size (in integer milliseconds; usually between 10 and 100)
- The number of processes allowed in the system (degree of multiprogramming - how many jobs are in the system)
2) An input text file contains incoming jobs. The first line is an integer that represents the total number of Lines (jobs) in the file. Each subsequent line has four integers: start time of the job, PID, the probability of I/O requests, and the job length (all units in ms).

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

- We have abided by the Wheaton College honor code in this work.
*/
// Include all the libraries
#include<iostream>
#include<fstream>
#include<stdlib.h>
#include<time.h>
#include<queue>


using namespace std;

//global int penalyzation for entering the cpu =4;

const int PENALTYENTERCPU = 4;

// create a job class;
class jobs
{
public:
	int StartTime;
	int PID;
	int ProbIORequest;
	int Length;
	float IOLength;
	int TurnaroundTime;
	// int WorkTime;
	// int TotalTime;

};

// Declaration for functions 
int RandomNumber(int start, int endtime); // calculating random integers in range start to end
// CPU function that process jobs in the CPU
int CPU(int quantum, queue<jobs> &incoming, queue<jobs> &ready, queue<jobs> &IO, int &Throughput, int &JobsInSystem, int CurrentSystemTime, float &TotalLength, float &TotalTurnaroundTime);
// IO function that process jobs in IO
void IO_Process(int quantum, queue<jobs> &ready, queue<jobs> &IO, int &Throughput, int &JobsInSystem, float &TotalTurnaroundTime, bool &EnterIO, time_t &IOTimeStart);
// Calculate the number of jobs still in the system
int JobsStillInSystem(queue<jobs> ready, queue<jobs> IO);
// Total number of jobs skipped
int TotalJobsSkip(queue<jobs> incoming);


int main()
{	//Declare queues for incoming and ready and IO
	queue<jobs> incoming;
	queue<jobs> ready;
	queue<jobs> IO;
	time_t IOTimeStart = 0;
	bool EnterIO = true;
	// Open file
	ifstream fin;
	string FileName;
	float SimulationTime;
	int QuantumSize;
	int NumProcesses;
	int Lines;
	float TotalLength = 0;
	float AveJobLength;
	float TotalTurnaroundTime = 0;
	float AveTurnaroundTime;
	// float TotalWaitTime = 0;
	// float AveWaitTime;

	srand(time(NULL));
	cout << "Please enter the file name: " << endl;
	cin >> FileName;
	fin.open(FileName.c_str());
	// check if file is open
	if(fin.is_open())
	{
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
		// get jobs into the incoming queue
		for(int i=0; i<Lines; i++)
		{
			fin >> NextJob.StartTime 
			>> NextJob.PID 
			>> NextJob.ProbIORequest 
			>> NextJob.Length;
			NextJob.TurnaroundTime = 0;
			// NextJob.WorkTime = 0;
			// NextJob.TotalTime = 0;
			incoming.push(NextJob);
		}

		// push the numbers of jobs allowed in system to the ready queue
		for(int j=0; j<NumProcesses; j++)
		{
			ready.push(incoming.front());
			// incoming.front().TotalTime = time(NULL);
			TotalLength = TotalLength + incoming.front().Length;
			incoming.pop();
		}
		// call the CPU and IO function to process each job in the queue
		while(CurrentSystemTime <= SimulationTime && (!ready.empty() || !IO.empty()))
		{
			CurrentSystemTime = CPU(QuantumSize, incoming, ready, IO, Throughput, JobsInSystem, CurrentSystemTime, TotalLength, TotalTurnaroundTime);
			IO_Process(QuantumSize, ready, IO, Throughput, JobsInSystem, TotalTurnaroundTime, EnterIO, IOTimeStart);
		}

		//call the job in system function to get how many jobs in the system
		JobsInSystem = JobsStillInSystem(ready, IO);
		// calculate the ave job length 
		AveJobLength = TotalLength/float(Throughput + JobsInSystem);
		//calculate the ave turn around time
		AveTurnaroundTime = TotalTurnaroundTime/float(Throughput + JobsInSystem);
		// AveWaitTime = TotalWaitTime/float(Throughput + JobsInSystem);
		//print output to screen
		cout << "Throughput (number of jobs completed during the simulation):\t" << Throughput << "\n"
		<< "Number of jobs still in system:\t" << JobsInSystem << "\n"
		<< "Number of jobs skipped:\t" << JobsSkipped << "\n"
		<< "Average job length excluding I/O time:\t" << AveJobLength << " (ms)" << "\n"
		<< "Average turnaround time:\t" << AveTurnaroundTime << " (ms)" << "\n";
		// << "Average waiting time per process:\t" << AveWaitTime << " (ms)" << "\n";
		// << "CPU utilization (percentage of time CPU is busy):\t" << Utilization << '%' << "\n";
	}
	else
		cout << "No such file exists!" << endl;

	return 0;
}

int RandomNumber(int start, int endtime)
{
	int RandomNumber;
	RandomNumber = rand() % endtime + start;
	return RandomNumber;
}

int CPU(int quantum, queue<jobs> &incoming, queue<jobs> &ready, queue<jobs> &IO, int &Throughput, int &JobsInSystem, int CurrentSystemTime, float &TotalLength, float &TotalTurnaroundTime)
{
	int newIOProb;
	if(!ready.empty())
	{
		jobs incpu;
		incpu = ready.front();
		CurrentSystemTime = CurrentSystemTime + PENALTYENTERCPU;
		ready.pop();

		if(incpu.Length > quantum)
		{
			incpu.Length = incpu.Length - quantum;
			CurrentSystemTime = CurrentSystemTime + quantum;
			incpu.TurnaroundTime = incpu.TurnaroundTime + quantum;
			// incpu.WorkTime = incpu.WorkTime + quantum;
			// TotalWorkTime = TotalWorkTime + quantum;
			newIOProb = RandomNumber(1,100);
			// incpu.TotalTime += quantum;
			// incpu.TotalTime += PENALTYENTERCPU;

			if(incpu.ProbIORequest >= newIOProb)
			{
				IO.push(incpu);
			}
			else if(incpu.ProbIORequest < newIOProb)
			{
				ready.push(incpu);
			}
		}
		else
		{
			Throughput++;
			CurrentSystemTime = CurrentSystemTime + incpu.Length;
			incpu.TurnaroundTime = incpu.TurnaroundTime + incpu.Length;
			// incpu.WorkTime = incpu.WorkTime + incpu.Length;
			// incpu.TotalTime += incpu.Length;
			TotalTurnaroundTime = TotalTurnaroundTime + incpu.TurnaroundTime;
			incpu.Length = 0;
			// TotalWaitTime = TotalWaitTime +incpu.TotalTime - incpu.WorkTime; 

			if(!incoming.empty())
			{
				ready.push(incoming.front());
				// incoming.front().TotalTime = time(NULL);
				TotalLength = TotalLength + incoming.front().Length;
				incoming.pop();
			}
		}
	}
	return CurrentSystemTime;
}

void IO_Process(int quantum, queue<jobs> &ready, queue<jobs> &IO, int &Throughput, int &JobsInSystem, float &TotalTurnaroundTime, bool &EnterIO, time_t &IOTimeStart){
	if(!IO.empty())
	{
		jobs inIO;
		inIO = IO.front();
		// Pop the job after finished the IO
		IO.pop();

		if(IOTimeStart != 0 )
		{
			if ((inIO.IOLength > (time(NULL) - IOTimeStart)))
			{
				EnterIO = false;
			}
			else
			{
				EnterIO = true;
			}
		}

		if(EnterIO)
		{
			IOTimeStart = time(NULL);

			// Generate initial random value for new process and put it at the end of the ready queue
			inIO.IOLength = RandomNumber(5,25);
			// inIO.TotalTime += inIO.IOLength;
			inIO.TurnaroundTime = inIO.TurnaroundTime + inIO.IOLength;
			// inIO.WorkTime = inIO.WorkTime + inIO.IOLength;
			TotalTurnaroundTime = TotalTurnaroundTime + inIO.TurnaroundTime;
			inIO.IOLength = inIO.IOLength/1000;
			time_t CurrentTime = IOTimeStart;
			while(inIO.IOLength > (CurrentTime - IOTimeStart))
			{
				CurrentTime = time(NULL);
				if(inIO.IOLength <= (CurrentTime - IOTimeStart))
				{
					inIO.ProbIORequest = RandomNumber(1,100);
					ready.push(inIO);
				}
			}
		}
	}
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














