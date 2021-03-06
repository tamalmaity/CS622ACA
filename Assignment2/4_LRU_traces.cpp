#include <bits/stdc++.h>
#define MAX_LINES 200000000

using namespace std;

typedef struct{
  list <unsigned long long int> lst;  //deque
  unordered_map<unsigned long long int, list<unsigned long long int>::iterator>mp; //hashtable
} cache;

int size_l = (1<<4);


cache L[2048];

void miss_l(unsigned long long int block_addr)
{
  unsigned long long int tag;
  int l_set=block_addr %2048;

  tag = block_addr>>11;
  if (L[l_set].lst.size() == size_l)  
  {
      unsigned long long int evict = L[l_set].lst.back();
      L[l_set].lst.pop_back();
      L[l_set].mp.erase(evict);
   }

  
  L[l_set].lst.push_front(tag);
  L[l_set].mp[tag] = L[l_set].lst.begin();

}


void update_l(unsigned long long int block_addr)
{
   int l_set = block_addr%2048;
   unsigned long long int tag = block_addr/2048;

   L[l_set].lst.erase(L[l_set].mp[tag]);
   L[l_set].lst.push_front(tag);
   L[l_set].mp[tag] = L[l_set].lst.begin();

}

int main(int argc, char** argv)
{ 
  
   unsigned long long int addr, block_addr;
   unsigned long long int tag;
   int thread_id,size;
   long hit = 0, miss = 0, accesses = 0;
   int l_set;
   FILE *trace;

   char file_prefix[100],input_file[100];
	int number;

 	cout<<"Enter number of prog file on which you are running the program: ";
	cin>>number;

 	snprintf(file_prefix, 30, "addrmiss_%d.txt", number); 
	snprintf(input_file, 30, "mach_acc_%d.txt", number); 


   trace = fopen(file_prefix, "w");
   ifstream fp(input_file);

   string str;
   while (getline (fp, str)) 
    { 
      int i = 2; //since first character is thread no. and then a space
      string s = "";
      while (str[i]!=' ')
      {
        s+= str[i];
        i++;
      }
      addr = stoull(s);
      accesses++;
      block_addr=addr/64;
      //cout<<addr<<endl;
      l_set=block_addr%2048;
      tag=block_addr/2048;
         
     if(L[l_set].mp.find(tag) != L[l_set].mp.end()) //tag present 
     {
        //hits in cache
        hit++;
        update_l(block_addr);
     }
       
     else
      {  
        miss++;
        fprintf(trace,"%llu\n",addr);
        miss_l(block_addr);
      }

    }           
       
    
    //fprintf(trace, "#eof");
    
    fclose(trace);
    fp.close();
      
   

   printf("Number of accesses are: %ld\nNumber of hits are: %ld\nNumber of misses are: %ld\n", accesses, hit, miss);

   return 0;
}
