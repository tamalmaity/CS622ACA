#include <bits/stdc++.h>

#define ull unsigned long long

using namespace std;

typedef struct{
   ull address;
   long int lru;
   int valid;
} cache;

int err = 0;

///********************************

/// While implementing PEnding state change the ownership when xfer_inv is recvd 


///********************************



cache L2[4096][16];
cache L1[8][64][8]; //1st index(0-7) to get that L1 cache

long long int l1_cache_acc[8] = {0};
long long int l1_cache_hit[8] = {0};
long long int l1_cache_miss[8] = {0};
long long int l1_cache_upgr_miss[8] = {0};
long long int l2_cache_miss[8] = {0};


/*
index of msgs :
0 get, 1 getx, 
2 put, 3 putx,
4 swb, 5 upgrade,
6 inv, 7 inv_ack,
8 nack, 9 xfer,
10 xfer_inv, 11 repl_s
*/
long long int L1_msg[8][12];
long long int L2_msg[8][12];

map<vector<ull>, int> outstanding; // <addr, tid> and  0 for load, 1 for store

map<ull, vector <int>> dir;
map<ull, int> shared_state; //block no . -> 0(modified), 1(present)
map<vector<ull>, int> nack_buffer; //keeps track of timeout counter <<core, addr>, int>
// <<id,tid, addr> , counter>

typedef struct 
{
  ull count;
  ull address;
  int tid;
  int mode;
} ts;

vector <queue<ts>> q1trace(8);
vector <queue<vector<ull>>> q1inp(8);
vector <queue<vector<ull>>> q2inp(8); //each of them is a vector of 8 queues
//0->id of msg, 1->tid, 2->addr

ull global_cnt = 0;
ull cycles = 0;

int compare_func(const void *a, const void *b)
{
  ts *x = (ts *)a;
  ts *y = (ts *)b;
  return (x->count - y->count);
}

int hit_l1 (ull addr, int tid)
{
    ull l1_set = addr%64;
    ull tag1 = addr/64;

    for (int i=0;i<8;i++)
    {
        if (L1[tid][l1_set][i].address == tag1 && L1[tid][l1_set][i].valid == 1)
        {
            return 1;
        }
    }

    return 0;
}


int check_l2( ull addr)
{
    ull l2_set = addr%4096;
    ull tag2 = addr/4096;

    for (int i=0;i<16;i++)
    {
        if (L2[l2_set][i].address == tag2 && L2[l2_set][i].valid == 1)
        {
            return 1;
        }
    }
    return 0;
}

void invoke_l1_replace(ull addr, int tid)
{
   int i,ind; //ind is the index which we want to replace
   long long maxm = -1;
   ull l1_set=addr%64;
   ull tag1 = addr/64;
   ull to_evict;
   for (i=0;i<8;i++)
   {
      if (L1[tid][l1_set][i].lru>maxm)
      {
         maxm = L1[tid][l1_set][i].lru;
         ind = i;
      }
   }
   to_evict = L1[tid][l1_set][ind].address; 
   L1[tid][l1_set][ind].address = tag1;
   L1[tid][l1_set][ind].lru = 0;

   to_evict = (to_evict*64) + l1_set;

   if (shared_state.find(to_evict) != shared_state.end())
   {
    if (shared_state[to_evict]==0) //in modified
    {
      dir.erase(to_evict);
      shared_state.erase(to_evict);
      
      vector <ull> vec(3);  //0->id of msg, 1->tid, 2->addr
      vec[0] = 11; //since repl_s message
      vec[1] = tid;
      vec[2] = addr;

      int bid = to_evict%8;
      q2inp[bid].push(vec);
    }
    else //in shared
    {
      if (dir.find(to_evict)!=dir.end())
      {
        vector <int> temp;
        for (auto x : dir[to_evict])
        {
          if (tid != x) temp.push_back(x);
        }
        if (temp.size()==0) 
        {
          dir.erase(to_evict);
          shared_state.erase(to_evict);
        }
        else dir[to_evict] = temp;
      }
    }
   }


   for(i=0;i<8;i++)
    {
      if (i!=ind) L1[tid][l1_set][i].lru++;
    }
}

void invalid_l1(ull addr, int tid)
{
  
   int i;
   ull l1_set = addr%64;
   ull tag1 = addr/64;
   for(i=0;i<8;i++)
    {
      if (L1[tid][l1_set][i].address == tag1 && L1[tid][l1_set][i].valid == 1) 
      {
          //vector<ull> v; v.push_back(addr), v.push_back(tid);
          //if (outstanding.find(v)!=outstanding.end()) outstanding.erase(v);
          L1[tid][l1_set][i].valid=0;

          /*
         //sending repl_s

         if (shared_state.find(addr) != shared_state.end() && shared_state[addr] == 0) //modified
         {
            shared_state.erase(addr);
            dir.erase(addr);
         

         vector <ull> vec(3);  //0->id of msg, 1->tid, 2->addr
          vec[0] = 11; //since repl_s message
          vec[1] = tid;
          vec[2] = addr;

          int bid = addr%8;
          q2inp[bid].push(vec);

        }
        */

         break;
      }
   }
}

//Filled set, remove some block and place tag2
void invoke_l2_replace(ull addr, int tid)
{
   int i,ind,maxm = -1;
   ull evict_block, tag2 = addr/4096;
   ull l2_set = addr%4096;
   for (i=0;i<16;i++)
   {
      if (L2[l2_set][i].lru>maxm)
      {
         maxm = L2[l2_set][i].lru;
         ind = i;
      }
   }
   evict_block= L2[l2_set][ind].address;
   L2[l2_set][ind].address = tag2;
   L2[l2_set][ind].lru = 0;

   for(i=0;i<16;i++)
    {
      if (i!=ind) L2[l2_set][i].lru++;
    }

   evict_block = (evict_block*4096)+l2_set;

   if (shared_state.find(evict_block)!=shared_state.end() && dir.find(evict_block)!=dir.end())
   {
    shared_state.erase(evict_block);
    dir.erase(evict_block);
   }

   invalid_l1(evict_block, tid);

}

void update_l1(ull addr, int tid)
{
  l1_cache_hit[tid]++;
   int i;
   ull l1_set=addr%64;
   ull tag1 = addr/64;
   for (i=0;i<8;i++)
   {
      if (L1[tid][l1_set][i].address == tag1)
      { 
        L1[tid][l1_set][i].lru = 0;
      }

      else if (L1[tid][l1_set][i].address != tag1 && L1[tid][l1_set][i].valid == 1)
          L1[tid][l1_set][i].lru++;
   }
}

void update_l2(ull addr)
{
   int i;
   ull l2_set=addr%4096;
   ull tag2 = addr/4096;
   for (i=0;i<16;i++)
   {
      if (L2[l2_set][i].address == tag2) 
        {
          L2[l2_set][i].lru = 0;
        }
      else if (L2[l2_set][i].address != tag2 && L2[l2_set][i].valid == 1)
          L2[l2_set][i].lru++;
   }
}

void updatex_l1(ull addr, int tid)
{
   int i;
   ull l1_set=addr%64;
   ull tag1 = addr/64;
   if (shared_state.find(addr) != shared_state.end() && shared_state[addr] == 0) //modified state
   { 
    /*
    if (addr==401136) 
    {
      if (dir.find(addr)!=dir.end()) cout<<"found"<<endl;
      else cout<<"not found"<<endl;
    }*/
      if (dir.find(addr)!=dir.end() && dir[addr][0]==tid) //itself in modified state
      {
        l1_cache_hit[tid]++;

         for (i=0;i<8;i++)
         {
            if (L1[tid][l1_set][i].address == tag1) 
            {
              L1[tid][l1_set][i].lru = 0;
            }

            else if (L1[tid][l1_set][i].address != tag1 && L1[tid][l1_set][i].valid == 1)
                L1[tid][l1_set][i].lru++;
         }
      }

      else //someone else in modified state
      {
          l1_cache_upgr_miss[tid]++;

          vector <ull> vec(3);  //0->id of msg, 1->tid, 2->addr
          vec[0] = 5; //since upgrade message
          vec[1] = tid;
          vec[2] = addr;

          int bid = addr%8;
          q2inp[bid].push(vec);
      }
   }
   else if (shared_state.find(addr) != shared_state.end() && shared_state[addr] == 1) //shared 
   {
      l1_cache_upgr_miss[tid]++;

      vector <ull> vec(3);  //0->id of msg, 1->tid, 2->addr
      vec[0] = 5; //since upgrade message
      vec[1] = tid;
      vec[2] = addr;

      int bid = addr%8;
      q2inp[bid].push(vec);
   }
}

void fill_in_L2(ull addr, int tid)
{
    int i,empty_flag=0;
    ull l1_set,l2_set;
    ull tag1,tag2;
   
    l2_set = addr%4096;
    tag2 = addr/4096;
     for(i=0;i<16;i++)
     {
        if(L2[l2_set][i].valid==0)
        { 
           L2[l2_set][i].address=tag2;
           L2[l2_set][i].lru=0;
           L2[l2_set][i].valid=1;

           int j;
           for (j=0;j<16;j++)
           {
              if (L2[l2_set][j].valid==1 && j!=i) 
                {
                  L2[l2_set][j].lru++;
                }
           }

           empty_flag=1;
           break;
        }
     }  
     if(!empty_flag)  
     {
        invoke_l2_replace(addr, tid);  
     } 
    
}

/*
bool termn_condn()
{
  cout<<q1inp[0].size()<<" "<<q1inp[1].size()<<" "<<q1inp[2].size()<<" "<<q1inp[3].size()<<" "<<q1inp[4].size()<<" "<<q1inp[5].size()<<" "<<q1inp[6].size()<<" "<<q1inp[7].size()<<endl;
  int flag_termn = 0;
    for (int i=0;i<8;i++)
      {
        if (!q1trace[i].empty())
        {
          flag_termn = 1;
          break;
        }
      }

      for (int i=0;i<8;i++){
      if (flag_termn == 0)
      {
        if (!q1inp[i].empty())
        {
          flag_termn = 1;
          break;
        }
      }
    }

      for (int i=0;i<8;i++){
      if (flag_termn == 0)
      {
        if (!q2inp[i].empty())
        {
          flag_termn = 1;
          break;
        }
      }
      }

      if (flag_termn == 1) return true;
      return false;
}
*/
void get_l1_l2(ull addr, int tid)
{

  vector <ull> vec(3);  //0->id of msg, 1->tid, 2->addr
  vec[0] = 0; //since get message
  vec[1] = tid;
  vec[2] = addr;

  int bid = addr%8; 

  q2inp[bid].push(vec);
}

void getx_l1_l2(ull addr, int tid)
{

  vector <ull> vec(3);  //0->id of msg, 1->tid, 2->addr
  vec[0] = 1; //since getx message
  vec[1] = tid;
  vec[2] = addr;

  int bid = addr%8; 

  q2inp[bid].push(vec);

}


void q1inp_processing(int tid)
{
    vector <ull> temp = q1inp[tid].front();
      q1inp[tid].pop();


    int q = temp[1];
    int r = temp[0];
    L1_msg[q][r]++;

      if (temp[0]==2) //put in L1 input trace
      {
          ull addr = temp[2];
          int core_no = temp[1];

          ull l1_set = addr%64;
          ull tag = addr/64;

          int empty_flag = 0;
          for (int i=0;i<8;i++)
          {
            if (L1[core_no][l1_set][i].valid == 0)
            {
              L1[core_no][l1_set][i].valid = 1;
              L1[core_no][l1_set][i].address = tag;
              L1[core_no][l1_set][i].lru = 0;

              int j;
               for (j=0;j<8;j++)
               {
                  if (L1[core_no][l1_set][i].valid == 1 && j!=i)
                    L1[core_no][l1_set][j].lru++;
               }

               empty_flag=1;
               break;
            }
          }

          if (empty_flag == 0)
          {
            invoke_l1_replace(addr, core_no);
          }

          vector<ull> v; 
          v.push_back(addr), v.push_back(tid);
          if (outstanding.find(v) != outstanding.end() && outstanding[v]==0) //Load 
          {
            outstanding.erase(v);
          }

      }

      else if (temp[0] == 3) //putx in L1 input trace
      {
          ull addr = temp[2];
          int core_no = temp[1];

          ull l1_set = addr%64;
          ull tag = addr/64;

          int empty_flag = 0;
          for (int i=0;i<8;i++)
          {
            if (L1[core_no][l1_set][i].valid == 0)
            {
              L1[core_no][l1_set][i].valid = 1;
              L1[core_no][l1_set][i].address = tag;
              L1[core_no][l1_set][i].lru = 0;

              int j;
               for (j=0;j<8;j++)
               {
                  if (L1[core_no][l1_set][i].valid == 1 && j!=i)
                    L1[core_no][l1_set][j].lru++;
               }

               empty_flag=1;
               break;
            }
          }

          if (empty_flag == 0)
          {
            invoke_l1_replace(addr, core_no);
          }

          vector<ull> v; 
          v.push_back(addr), v.push_back(tid);
          if (outstanding.find(v) != outstanding.end()) //Store or Load
          {
            outstanding.erase(v);
          }
      }

      else if (temp[0]==6) //invalidation <6, from_core, addr>
      {
          ull addr = temp[2];
          int core_no = temp[1];

          ull l1_set = addr%64;
          ull tag = addr/64;

          for (int i=0;i<8;i++)
          {
              if(L1[tid][l1_set][i].address == tag && L1[tid][l1_set][i].valid == 1)
              { 
                L1[tid][l1_set][i].valid = 0;

                vector <int> temp;
                for (auto x : dir[addr])
                {
                  if (tid != x) temp.push_back(x);
                }
                if (temp.size()==0) 
                {
                  dir.erase(addr);
                  shared_state.erase(addr);
                }
                else dir[addr] = temp;

                break;
              }

          }

          vector <ull> vec(3);  //0->id of msg, 1->tid, 2->addr
          vec[0] = 7; //since inv-ack message
          vec[1] = core_no;
          vec[2] = addr;

          q1inp[core_no].push(vec);


      }


      else if (temp[0]==7) //inv ack
      {
          ull addr = temp[2];
          int core_no = temp[1];

          ull l1_set = addr%64;
          ull tag = addr/64;



      }


      else if (temp[0]== 9) //xfer  <9, from_core, addr>
      {
          ull addr = temp[2];
          int core_no = temp[1];

          ull l1_set = addr%64;
          ull tag = addr/64;

          vector <ull> vec(3);  //0->id of msg, 1->tid, 2->addr
          vec[0] = 2; //since put message
          vec[1] = core_no;
          vec[2] = addr;

          q1inp[core_no].push(vec);

          //sending swb

          vector <ull> vec1(3);  //0->id of msg, 1->bankid, 2->addr
          vec1[0] = 4; //since swb message
          vec1[1] = core_no; //bankid
          vec1[2] = addr;
          
          int bid = addr%8;
          q2inp[bid].push(vec1);
      }

      else if (temp[0]== 10) //xfer_inv  <9, from_core, addr>
      {
          ull addr = temp[2];
          int core_no = temp[1];

          ull l1_set = addr%64;
          ull tag = addr/64;

          for (int i=0;i<8;i++)
          {
              if(L1[tid][l1_set][i].address == tag && L1[tid][l1_set][i].valid == 1)
              { 
                L1[tid][l1_set][i].valid = 0;

                vector <int> temp;
                for (auto x : dir[addr])
                {
                  if (tid != x) 
                    {
                      temp.push_back(x);
                    }
                    //else cout<<"error here "<<endl;
                }
                if (temp.size()==0) 
                {
                  dir.erase(addr);
                  shared_state.erase(addr);
                }
                else dir[addr] = temp;

                break;
              }

          }

          vector <ull> vec(3);  //0->id of msg, 1->tid, 2->addr
          vec[0] = 3; //since putx message
          vec[1] = core_no;
          vec[2] = addr;

          q1inp[core_no].push(vec);

          //sending swb

          vector <ull> vec1(3);  //0->id of msg, 1->bankid, 2->addr
          vec1[0] = 4; //since swb message
          vec1[1] = core_no; //bankid
          vec1[2] = addr;
          
          int bid = addr%8;
          q2inp[bid].push(vec1);
      }

      else if(temp[0] == 8)  //nack  <8, from_core, addr>
      {
          ull addr = temp[2];
          int core_no = temp[1];

          ull l1_set = addr%64;
          ull tag = addr/64;

          //vector<ull>v; v.push_back(temp[0]); v.push
          //nack_buffer[make_pair(addr, core_no)] = 19; //setting counter < 20

      }

}


void q2inp_processing(int bid)
{
  //cout<<"heee"<<endl;

      vector <ull> temp = q2inp[bid].front();
      q2inp[bid].pop();


      int q = temp[2]%8;
      int r = temp[0];
      L2_msg[q][r]++;

      if (temp[0]==0) //get in L2 input trace
      {
          ull addr = temp[2];
          int core_no = temp[1];

          ull l2_set = addr%4096;
          ull tag = addr/4096;


          if (check_l2(addr) == 1) //present in L2
          {

              if(shared_state.find(addr) == shared_state.end()) //not in dir
              {
                update_l2(addr);
                  shared_state[addr] = 0; //initially setting it as modified
                  vector <int> v;
                  v.push_back(core_no);
                  dir[addr] = v;

                  //sending put

                  vector <ull> vec(3);  //0->id of msg, 1->tid, 2->addr
                  vec[0] = 2; //since put message
                  vec[1] = core_no; //since this is the tid to be sent
                  vec[2] = addr;

                  int q = vec[1];
                  q1inp[q].push(vec);
              }

              else if (shared_state.find(addr) != shared_state.end()) //present in dir
              {
                  if (shared_state[addr]==1) //in shared state
                  {
                    update_l2(addr);
                      vector<int> temp = dir[addr];
                      int present = 0;
                      for (int i=0;i<temp.size();i++)
                      {
                          if (temp[i]==core_no)
                          {
                            present = 1;
                            break;
                          }
                      }

                      if (present == 0) temp.push_back(core_no);

                      dir[addr] = temp; //added sharer

                      //sending put

                      vector <ull> vec(3);  //0->id of msg, 1->tid, 2->addr
                      vec[0] = 2; //since put message
                      vec[1] = core_no; //since this is the tid to be sent
                      vec[2] = addr;

                      int q = vec[1];
                      q1inp[q].push(vec);
                  }

                  else //has owner
                  {
                      vector<int> temp = dir[addr];
                      int owner = temp[0];
                      temp.push_back(core_no);
                      dir[addr] = temp;
                      shared_state[addr] = 1; //putting core_no in shared too

                      //sending xfer

                      vector <ull> vec(3);  //0->id of msg, 1->tid, 2->addr
                      vec[0] = 9; //since xfer message
                      vec[1] = core_no; //since this is the tid to be sent to xfer.
                      //since xfer now has to send block to that core
                      vec[2] = addr;

                      int q = vec[1];
                      q1inp[owner].push(vec);
                  }
              }
          }

          else //not even in L2
          {
              l2_cache_miss[addr%8]++;
              fill_in_L2(addr, core_no);

              shared_state[addr] = 0; //initially setting it as modified
              vector <int> v;
              v.push_back(core_no);
              dir[addr] = v;

              //sending put

              vector <ull> vec(3);  //0->id of msg, 1->tid, 2->addr
              vec[0] = 2; //since put message
              vec[1] = core_no; //since this is the tid to be sent
              vec[2] = addr;

              int q = vec[1];
              q1inp[q].push(vec);
          }
      }


      else if (temp[0]==1) //getx in L2 input trace
      {
          ull addr = temp[2];
          int core_no = temp[1];

          ull l2_set = addr%4096;
          ull tag = addr/4096;

          if (check_l2(addr) == 1) //present in L2
          {

              

              if(shared_state.find(addr) == shared_state.end()) //not in dir
              {
                update_l2(addr);
                  shared_state[addr] = 0; //initially setting it as modified
                  vector <int> v;
                  v.push_back(core_no);
                  dir[addr] = v;

                  //sending putx

                  vector <ull> vec(3);  //0->id of msg, 1->tid, 2->addr
                  vec[0] = 3; //since putx message
                  vec[1] = core_no; //since this is the tid to be sent
                  vec[2] = addr;

                  int q = vec[1];
                  q1inp[q].push(vec);
              }

              else if (shared_state.find(addr) != shared_state.end()) //present in dir
              {
                  if (shared_state[addr]==1) //in shared state
                  {
                    update_l2(addr);
                      vector<int> temp = dir[addr];
                      for (auto x:temp)
                      {
                          //sending inv

                          vector <ull> vec(3);  //0->id of msg, 1->tid, 2->addr
                          vec[0] = 6; //since inv message
                          vec[1] = x; //since this is the tid to be sent
                          vec[2] = addr;

                          q1inp[x].push(vec);
                      }

                      vector<int> v;
                      v.push_back(core_no);
                      dir[addr] = v; 
                      shared_state[addr] = 0; //in modified

                      //sending putx

                      vector <ull> vec(3);  //0->id of msg, 1->tid, 2->addr
                      vec[0] = 3; //since putx message
                      vec[1] = core_no; //since this is the tid to be sent
                      vec[2] = addr;

                      int q = vec[1];
                      q1inp[q].push(vec);
                  }

                  else //has owner
                  {
                      vector<int> temp = dir[addr];
                      int owner = temp[0];

                      vector<int>v;
                      v.push_back(core_no);
                      dir[addr] = v;
                      shared_state[addr] = 0; //core_no put in modified

                      //sending xfer_inv

                      vector <ull> vec(3);  //0->id of msg, 1->tid, 2->addr
                      vec[0] = 10; //since xfer_inv message
                      vec[1] = core_no; //since this is the tid to be sent to xfer.
                      //since xfer_inv now has to send block to that core
                      vec[2] = addr;

                      q1inp[owner].push(vec);
                  }
              }
          }

          else //not even in L2
          {
            l2_cache_miss[addr%8]++;

              fill_in_L2(addr, core_no);

              shared_state[addr] = 0; //initially setting it as modified
              vector <int> v;
              v.push_back(core_no);
              dir[addr] = v;

              //sending putx

              vector <ull> vec(3);  //0->id of msg, 1->tid, 2->addr
              vec[0] = 3; //since putx message
              vec[1] = core_no; //since this is the tid to be sent
              vec[2] = addr;

              int q = vec[1];
              q1inp[q].push(vec);
          }
      }

      else if (temp[0] == 5) //upgrade processing in L2
      {
          ull addr = temp[2];
          int core_no = temp[1];

          ull l2_set = addr%4096;
          ull tag = addr/4096;

          if (shared_state.find(addr) != shared_state.end() && shared_state[addr] == 1) //in shared state
          {
              vector<int> temp = dir[addr];
              for (auto x:temp)
              {
                  //sending inv

                  vector <ull> vec(3);  //0->id of msg, 1->tid, 2->addr
                  vec[0] = 6; //since inv message
                  vec[1] = x; //since this is the tid to be sent
                  vec[2] = addr;

                  q1inp[x].push(vec);
              }

              shared_state[addr] = 0; //initially setting it as modified
              vector <int> v;
              v.push_back(core_no);
              dir[addr] = v;

               //sending putx

              vector <ull> vec(3);  //0->id of msg, 1->tid, 2->addr
              vec[0] = 3; //since putx message
              vec[1] = core_no; //since this is the tid to be sent
              vec[2] = addr;

              int q = vec[1];
              q1inp[q].push(vec);

          }

          else if (shared_state.find(addr) != shared_state.end() && shared_state[addr] == 0) //in modified state some block
          {
              vector<int> temp = dir[addr];
              int owner = temp[0];

              vector<int>v;
              v.push_back(core_no);
              dir[addr] = v;
              shared_state[addr] = 0; //core_no put in modified

              //sending xfer_inv

              vector <ull> vec(3);  //0->id of msg, 1->tid, 2->addr
              vec[0] = 10; //since xfer_inv message
              vec[1] = core_no; //since this is the tid to be sent to xfer.
              //since xfer_inv now has to send block to that core
              vec[2] = addr;

              q1inp[owner].push(vec);
          }
      }

      else if (temp[0] == 4) //swb sent to L2
      {
          ull addr = temp[2];
          int core_no = temp[1];

          ull l2_set = addr%4096;
          ull tag = addr/4096;
      }

      
      else if (temp[0] == 11)  //repl_s sent to L2
      {
          ull addr = temp[2];
          int core_no = temp[1];

          ull l2_set = addr%4096;
          ull tag = addr/4096;

      }

}

void q1trace_processing(int tid)
{
    ts ele = q1trace[tid].front();
    q1trace[tid].pop();

    l1_cache_acc[tid]++;


    
    if (ele.mode == 0)  ///read request
    {
       ///checking for hit 
      if (hit_l1(ele.address, tid) == 1) 
        {
          update_l1(ele.address, tid);
        }
      else 
        {

          vector<ull> v; 
          v.push_back(ele.address), v.push_back(tid);

          if (outstanding.find(v) != outstanding.end())
          {
            //increment hit counter

            l1_cache_hit[tid]++;
          }

          else 
          {

            l1_cache_miss[tid]++;

            outstanding[v] = 0; //Load state

            if (shared_state.find(ele.address)!=shared_state.end())
            get_l1_l2(ele.address, tid);

          else getx_l1_l2(ele.address, tid);

          }
          
        }
    }
    else //write request
    {
        
        if (hit_l1(ele.address, tid) == 1) 
        {
          updatex_l1(ele.address, tid);
        }
        else 
        {

          vector<ull> v; 
          v.push_back(ele.address), v.push_back(tid);
          if (outstanding.find(v) != outstanding.end() && outstanding[v] == 1) //Store state
          {
            //increment hit counter
            l1_cache_hit[tid]++;
          }

          else if (outstanding.find(v) != outstanding.end() && outstanding[v] == 0) //Load state
          {
            //increment hit counter

            l1_cache_upgr_miss[tid]++;

            vector <ull> vec(3);  //0->id of msg, 1->tid, 2->addr
            vec[0] = 5; //since upgrade message
            vec[1] = tid;
            vec[2] = ele.address;

            int bid = ele.address%8;
            q2inp[bid].push(vec);

            outstanding[v] = 1;

          }


          else 
          {

            l1_cache_miss[tid]++;
            outstanding[v] = 1; //Store
            getx_l1_l2(ele.address, tid);
            
            /*
            if (shared_state.find(ele.address) != shared_state.end() && shared_state[ele.address] == 0)
            {
              if (dir.find(ele.address) != dir.end() && dir[ele.address][0] == tid)  //block present in modified state 
              {
                //cout<<ele.address;
                //cout<<" kuch bhi "<<endl;
                err++;
              }
            }*/

          }
          
        }

    }
}


void core (int tid)
{
  //cout<<global_cnt<<endl;
    if (!q1trace[tid].empty())
    {
        if (global_cnt == q1trace[tid].front().count) q1trace_processing(tid);
    }

  //possible messages -> put, putx, inv, inv_ack, xfer, xfer_inv, nack
  if (!q1inp[tid].empty()) q1inp_processing(tid);
  //possible messages -> get, getx, upgrade, swb, repl_s
  if (!q2inp[tid].empty()) q2inp_processing(tid);

    
}

void print_data()
{

  long long int L2sum=0;
  for(int i=0;i<8;i++)
   L2sum+= l2_cache_miss[i];

  cout<<"Number of simiulated cycles :"<<cycles<<endl;
  cout<<"Total machine accesses :"<<global_cnt-1<<endl;

  cout <<"L1 cache accesses (per core) ::";
   for (int i=0;i<8;i++)
   {
    cout<<"("<<i<<") : "<<l1_cache_acc[i];
    if (i==7) cout<<endl;
    else cout<<", ";
   }
   cout<<endl;
   cout <<"L1 cache hits (per core) ::";
   for (int i=0;i<8;i++)
   {
    cout<<"("<<i<<") : "<<l1_cache_hit[i];
    if (i==7) cout<<endl;
    else cout<<", ";
   }
   cout<<endl;
   cout <<"L1 cache misses (per core) ::";
   for (int i=0;i<8;i++)
   {
    cout<<"("<<i<<") : "<<l1_cache_miss[i];
    if (i==7) cout<<endl;
    else cout<<", ";
   }
   cout<<endl;
   cout <<"L1 cache upgrade misses (per core) ::";
   for (int i=0;i<8;i++)
   {
    cout<<"("<<i<<") : "<<l1_cache_upgr_miss[i];
    if (i==7) cout<<endl;
    else cout<<", ";
   }
   cout<<endl;
   cout <<"L2 cache misses (per bank) ::";
   for (int i=0;i<8;i++)
   {
    cout<<"("<<i<<") : "<<l2_cache_miss[i];
    if (i==7) cout<<endl;
    else cout<<", ";
   }

   cout<<endl<<endl;

   cout<<"L1 messages (in order) ::"<<endl;
   cout<< "get | getx | put | putx | swb | upgrade | inv | inv_ack | nack | xfer | xfer_inv | repl_s"<<endl;
   for (int i=0;i<8;i++)
   {
    for (int j=0;j<12;j++)
    {
      cout<<L1_msg[i][j]<<"\t";
    }
    cout<<endl;
   }

   cout<<"L2 messages (in order) :: "<<endl;
   cout<< "get | getx | put | putx | swb | upgrade | inv | inv_ack | nack | xfer | xfer_inv | repl_s"<<endl;
   for (int i=0;i<8;i++)
   {
    for (int j=0;j<12;j++)
    {
      cout<<L2_msg[i][j]<<"\t";
    }
    cout<<endl;
   }

   //cout<<err<<endl;

}

int main(int argc, char** argv)
{
  int number;
  printf("Enter number of prog file on which you are running the program: ");
  scanf("%d",&number);
  string s = "mach_acc_"+ to_string(number) + ".txt";
   ifstream file (s);

   string str;

   while (getline(file, str))
   {
      ull count, addr;
      int tid, size, mode; //mode 0 for read, 1 for Write
      int i = 0;
      string s = "";
      while (str[i]!=' ')
      {
        s+= str[i];
        i++;
      }
      count = stoull (s);

      tid = str[++i] - '0';
      i+= 2;

      s = "";
      while (str[i]!=' ')
      {
        s+= str[i];
        i++;
      }
      addr = stoull (s) >>6;  /// finding out the block addr (ALL ADDR HENCEFORTH IS THIS)
      //addr = stoull(s);
      size = str[++i] - '0';
      i+=2;

      mode = str[i] - '0'; 

      ts x;
      x.count = count;
      x.address = addr;
      x.tid = tid;
      x.mode = mode;

      q1trace[tid].push(x);
      

      //cout<<count<<" "<<addr<<" "<<tid<<" "<<size<<" "<<mode<<endl;

   }

   int termn_cond = 0;
   while(termn_cond<8){
    int arr[8]={0};
    ts temp [8];
    int entries=0;
    for(ull i=0;i<8;i++){
      if(!q1trace[i].empty())
          {
            temp[entries] = q1trace[i].front();
            entries++;
          }

      }

     qsort(temp,entries,sizeof(ts),compare_func);


     for(int i =0 ;i < entries;i++)
      {
          if(global_cnt==temp[i].count )
            {
              core(temp[i].tid);
              global_cnt++;
              arr[temp[i].tid]=1;
            }
          else
            break;


      }

      for(int i=0;i<8;i++)
       {
          if(arr[i]!=1)
           core(i);

       }

    cycles++;
    termn_cond =0;
    for(int i =0;i<8;i++){
      if(q1trace[i].empty() && q1inp[i].empty() && q2inp[i].empty()){
        termn_cond++;
      }
    }
  }

   //cout<<"heeeee"<<endl;

   print_data();


   return 0;
}
