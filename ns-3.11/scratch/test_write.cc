#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/mf-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/data-rate.h"

#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/ideal-wifi-manager.h"
#include "ns3/wifi-remote-station-manager.h"
#include "ns3/wifi-mode.h"
using namespace ns3;
using namespace std;

void writeFile(string, vector<string>);
void readFile(string, vector<string> &);
unsigned int Random(int,int);
bool Find(vector<string> , string);
void selectNodes(vector<string>);

vector<string> senders;

int main(int argc, char **argv)
{
	vector<string> vect;
	vect.push_back("10.1.1.1");
	vect.push_back("10.1.1.2");
	vect.push_back("10.1.1.3");
	vect.push_back("10.1.1.4");
	vect.push_back("10.1.1.5");
	vect.push_back("10.1.1.6");
	vect.push_back("10.1.1.7");
	
	writeFile("data.txt", vect);
	
	vector<string> ret;
	readFile("data.txt",ret);
	selectNodes(ret);
}

void writeFile(string name, vector<string> vs)
{
	ofstream outfile(name.c_str(), ios::out);
	ostream_iterator<string> oi(outfile, "\n");
	copy(vs.begin(), vs.end(), oi);
}

void readFile(string name, vector<string> &vect)
{	
	ifstream file(name.c_str());
	copy(istream_iterator<string> (file), istream_iterator<string>(), back_inserter(vect));
}

void selectNodes(vector<string> ret)
{
	srand(time(NULL));
		
	string src;
	string dest;
	
	unsigned int s= ret.size()-1;
	cout << s << endl;
	src = ret[Random(1,s)];
	dest = ret[Random(1,s)];

	
	while(Find(senders, src))
	{
		src = ret[Random(1,s)];
	}
	
	while (src == dest)
	{
		src = ret[Random(1,s)];
		if (dest != src)
			break;
	}
	
	cout << "##Source: " << src << std::endl;
	cout << "##Destination: " << dest << std::endl;
	
	senders.push_back(src);
}

unsigned int Random(int nLow, int nHigh)
{
	return (rand() % (nHigh - nLow + 1)) + nLow;
}

bool Find(vector<string> senders, string addr)
{
	for(unsigned int i=0;i<senders.size();i++)
		if(senders[i] == addr)
			return 1;
	return 0;
}
