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
const int PENALTYENTERCPU = 4;

struct jobs
{

	int StartTime;
	int PID;
	int ProbIORequest;
	int Length;
	float IOLength;
	int TurnaroundTime;
	int WorkTime;
	int TotalTime;

};

queue<jobs> incoming;
queue<jobs> ready;
queue<jobs> IO;

int RandomNumber(int start, int endtime);
int CPU(int quantum, queue<jobs> &ready, queue<jobs> &IO, int &Throughput, int &JobsInSystem, int CurrentSystemTime, float &TotalLength, float &TotalTurnaroundTime, float &TotalWaitTime);
void IO_Process(int quantum, queue<jobs> &ready, queue<jobs> &IO, int &Throughput, int &JobsInSystem, float &TotalTurnaroundTime);
int JobsStillInSystem(queue<jobs> ready, queue<jobs> IO);
int TotalJobsSkip(queue<jobs> incoming);


ofstream fout("output.txt");



int main()
{
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
	float TotalWaitTime = 0;
	float AveWaitTime;


	srand(time(NULL));
	cout << "Please enter the file name: " << endl;
	// cin >> FileName;
	FileName = "input.txt";
	fin.open(FileName.c_str());

	if(fin.is_open())
	{

		int Throughput = 0;
		int JobsInSystem = 0;
		int CurrentSystemTime = 0;
		// int JobsSkipped = 0;
		

		cout << "Please enter the desired simulation time (in seconds): ";
		//cin >> SimulationTime;
		SimulationTime = 2.1;
		SimulationTime = SimulationTime * 1000;
		cout << endl;

		cout << "Please enter the desired quantum size (in milliseconds): ";
		//cin >> QuantumSize;
		QuantumSize = 50;
		cout << endl;

		cout << "Please enter the number of processes allowed in the system: ";
		//cin >> NumProcesses;
		NumProcesses = 4;
		cout << endl;

		jobs NextJob;
		fin >> Lines;
		for(int i=0; i<Lines; i++)
		{
			fin >> NextJob.StartTime 
			>> NextJob.PID 
			>> NextJob.ProbIORequest 
			>> NextJob.Length;
			NextJob.TurnaroundTime = 0;
			NextJob.WorkTime = 0;
			NextJob.TotalTime = 0;
			incoming.push(NextJob);

			// cout << NextJob.StartTime << "\t"
			// << NextJob.PID << "\t"
			// << NextJob.ProbIORequest << "\t"
			// << NextJob.Length << "\n";

			fout << NextJob.StartTime << "\t"
			<< NextJob.PID << "\t"
			<< NextJob.ProbIORequest << "\t"
			<< NextJob.Length << "\n";
		}

		for(int j=0; j<NumProcesses; j++)
		{
			ready.push(incoming.front());
			// incoming.front().TotalTime = time(NULL);
			TotalLength = TotalLength + incoming.front().Length;
			// cout << "TotalLength is " << TotalLength << "\n";
			incoming.pop();
		}


		while(CurrentSystemTime <= SimulationTime && (!ready.empty() || !IO.empty()))
		{
			// cout << " CurrentSystemTime is :  " << CurrentSystemTime << "\n";
			fout << " CurrentSystemTime is :  " << CurrentSystemTime << "\n";
			
			CurrentSystemTime = CPU(QuantumSize, ready, IO, Throughput, JobsInSystem, CurrentSystemTime, TotalLength, TotalTurnaroundTime, TotalWaitTime);
			IO_Process(QuantumSize, ready, IO, Throughput, JobsInSystem, TotalTurnaroundTime);
		}


		// JobsSkipped = TotalJobsSkip(incoming);

		JobsInSystem = JobsStillInSystem(ready, IO);

		AveJobLength = TotalLength/float(Throughput + JobsInSystem);
	
		AveTurnaroundTime = TotalTurnaroundTime/float(Throughput + JobsInSystem);

		AveWaitTime = TotalWaitTime/float(Throughput + JobsInSystem);

		// -----------------------------------------------------------------------------------------
		// cout << "Throughput (number of jobs completed during the simulation):\t" << Throughput << "\n"
		// << "Number of jobs still in system:\t" << JobsInSystem << "\n"
		// // << "Number of jobs skipped:\t" << JobsSkipped << "\n"
		// << "Average job length excluding I/O time:\t" << AveJobLength << " (ms)" << "\n"
		// << "Average turnaround time:\t" << AveTurnaroundTime << " (ms)" << "\n"
		// // << "Average waiting time per process:\t" << AveWaitTime << " (ms)" << "\n";
		// // << "CPU utilization (percentage of time CPU is busy):\t" << Throughput << '%' << "\n";

		fout << "Throughput (number of jobs completed during the simulation):\t" << Throughput << "\n"
		<< "Number of jobs still in system:\t" << JobsInSystem << "\n"
		// << "Number of jobs skipped:\t" << JobsSkipped << "\n"
		<< "Average job length excluding I/O time:\t" << AveJobLength << " (ms)" << "\n"
		<< "Average turnaround time:\t" << AveTurnaroundTime << " (ms)" << "\n"
		<< "Average waiting time per process:\t" << AveWaitTime << " (ms)" << "\n";
		// << "CPU utilization (percentage of time CPU is busy):\t" << Throughput << '%' << "\n";



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


int CPU(int quantum, queue<jobs> &ready, queue<jobs> &IO, int &Throughput, int &JobsInSystem, int CurrentSystemTime, float &TotalLength, float &TotalTurnaroundTime, float &TotalWaitTime)
{
	int newIOProb;
	if(!ready.empty()){
		jobs incpu;
		incpu = ready.front();
		CurrentSystemTime = CurrentSystemTime + PENALTYENTERCPU;
		ready.pop();


		// ---------------------------------------------------------
		// cout << "-------- cpu -------\n"
		// << incpu.StartTime << "\t"
		// << incpu.PID << "\t"
		// << incpu.ProbIORequest << "\t"
		// << incpu.Length << "\n"
		// << "-------- cpu -------\n";
		fout << "-------- cpu -------\n"
		<< incpu.StartTime << "\t"
		<< incpu.PID << "\t"
		<< incpu.ProbIORequest << "\t"
		<< incpu.Length << "\n"
		<< "-------- cpu -------\n";


		if(incpu.Length > quantum)
		{
			incpu.Length = incpu.Length - quantum;
			CurrentSystemTime = CurrentSystemTime + quantum;
			incpu.TurnaroundTime = incpu.TurnaroundTime + quantum;
			incpu.WorkTime = incpu.WorkTime + quantum;
			// TotalWorkTime = TotalWorkTime + quantum;
			newIOProb = RandomNumber(1,100);
			incpu.TotalTime += quantum;
			incpu.TotalTime += PENALTYENTERCPU;
			// cout << incpu.Length << "\t"
			// << newIOProb << "\n";

			fout << incpu.Length << "\t"
			<< newIOProb << "\n";

			if(incpu.ProbIORequest >= newIOProb)
			{
				// cout << "IO PUSH" << endl;
				fout << "IO PUSH" << "\n";
				IO.push(incpu);
			}
			else if(incpu.ProbIORequest < newIOProb)
			{
				// cout << "not" << endl;
				fout << "not" << "\n";
				ready.push(incpu);
			}
		}
		else
		{
			Throughput++;
			CurrentSystemTime = CurrentSystemTime + incpu.Length;
			incpu.TurnaroundTime = incpu.TurnaroundTime + incpu.Length;
			incpu.WorkTime = incpu.WorkTime + incpu.Length;
			incpu.TotalTime += incpu.Length;
			TotalTurnaroundTime = TotalTurnaroundTime + incpu.TurnaroundTime;
			incpu.Length = 0;


			cout << "incpu.TotalTime is " << incpu.TotalTime << "\t"
				<< "work" << incpu.WorkTime << "\n";
			TotalWaitTime = TotalWaitTime +incpu.TotalTime - incpu.WorkTime; 
			cout << "TotalWaitTime is " << TotalWaitTime << "\n";

			if(!incoming.empty())
			{
				ready.push(incoming.front());
				incoming.front().TotalTime = time(NULL);
				TotalLength = TotalLength + incoming.front().Length;
				// cout << "Incpu TotalLength is :" << TotalLength << "\n";
				incoming.pop();
			}
		}

	}

	return CurrentSystemTime;
}


void IO_Process(int quantum, queue<jobs> &ready, queue<jobs> &IO, int &Throughput, int &JobsInSystem, float &TotalTurnaroundTime){
	if(!IO.empty())
	{
		jobs inIO;
		inIO = IO.front();
		// Pop the job after finished the IO
		IO.pop();


		if(IOTimeStart != 0 ){
			// cout << "enter IOTimeStart " << endl;
			fout << "enter IOTimeStart " << "\n";
			if ((inIO.IOLength > (time(NULL) - IOTimeStart))) {
				EnterIO = false;
				// cout << "false" << endl;
				fout << "false" << "\n";
			} else {
				EnterIO = true;
				// cout << "true" << endl;
				fout << "true" << "\n";
			}
		}

		if(EnterIO){

			IOTimeStart = time(NULL);

			// Generate initial random value for new process and put it at the end of the ready queue
			inIO.IOLength = RandomNumber(5,25);
			inIO.TotalTime += inIO.IOLength;
			inIO.TurnaroundTime = inIO.TurnaroundTime + inIO.IOLength;
			inIO.WorkTime = inIO.WorkTime + inIO.IOLength;
			TotalTurnaroundTime = TotalTurnaroundTime + inIO.TurnaroundTime;
			//cout << "IO IOLength1 is :" << inIO.IOLength << endl;
			inIO.IOLength = inIO.IOLength/1000;
			// cout << "IO IOLength2 is :" << inIO.IOLength << endl;
			fout << "IO IOLength2 is :" << inIO.IOLength << "\n";
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


/*
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
*/