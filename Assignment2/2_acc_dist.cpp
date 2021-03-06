#include <bits/stdc++.h>

using namespace std;

vector<long long unsigned> vec;

FILE * file;

int main(int argc, char* argv[])
{
	char file_prefix[100],input_file[100];
	int number;
//char c;
 	cout<<"Enter number of prog file on which you are running the program: ";
	cin>>number;

 	snprintf(file_prefix, 30, "acc_dist_%d.txt", number); 
 	snprintf(input_file, 30, "mach_acc_%d.txt", number); 
	ifstream f(input_file); // for eg. reading mac_acc_4.txt 
	string str;
	while (getline (f, str)) 
	{
		//cout<<str<<endl;

		
		//string containing address being grabbed in s
		int i = 2; //since first character is thread no. and then a space
     	string s = "";
     	while (str[i]!=' ')
     	{
     		s+= str[i];
     		i++;
     	}

     	long long unsigned num = stoull(s)/64;

     	vec.push_back(num);

	}
	

	
	/*
	// sprintf(programname,"%s",argv[1]);
	//char inputname[100];
	//sprintf(inputname,"MEME.txt");
	tracefile = fopen("MEME.txt", "r");
	assert(tracefile != NULL);
	unsigned long long addr;
	int i= 0;
	while(fscanf(tracefile, "%llu", &addr)) {
	// for(int i=0;i<108800;i++){    //debug
		i++;
		//cout<<addr<<endl;
		vec.push_back(addr/64);
		//size++;
		//cout<<i++<<endl;
		//if(*addr==140734397044872) cout<<*addr<<endl;
	}
	//cout<<"Total size of trace is "<<size<<endl;
	//cout<<"block trace size "<<blocktrace.size()<<endl;
	//closing the file fp
	*/

	map<long long unsigned, vector<int> > mp;

	int sz = vec.size();

	for (int i=0;i<sz;i++)
	{
		if (mp.find(vec[i]) == mp.end())
		{
			vector <int> v;
			v.push_back(i);
			mp[vec[i]] = v;
		}
		else mp[vec[i]].push_back(i);
	}

	int total_accesses = 0;
	for (auto itr = mp.begin(); itr!= mp.end(); itr++)
	{
		total_accesses+= itr->second.size()-1;
	}


	vector <int> a_dist;

	for (auto itr = mp.begin(); itr!= mp.end(); itr++)
	{
		vector<int> v = itr->second;
		int l = v.size();
		if (l==1) continue;

		for (int i=1;i<l;i++) a_dist.push_back(v[i]-v[i-1]);

	}
	
	sort(a_dist.begin(), a_dist.end());

	int a_size = a_dist.size();

	int N = a_dist[a_size-1];

	vector<pair<int,double>> result;

	for (int i=0;i<a_size-1;i++)
	{
		if (a_dist[i]!=a_dist[i+1])
		{
			result.push_back(make_pair(a_dist[i], i+1));
		}
	}

	if (a_dist[a_size-2]==a_dist[a_size-1]) 
	{
		result.pop_back();
	}
	result.push_back(make_pair(a_dist[a_size-1], a_size));


	file = fopen(file_prefix, "w");

	for(int i=0;i<result.size();i++)
	{
		//if (i==0) cout<<result[0]<<endl;
		fprintf(file,"%lf %lf\n", log10(result[i].first),  (result[i].second/a_size));
		//cout<<log10(result[i].first)<<" "<<result[i].second/a_size<<endl;
	}
	cout<<"Total no. of memory blocks : " <<mp.size()<<endl;
	//cout<<total_accesses<<" "<<N<<endl;
	//cout<<N<<" "<<result[N-1]<<endl;
	fclose(file);
	//fclose(tracefile);
	f.close(); 

	return 0;
}