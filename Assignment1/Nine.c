#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

typedef struct{
   unsigned long long int address;
   long int lru;
   int valid;
} cache;

cache L2[1024][8],L3[2048][16];

void invalid_l2(unsigned long long int block_addr)
{
   int i,l2_set=block_addr%1024;
   for(i=0;i<8;i++)
    {
      if (L2[l2_set][i].address == block_addr && L2[l2_set][i].valid == 1) 
      {
         L2[l2_set][i].valid=0;
         break;
      }
   }
}

//Filled set, remove some block and place tag2
void invoke_l2_replace(unsigned long long int block_addr)
{
   int i,ind; //ind is the index which we want to replace
   int maxm = -1,l2_set=block_addr%1024;
   unsigned long long int tag2=block_addr/1024;
   for (i=0;i<8;i++)
   {
      if (L2[l2_set][i].lru>maxm)
      {
         maxm = L2[l2_set][i].lru;
         ind = i;
      }
   }
   L2[l2_set][ind].address = tag2;
   L2[l2_set][ind].lru = 0;

   for(i=0;i<8;i++)
    {
      if (i!=ind) L2[l2_set][i].lru++;
    }
}

//Filled set, remove some block and place tag3
void invoke_l3_replace(unsigned long long int block_addr)
{
   int i,ind,maxm = -1;
   unsigned long long int evict_block,tag3=block_addr/2048;
   int l3_set=block_addr%2048;
   for (i=0;i<16;i++)
   {
      if (L3[l3_set][i].lru>maxm)
      {
         maxm = L3[l3_set][i].lru;
         ind = i;
      }
   }
   evict_block= L3[l3_set][ind].address;
   L3[l3_set][ind].address = tag3;
   L3[l3_set][ind].lru = 0;

   for(i=0;i<16;i++)
    {
      if (i!=ind) L3[l3_set][i].lru++;
    }

}



void hit_l3(unsigned long long int block_addr)
{
   int i,empty_flag=0;
  unsigned long long int tag2=block_addr/1024;
   int l2_set=block_addr%1024;
   for(i=0;i<8;i++)
   {      
      if(L2[l2_set][i].valid==0)
      {  
         L2[l2_set][i].address=tag2;
         L2[l2_set][i].lru=0;
         L2[l2_set][i].valid=1;

         int j;
         for (j=0;j<8;j++)
         {
            if (L2[l2_set][j].valid==1 && j!=i) L2[l2_set][j].lru++;
         }

         empty_flag=1;
         break;
      }
   }   
   if(!empty_flag)  
   {
      invoke_l2_replace(block_addr);  
   } 
}

void miss_l3(unsigned long long int block_addr)
{

   int i,empty_flag=0,l2_set,l3_set;
    unsigned long long tag2=block_addr/1024,tag3=block_addr/2048;
  
  l2_set=block_addr%1024;
  l3_set=block_addr%2048;
  
   for(i=0;i<16;i++)
   {
      if(L3[l3_set][i].valid==0)
      { 

         L3[l3_set][i].address=tag3;
         L3[l3_set][i].lru=0;
         L3[l3_set][i].valid=1;

         int j;
         for (j=0;j<16;j++)
         {
            if (L3[l3_set][j].valid==1 && j!=i) L3[l3_set][j].lru++;
         }

         empty_flag=1;
         break;
      }
   }  
   if(!empty_flag)  
   {
      invoke_l3_replace(block_addr);  
   } 

   
   empty_flag=0; 
   for(i=0;i<8;i++)
   {
      if(L2[l2_set][i].valid==0)
      {  
         L2[l2_set][i].address=tag2;
         L2[l2_set][i].lru=0;
         L2[l2_set][i].valid=1;

         int j;
         for (j=0;j<8;j++)
         {
            if (L2[l2_set][j].valid==1 && j!=i) L2[l2_set][j].lru++;
         }

         empty_flag=1;
         break;
      }
   }  
   if(!empty_flag)  
   {
      invoke_l2_replace(block_addr);  
   } 

}

void update_l2(unsigned long long int block_addr)
{
   int i,l2_set=block_addr%1024;
   unsigned long long int tag2=block_addr/1024;
   for (i=0;i<8;i++)
   {
      if (L2[l2_set][i].address == tag2) L2[l2_set][i].lru = 0;
      else if (L2[l2_set][i].address != tag2 && L2[l2_set][i].valid == 1)L2[l2_set][i].lru++;
   }
}

void update_l3(unsigned long long int block_addr)
{
   int i,l3_set=block_addr%2048;
   unsigned long long int tag3=block_addr/2048;
   for (i=0;i<16;i++)
   {
      if (L3[l3_set][i].address == tag3) L3[l3_set][i].lru = 0;
      else if (L3[l3_set][i].address != tag3 && L3[l3_set][i].valid==1) L3[l3_set][i].lru++;
   }
}


int main(int argc, char** argv)
{
   char iord;
   char type;
   unsigned long long int addr,block_addr;
   unsigned long long int tag2,tag3;
   unsigned pc;
   long l2_miss=0,l3_miss=0;
   int k,l2_row,l3_row,l2_col,l3_col,l2_set,l3_set,i,j;
   char input_name[30];
   FILE *fp;
   int numtraces = atoi(argv[2]);
   int count = 0;

   for(l2_row=0;l2_row<1024;l2_row++)
   {
      for(l2_col=0;l2_col<8;l2_col++)
      {
         L2[l2_row][l2_col].valid=0;
      }
   }

   for(l3_row=0;l3_row<2048;l3_row++)
   {
      for(l3_col=0;l3_col<16;l3_col++)
      {
         L3[l3_row][l3_col].valid=0;
      }
   }

   for (k=0; k<numtraces; k++) 
   {      
      sprintf(input_name, "%s_%d", argv[1], k);
      fp = fopen(input_name, "rb");
      assert(fp != NULL);
      while (!feof(fp)) 
      { 
         int l2_flag=0,l3_flag=0;

         fread(&iord, sizeof(char), 1, fp); 
         fread(&type, sizeof(char), 1, fp);
         fread(&addr, sizeof(unsigned long long int), 1, fp);
         fread(&pc, sizeof(unsigned), 1, fp);
        
         if (type+'0'=='1')
         {
            count++;
            block_addr=addr/64;
            l2_set=block_addr%1024;
            tag2=block_addr/1024;
             
             for(i=0;i<8;i++)
             {     if(L2[l2_set][i].address == tag2 && L2[l2_set][i].valid == 1)
                 {
                    //hits in l2
                    l2_flag=1;
                    update_l2(block_addr);
                    break;
                 }

             }
               //Block misses in l2,check for l3 cache
             if(l2_flag == 0)
             {  
               l2_miss++;
                l3_set=block_addr%2048;
                tag3=block_addr/2048;


               for(i=0;i<16;i++)
               {    
                   if(L3[l3_set][i].address ==tag3 && L3[l3_set][i].valid == 1)
                  {
                  //hits in l3
                      l3_flag=1;
                      update_l3(block_addr);
                      hit_l3(block_addr);
                  } 
               }


               //block misses in L3
               if(l3_flag == 0)
                {  
                  l3_miss++;
                  miss_l3(block_addr);
                }

               }           
          }
      } 
      
      fclose(fp);
   }

   printf("Number of L1 misses: %d\nNumber of L2 misses: %ld\nNumber of L3 misses: %ld \n", count, l2_miss, l3_miss);

   return 0;
}
