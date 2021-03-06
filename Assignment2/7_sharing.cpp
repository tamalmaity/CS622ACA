#include <bits/stdc++.h>

using namespace std;

map<long long unsigned, set<int> > mp;


int main(int argc, char* argv[])
{	
	char input_file[100];
	int number;

 	cout<<"Enter number of prog file on which you are running the program: ";
	cin>>number;
 	snprintf(input_file, 30, "mach_acc_%d.txt", number); 
	
	ifstream f(input_file); // for eg. reading mac_acc_4.txt 

	string str;
	while (getline (f, str)) 
	{
		//cout<<str<<endl;
		int tid = str[0]-'0';
		//string containing address being grabbed in s
		int i = 2; //since first character is thread no. and then a space
     	string s = "";
     	while (str[i]!=' ')
     	{
     		s+= str[i];
     		i++;
     	}
     	
     	long long unsigned num = stoull(s)/64;
     	
     	if (mp.find(num)==mp.end())
     	{
     		set<int> s;
     		s.insert(tid);
     		mp[num] = s;
     	} 
     	else mp[num].insert(tid);
     	
	}

	vector<int> result (8,0);

	for (auto itr=mp.begin();itr!=mp.end();itr++)
	{
		int sz = itr->second.size();
		//cout<<sz<<endl;

		result[sz-1]++;
	}


	for (int i=0;i<8;i++)
	{
		if (i==0) cout<<"No. of blocks private to a thread : "<< result[i] <<endl;
		else cout<<"No. of blocks shared by "<< i+1<< " threads : "<<result[i]<<endl;
	}
	

	f.close(); 

	return 0;
}