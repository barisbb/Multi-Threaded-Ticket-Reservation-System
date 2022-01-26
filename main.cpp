

/*
Student Name: Barış Büyüktaş
Compile Status: Compiling
Program Status: Working


Notes  : To run the code         1)  make
                                 2) ./simulation.o configuration_file.txt output.txt

/**/

#include <string.h>
#include <vector>
#include <sstream>
#include <unistd.h>

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <algorithm>


using namespace std;
struct timespec delta = {0 /*secs*/, 30000 /*nanosecs*/};
struct timespec delta1 = {0 /*secs*/, 300000 /*nanosecs*/};

ofstream outFile; //The text file in which the results are written.
vector<vector<int> > memory(301); //The shared memory for passing parameters between client threads and teller threads.


vector<int> seatNumbers; //The occupied seat numbers are held.
int numOfClients; //The total number of clients.
string theatreName; //The name of the theatre, which specifies the total capacity.
int maxCapacity; //The maximum capacity that theaters can take.
int currentClients; //The number of customers at that moment.
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


int findNextSeat(vector<int> seatNumbers){   //It finds the lowest index available seat.
	int result=0;
	for(int i=0;i<numOfClients;i++){
		if(seatNumbers.size()>=maxCapacity){ //If all the seats are already occupied, the algorithm doesn't try to find the next seat.
			break;
		}
		if(seatNumbers[i]!=i+1){
			result=i+1;                      //It finds the next available seat.
			break;

		}
	}

	return result;
}


void *teller_thread (void *param) {
	int serviceTime;
	int seatNumber;
	int order;
	int arrivalTime;

	string teller = *(string *) param;
	outFile << "Teller "+teller+" has arrived.\n";
	while(true){

		int arrivalTime;
		int serviceTime;
		int seatNumber;
		int order;
		int randomValue;
		int count = 0;
		int minArrivalTime=10000;
		for(int i=0;i<memory.size();i++){


			if(memory[i].size()>2){              //After the arrival time, the client thread is waiting in line.

				arrivalTime=memory[i][3];


				if(arrivalTime<minArrivalTime){  //It checks if there are multiple clients waiting in the line.

					pthread_mutex_lock(&mutex);

					serviceTime=memory[i][1];
					seatNumber=memory[i][2];
					order=memory[i][0];          //The parameters are obtained from the shared memory.
					pthread_mutex_unlock(&mutex);

					randomValue=i;
					minArrivalTime=arrivalTime;
				}
			}

		}




		if(currentClients==numOfClients){
			//The threads will be stopped if the number of current customers and
			//the total number of customers are the same.
			pthread_exit(NULL);

		}
		if(randomValue!=0){


			if(teller=="B"){
				nanosleep(&delta, &delta);
			}                                   //If the multiple teller threads are available, A, B, C
			if(teller=="C"){					//choose the clients respectively.
				nanosleep(&delta1, &delta1);;
			}


			pthread_mutex_lock(&mutex);


			if(memory[randomValue].size()>2){  //It allows only one thread to be processed.
				int reservedSeat;
				reservedSeat=seatNumber;
				for(int i=0;i<seatNumbers.size();i++){
					if(seatNumbers[i]==seatNumber || reservedSeat>maxCapacity){ //If the proposed seat is already occupied, the teller threads
						reservedSeat=findNextSeat(seatNumbers);                 //assign next available seat.
						break;
					}
				}
				currentClients=currentClients+1;
				seatNumbers.push_back(reservedSeat);
				sort(seatNumbers.begin(), seatNumbers.end());
				memory[randomValue].clear();
				pthread_mutex_unlock(&mutex);
				usleep(serviceTime*1000);
				//If the system couldn't find the available seats, the reserved seat will be printed "None".
				if(reservedSeat!=0){
					outFile << "Client"+to_string(order)+" requests seat "+to_string(seatNumber)+", reserves seat "+to_string(reservedSeat)+"."+" Signed by Teller "+ teller+".\n";
				}
				else{
					outFile << "Client"+to_string(order)+" requests seat "+to_string(seatNumber)+", reserves None"+"."+" Signed by Teller "+ teller+".\n";
				}
			}

			pthread_mutex_unlock(&mutex);


		}

	}

}
// The client threads are created one by one and pass the parameters to the shared memory
// to communicate with the teller threads.
void *client_thread (void *param) {


	vector<string> threadParams = *(vector<string> *) param;
	string clientName = threadParams[0];
	int arrivalTime = stoi(threadParams[1]);
	int serviceTime = stoi(threadParams[2]);
	int seatNumber = stoi(threadParams[3]);

	usleep(arrivalTime*1000); //It waits for the time to arrive.
	string c = clientName.substr(clientName.find("t")+1,clientName.length()+1);
	int order = stoi(c);
	pthread_mutex_lock(&mutex);
	memory[order].push_back(order);
	memory[order].push_back(serviceTime); //The client threads write the parameters to the shared memory.
	memory[order].push_back(seatNumber);
	memory[order].push_back(arrivalTime);

	pthread_mutex_unlock(&mutex);

	pthread_exit(NULL);

}

int main (int argc,char *argv[])
{
	vector<vector<string> > tokens;
	vector<string> tellerNames={"A","B","C"};

	string outputPath=argv[2];
	outFile.open(outputPath);

	string fileName=argv[1];
	ifstream ifs(fileName);
	string line;
	getline(ifs, line);        //The main thread reads the parameters from the input file.
	theatreName=line;
	getline(ifs, line);
	numOfClients=stod(line);

	pthread_t clients[numOfClients];
	pthread_t tellers[3];

	outFile << "Welcome to the Sync-Ticket!\n";
	for(int i=0;i<3;i++){
		pthread_create(&tellers[i], NULL,teller_thread, &tellerNames[i]); //3 teller threads are created.
		nanosleep(&delta, &delta);
	}


	char firstLetterTheatre=theatreName.at(0);
	if(firstLetterTheatre=='O'){

		maxCapacity=60;
	}
	else if(firstLetterTheatre=='U'){     //Each theatre has own capacity and if there is no available seat in the
		maxCapacity=80;                   //theatre, the customers reserve None.
	}
	else{

		maxCapacity=200;
	}

	while (getline(ifs, line)) {

		stringstream ss(line);
		vector<std::string> item;
		string tmp;                       //The parameters are read one by one.
		while(getline(ss, tmp, ',')) {

			item.push_back(tmp);
		}

		tokens.push_back(item);
	}

	for(int i=0;i<numOfClients;i++){


		pthread_create(&clients[i], NULL,client_thread, &tokens[i]);  // The client threads are created.

	}
	//There are 3 teller threads.
	for(int i=0;i<3;i++){
		pthread_join(tellers[i], NULL);

	}


	for(int i=0;i<numOfClients;i++){ //The number of client threads is specified in input file.
		pthread_join(clients[i], NULL);
	}
	outFile << "All clients received service.\n";
	outFile.close();
	return 0;
}
