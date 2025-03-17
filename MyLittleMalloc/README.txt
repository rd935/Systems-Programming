Ritwika Das - rd935

Performance Test:
- Task 4
    - tests malloc and free with a matrix
    - so far we have only tested malloc and free with arrays,
      so I figured it would be a good idea to also test it with a matrix
    - in this tast speecifically, the matrix is made as ptrs and then it 
      goes through a nested for loop and allocates and frees each elements 
      as it goes through the matrix

- Task 5
    - task 5 is very similar to task 4, however the only difference is that 
      instead of immediately freeing each element of the matrix right after 
      allocating them, the whole matrix is being allocated and then the whole 
      matrix is freed with another nested for loop

Plan/Strategy
- first my idea was to create a function that sets up a block of a certain 
  size and allocate it
- next I figure it would be necessary to create a function which can check 
  to see if a block of memory is free or not, which is why I made the isFree()
- there needed to be a function which merged blocks of memory based on what 
  mymalloc was called for, so that is the function i created next
- and the last important function i utilized before working on mymalloc() and 
  myfree was a function made to clear any memory that had information in it
- once i had those basic functions set, it made creating mymalloc() and myfree() 
  much easier, because i simply had to set up the conditions for when memory would 
  be stored and when it would be freed  