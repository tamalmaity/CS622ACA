#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <list>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#define M 20000000

using namespace std;

unordered_set<unsigned long long int> st; //for cold misses
unordered_set<unsigned long long int> L3; //L3 cache
unordered_map<unsigned long long int, vector<int> > mp_cnt;

typedef struct{
  list <unsigned long long int> lst;  //deque
  unordered_map<unsigned long long int, list<unsigned long long int>::iterator>mp; //hashtable
} cache;

int size_l2 = (1<<3);
int size_l3 = (1<<15);
cache L2[1024];

unsigned long long int arr[M];


void hit_l3(unsigned long long int block_addr)
{
   unsigned long long int tag2;
   int l2_set=block_addr %1024;

   tag2 = block_addr>>10;

   if (L2[l2_set].mp.find(tag2) == L2[l2_set].mp.end()) //not present
   {
      if (L2[l2_set].lst.size() == size_l2) //filled set
      {
        unsigned long long int evict = L2[l2_set].lst.back();
        L2[l2_set].lst.pop_back();
        L2[l2_set].mp.erase(evict);
      }
   }
   else
   {
      L2[l2_set].lst.erase(L2[l2_set].mp[tag2]);
   }

   //do this always
   L2[l2_set].lst.push_front(tag2);
   L2[l2_set].mp[tag2] = L2[l2_set].lst.begin();

}


void miss_l3(unsigned long long int block_addr)
{

  unsigned long long int tag2, tag3, evict;
  int l2_set=block_addr %1024;

  tag2 = block_addr>>10;
  tag3 = block_addr;
  int maxm = -1;
  
  if (L3.size() == size_l3)
  {
    for (auto itr = L3.begin(); itr!=L3.end(); itr++)
    {
      if (mp_cnt.find(*itr)==mp_cnt.end()) //not found in mp_cnt
      {
        evict = *itr;
        break;
      }

      if (mp_cnt[*itr][0]>maxm)
    {
      maxm = mp_cnt[*itr][0];
      evict = *itr;
    }
  }

    L3.erase(evict);
  }

  L3.insert(block_addr);


  //invalidate in L2 as well

  int l2_set_evict = evict%1024;
  unsigned long long int l2_tag_evict = evict>>10;

  if(L2[l2_set_evict].mp.find(l2_tag_evict) != L2[l2_set_evict].mp.end())
  {
      for (auto itr = L2[l2_set_evict].lst.begin(); itr != L2[l2_set_evict].lst.end(); itr++)
      {
          if (*itr == l2_tag_evict)
          {
            L2[l2_set_evict].lst.erase(itr);
            break;
          }
      }

      L2[l2_set_evict].mp.erase(l2_tag_evict);

  }



  if (L2[l2_set].lst.size() == size_l2) //filled set
  {
    unsigned long long int evict = L2[l2_set].lst.back();
    L2[l2_set].lst.pop_back();
    L2[l2_set].mp.erase(evict);
  }

  L2[l2_set].lst.push_front(tag2);
  L2[l2_set].mp[tag2] = L2[l2_set].lst.begin();

}



void update_l2(unsigned long long int block_addr)
{
  //cout<<"update_l2\n";
   int l2_set = block_addr%1024;
   unsigned long long int tag2 = block_addr/1024;

   L2[l2_set].lst.erase(L2[l2_set].mp[tag2]);
   L2[l2_set].lst.push_front(tag2);
   L2[l2_set].mp[tag2] = L2[l2_set].lst.begin();

}


void update(unsigned long long int block_addr)
{
  vector<int> v = mp_cnt[block_addr];
  if (v.size()==1) 
  {
    mp_cnt.erase(block_addr);
    return;
  }

  vector <int> :: iterator it;
  it = v.begin();
  v.erase(it);
  mp_cnt[block_addr] = v;
}



int main(int argc, char** argv)
{
   char iord;
   char type;
   char input_name[30];
   unsigned long long int addr;
   unsigned pc;
   FILE *fp;
   int numtraces = atoi(argv[2]);
   int count = 0;
   long cold = 0, cold_n_cap=0;

   for (int k=0; k<numtraces; k++) 
   {      
      sprintf(input_name, "%s_%d", argv[1], k);
      fp = fopen(input_name, "rb");
      assert(fp != NULL);
      while (!feof(fp)) 
      {
         fread(&iord, sizeof(char), 1, fp);
         fread(&type, sizeof(char), 1, fp);
         fread(&addr, sizeof(unsigned long long), 1, fp);
         fread(&pc, sizeof(unsigned), 1, fp);

        

         if ((int)type == 1)
         {
          addr = addr/64;
          arr[count] =  addr;
          if (mp_cnt.find(addr) == mp_cnt.end())
          {
            vector<int> v;
            v.push_back(count);
            mp_cnt[addr] = v;
          }  
          else mp_cnt[addr].push_back(count);

          count++;
         }

      }
      
      fclose(fp);
   }

   /*
   for (auto itr = mp_cnt.begin(); itr!=mp_cnt.end(); itr++)
      {
        sort(itr->second.begin(), itr->second.end());
      } 

      for (auto itr = mp_cnt.begin(); itr!=mp_cnt.end(); itr++)
      {
        cout<<itr->first<<" ";
        for (auto x:itr->second) cout<<x<<" ";
        cout<<endl;
      } 
      */

    for (int i=0;i<count;i++)
    {
       unsigned long long int block_addr;
     unsigned long long int tag2;
     int l2_row,l2_col,l2_set,j;
       int l2_flag=0,l3_flag=0;

        block_addr=arr[i];
        update(block_addr);

        l2_set=block_addr%1024;
        tag2=block_addr/1024;
         
         if(L2[l2_set].mp.find(tag2) != L2[l2_set].mp.end()) //tag present 
         {
            //hits in l2
            l2_flag=1;
            update_l2(block_addr);
         }
           //Block misses in l2,check for l3 cache
         if(!l2_flag)
         {
           //cold miss counter
           if (st.find(block_addr) == st.end())
           {
            cold++;
            st.insert(block_addr);
           }


           if(L3.find(block_addr) != L3.end())
            {
            //hits in l3
                l3_flag=1;
                hit_l3(block_addr);
            }  


           //block misses in L3
           if(!l3_flag)
            { 
              cold_n_cap++;
              miss_l3(block_addr);
            }
       }
    }

   printf("Number of cold misses are: %ld\nNumber of capacity misses are: %ld\n",cold, cold_n_cap-cold);

   return 0;
}
