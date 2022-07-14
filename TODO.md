# TODO
- [ ] crossover
- [ ] mutation
- [ ] selection
- [ ] design meta programmer
- [ ] add summary to report class
- [ ] reformat and save output
- [ ] see which mathematic operation is done most
- [ ] profile code
    - [ ] use perf command
    - [ ] investigate clock cycles



# SPEED TEST
- compiler speed
    - clang compiler
    - GNU compiler (g++)
- population storage
    - scope
        - global/static
        - local
    - register storage
    - thread local storage
        - using thread_local, '__thread__', __delspec(thread) keywords
        - accessing from single population array
        - creating thread arrays
    - other
        - dynamic
        - static
        - const
- population abstraction
    - multidimension array of integers
    - multidimension array of structs
    - multidimension array of node objects
    - array of individual objects
    - priorty queue
- encoding (last)
    - single permuation for all operations
    - one permutation for 2 and 1 operations
    - different permuation for each operation
- sorting
    - exact fitness
        - by pointers
        - by whole object
    - estimated fitness
        - by pointers
        - by whole objects



# QUALITY TEST
- crossover
    - resolution
    - operators
        - tournament
        - roulette
    - selection
        - random
        - fitess
        - both random and fitess
- mutation
    - resolution
    - operators
      - inverse
      - scrambled
- replacement
    - inplace of population array
    - seperate children and parent array
- two opt frequency
- migration frequenecy
- fitness metrics
    - euclidean
    - squared distances



# REMARKS
- store functions frequently used together, together
- store variables frequently used together, together
- use vector operations
- keep in mind alignment of data
- caching
- lookup tables
- varible storage