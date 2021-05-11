any restrictions (applied for our simulator), which are mentioned for the before phases applies here also.

input: (in the console)
   we should enter the file name (should be from the same folder)
   enter 1 to allow forwarding, else 0
   enter l1 cache size , l2 cache size , l1 block size , l2 block size , l1 associativituy , l2 associativity , l1 , l2 and memory access times
   There is a cout statement while asking input for each of these
   Enter the input as per the requested, by the cout statements in the console.
   
   all the values related to the cache must be in sync (i.e, when combined , it must make sense)
   if we give some random values, then program may terminate in the middle or program might not run as expected.
   
   block size must be in power's of 2
   size values must be in bytes
   make sure that the no. of sets will be in power's of 2. (set the block size , cache size and associativity s/t no. of sets must be a power of 2)
   latency values (in sec's)
   associativity must be a number (make sure to enter a number s/t everything goes in sync)
   
   we are using the lru cache replacement policy with 2 levels of cache
   we are using the non-inclusive strategy for maintaining the multi level cache
   
   when an address is not found in both the levels of cache, then we go to the memory and pick required no of bytes and insert them in both levels of the cache
   when a block is removed from the l1 cache, then we don't remove it from the l2 cache
   when we search for an address in the l1 cache,
        if it's a miss
            then we check in the l2 cache
            if it's a hit in the l2 level , we bring it back to the l1 cache.
   l1 and l2 caches use lru replacement policy , independently
   
   program runs good when l1 cache block size <= l2 cache block size
   
   when l1 cache block size > l2 cache block size
        then , if something is a miss in l1 cache , then it searches in the l2 cache 
              if it's a hit in l2 cache , then it inserts this block in the l1 cache at an expense of (l1_cache latency + l2_cache latency)
              
              the only catch here is that , l2 cache has some number of blocks required by the program, but it doesn't have all the bytes required to fill the l1 cache block.
              so, to keep it simple , I am assuming that, at an expense of (l1_cache latency + l2_cache latency), we are inserting a new block in the l1 cache which we found in 
              the l2 cache (though, we donot have all the bytes to fill the l1 cache block, we are assuming that we can get all the required bytes and fill the l1_cache)
              
        if it's found in l1 cache ,there is no problem
        if it's not found in either of the caches , then also , there won't be any problem.
   
Output:

      we printed 1 , if it's a hit in l1_cache , 2 , if it's a hit in the l2_cache , 3 , if we go to the memory 
      these numbers are seperated by a " # "
      
      number of stalls , cycles , instructions are printed in the console
      IPC (instructions per cycle) is printed
      
      l1 cache miss rate and l2 cache miss rate is printed
      
           l1 cache miss rate = (l1_misses)/(l1_accesses)
           l2 cache miss rate = (l2_misses)/(l2_accesses)
           
      all the 32 registers and all the memory is also printed at the end
      
      
   

