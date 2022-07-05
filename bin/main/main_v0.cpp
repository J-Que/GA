#include <mutex>
#include <math.h>
#include <thread>
#include <chrono>
#include <random>
#include <vector>
#include <string>
#include <iomanip>
#include <fstream>
#include <iostream>
#include <algorithm>
using namespace std;
auto start = chrono::high_resolution_clock::now();

// parameters
const int C = 20;                     // population mulitplier
const float U = 0.05;                 // percent of nodes to be mutated in an individual
const int BIT_SIZE = 32;              // number of bits to assign each encoded node
const int DEMAND_BITS = 8;            // number of bits per encoded node that is assigned to the demand
const int MAX_GENERATIONS = 20000;    // maximum number of generations
const float MUTATION_RATE = 0.30;     // rate of population that is mutated
const float CROSSOVER_RATE = 0.60;    // rate of population that will produce offspring for each generation
const int BRIEFING_INTERVAL = 10000;  // how often to report results from generation
const int SUMMARY_INTERVAL = 500;     // how often to result report

// CVRP attributes
vector<int> locations;  // gausian coordinates of nodes
vector<int> demand;     // positive real demand of nodes
vector<int> nodes;      // encoded nodes
char* PROBLEM;          // name of problem
bool optimality;        // is the problem optimal
float optimal = 0;      // optimal cost if there is one
float best;             // best known cost
float score;            // the score for that generation
int capacity;           // capacity of vehicle
int trucks;             // get the number of routes to service the problem
int depot;              // value of encoded depot
int N;                  // number of nodes

// GA attributes
vector<vector<int>> population;	    // holds the population
vector<vector<int>> offspring;      // holds the resulting offsprings after crossover 
vector<vector<int>> rankings;       // ranks the best performing offspring and parents 
int M;                              // number of inidividuals in population
int S;                              // number of individuals in offspring
int gen = 0;			            // track the generations
string CVRP;                        // the name of the problem
int y = 150; 			            // border length of report printing

// Multithreading Attributes
vector<thread> threads;                                 // vector holding the threads
mutex threadlock;                                       // mutex object to lock a thread when performing certain writing operations
const int MAX_THREADS = thread::hardware_concurrency(); // number of threads available
int Q;                                                  // number of individuals to be processed on each thread

// utility read functions
int getValue(string line, int pos) {return stoi(line.substr(pos, line.find(",") - pos));}
void getLocations(string line) {if (line[8] != '[' && line[8] != ']') {locations.push_back(getValue(line, 12));}}

// read in a problem
void read() {
    ifstream file ("../../data/" + string(PROBLEM));
    string line;
    bool locationLines = false;
    bool demandLines = false;

    // open file
    if (file.is_open()) {

        // go through each line
        while (file) {

            // output the content onto line
            getline (file, line);

                 if (line[5] == 'N') {N = getValue(line, 13);}           // get the number of nodes
            else if (line[5] == 'R') {trucks = getValue(line, 14);}      // get the number of routes
            else if (line[5] == 'C') {capacity = getValue(line, 16);}    // get the truck capacity
            else if (line[5] == 'B') {best = getValue(line, 12);}        // get the best known score
            else if (line[5] == 'O') {optimality = getValue(line, 15);}  // determine if the best known cost is optimal
            else if (line[5] == 'L') {locationLines = true;}             // begin adding the locations
            else if (line[5] == 'D') {demandLines = true;}               // begin adding the demand
            else if (line[4] == ']') {locationLines = false;}            // stop adding the location lines to start adding the demand instead
            else if (locationLines) {getLocations(line);}                // add the node coordinates to the location vector
            else if (demandLines) {demand.push_back(getValue(line, 8));} // add the demand to the vector
        }
    }
}

// print a summary of the problem
void brief() {
    cout << "\n\n   ";
    for (int i=0; i<y; i++) {cout << "_";}

    cout << "\n\n   CVRP: " << CVRP << "\n   ";
    for (int i=0; i<y; i++) {cout << "-";}
    cout << "\n   Nodes: " << setw(25) << N << " Vehicles: " << setw(22) << trucks << " Capacity: " << setw(22) << capacity << " Best Known Cost: " << setw(22) << best*10 << " Optimal: " << optimality*1000;

    cout << "\n\n   Genetic Algorithm with Two-Opt Optimization\n   ";
    for (int i=0; i<y; i++) {cout << "-";}
    cout << "\n   Crossover: Two-Point             Mutation: Inverse                Selection: Steady-State";
    cout << "\n   Crossover Rate: " << setw(16) << CROSSOVER_RATE << " Mutation Rate: " << setw(17) << MUTATION_RATE << " Mutated Portion: " << U;
    cout << "\n   Parent Size: " << setw(19) << M << " Max Generations: " << setw(15) << MAX_GENERATIONS << " Threads: " << setw(23) << MAX_THREADS << " Individuals per Thread: " << Q;

    cout << "\n\n   " << setw(33) << "Generation" << setw(33) << "Cost" << setw(33) << "Gap" << setw(33) << "Secs/Loop" << "Time\n   ";
    for (int i=0; i<y; i++) {cout << "-";}
}

// print the results after each few generations
void summary() {
    auto t = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count()/1000.0;
    cout << "\n    " <<setw(32)<< gen <<setw(33)<< score <<setw(33)<< (score - best*10)/(best*10) <<setw(33)<< t/gen << t;
}

// print a summary of the results
void report(bool printGenes) {
    // print results
    auto t = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count()/1000.0;
    int solution;
    for (int m = 0; m < M; m++) {if (population[m][0] == score) {solution = m;}}

    cout << "\n\n\n\n   Cost: " << score;
    cout << "\n   Gap: " << (score - (best*10))/(best*10);
    cout << "\n   Generations: " << gen - 1;
    cout << "\n   Total Execution Time: " << t;
    cout << "\n   Seconds per Loop: " << t/(gen - 1) << "\n   ";

    // print genes
    if (printGenes) {
        cout << "\n   Genes:\n   ";
        for (int i=0; i<y; i++) {cout << "-";}
        cout << "\n   ";
        for (int n = 2; n < 2*N+1; n++) {
            cout << setw(15) << population[solution][n] << " ";
            if (n % 8 == 1) {cout << "\n   ";}
        }
    }

    cout << "\n   ";
    for (int i=0; i<y; i++) {cout << "_";}
    cout << "\n\n\n\n\n\n";
}

// save the results
void save(bool saveGenes) {
    ofstream results ("../../results/" + CVRP + "_report.json");
    auto t = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - start).count()/1000.0;
    int solution;
    for (int m = 0; m < M; m++) {if (population[m][0] == score) {solution = m;}}

    // save report
    results << "{\n    \"problem\": \"" << CVRP << "\",";
    results << "\n    \"cost\": " << score << ",";
    results << "\n    \"best known\": " << best << ",";
    results << "\n    \"gap\": " << (score - best)/best << ",";
    results << "\n    \"time in seconds\": " << t << ",";
    results << "\n    \"seconds per loop\": " << t/(gen - 1) << ",";
    results << "\n    \"nodes\": " << N << ",";
    results << "\n    \"vehicles\": " << trucks << ",";
    results << "\n    \"capacity\": " << capacity << ",";
    results << "\n    \"crossover rate\": " << CROSSOVER_RATE << ",";
    results << "\n    \"mutation rate\": " << MUTATION_RATE << ",";
    results << "\n    \"mutation portion\": " << U << ",";
    results << "\n    \"population size\": " << M  << ",";
    results << "\n    \"generations\": " << gen - 1 << ",";
    results << "\n    \"threads\": " << MAX_THREADS << ",";
    results << "\n    \"genes: [\n        ";
    for (int n = 2; n < 2*N+1; n++) {
        results << population[solution][n];
        if (n < 2*N) {results << ", ";}
        else {results << "\n    ]\n}";}
        if (n % 20 == 1) {results << "\n        ";}
    }
    results.close();

    // save the genes
    if (saveGenes) {
        ofstream genes ("../results/" + CVRP + "_genes.json");
        genes << "{\n    ";
        for (int m = 0; m < M ; m++) {
            genes << m << ": [\n";
            genes << "        \"distance\": " << population[m][0] << ",\n";
            genes << "        \"generation\": " << population[m][1] << ",\n";
            genes << "        \"genes\": [\n            ";
            for (int n = 2; n < 2*N+1; n++) {
                genes << population[m][n];
                if (n < 2*N) {genes << ", ";}
                else {genes << "\n        ]\n    ]";}
                if (n % 20 == 0) {genes << "\n            ";}
            }
            if (m < M-1) {genes << ",\n    ";}
        }
        genes << "\n}";
        genes.close();
    }

}

// encode x, y and demand into a single unique ID
int encode(int x, int y, int demand) {
    int bitShift = pow(2, (BIT_SIZE - DEMAND_BITS)/2);      // bits to shift
    int location = x * bitShift + y;                        // pair the x and y
    return demand * bitShift * bitShift + location;         // return the unique ID for the node by pairing the x, y and demand
}

// get the x, y, or demand from the unique ID
vector<int> decode(int ID) {
    int bitShift = pow(2, (BIT_SIZE - DEMAND_BITS)/2);
    int x =  (ID % (bitShift * bitShift)) / bitShift;
    int y = ID % bitShift;
    int demand = ID/(bitShift * bitShift);
    return vector<int> {x, y, demand};
}

// initialize addiditional parameters
void initialize() {
    // seed for randomization
    srand(time(0));

    // get the name of the problem;
    string s(PROBLEM);
    string t(PROBLEM, PROBLEM + s.length() - 5);
    CVRP = t;

    // normalize the coordinates
    vector<int> x, y;

    for (int n = 0; n < N; n++) {
	x.push_back(locations[2*n]);
	y.push_back(locations[2*n + 1]);
    }

    int minX = *min_element(x.begin(), x.end());
    int minY = *min_element(y.begin(), y.end());

    if (minX < 0) {for (int n = 0; n < N; n+=2) {locations[n] += minX;}}
    if (minY < 0) {for (int n = 1; n < N; n+=2) {locations[n] += minY;}}

    // get the optimal value if present
    if (optimality == true) {optimal = best;}

    // get the number of individuals to be proccessed on each thread
    Q = (1 + (N*C - 1) / MAX_THREADS);

    // set the size of the population
    if (Q % 2 == 0) {M = Q*MAX_THREADS;} else {M = (Q - 1) * MAX_THREADS; Q -= 1;}

    // format the output of the results
    cout << left;
} 

// shuffle a vector
void shuffle(vector<int> &vect , int start, int end) {
    for (int i = start; i < end; i++) {     // for each node
        int r = 4 + (rand() % (end - 5));   // get the random position
        swap(vect[i], vect[r]);             // swap the current node with a random node further along
    }
}

// initialize a random phenotype: for each solution in the population, add a randomized array of the nodes and zeros
vector<int> init_population(vector<int> &nodes) {
    vector<int> pheno;
    pheno.assign(N + 1, depot);                                             // fill the phenotype with the depot's ID
    for (int n = 0; n < N; n++) {pheno.insert(pheno.end() - 5, nodes[n]);}  // add in the rest of the nodes to the phenos (1 place over from the end of the vector)
    shuffle(pheno, 3, pheno.size() - 1);                                    // shuffle the phenotype
    pheno[1] = 0;                                                           // assign the generation to the pheno
    return pheno;
}

// get the cost of the solution
float cost(vector<int> &pheno, int start, int end) {
    float distance = 0;
    vector<int> prev, curr = decode(pheno[start - 1]);

    // iterate through the nodes
    for (int node = start; node <= end; node++) {

        // get the location of the previous and current node
        prev = curr;
        curr = decode(pheno[node]);

        // get the euclidean distance from the previous node and current node and add it to the total
        distance += pow(prev[0]*10 - curr[0]*10, 2) + pow(prev[1]*10 - curr[1]*10, 2);

    }
    return distance;
}

// establish the routes based of capacity (fit as many nodes into a route)
void routes(vector<int> &pheno) {
    int bitShift = pow(2, BIT_SIZE - DEMAND_BITS);   // the bits to shift the ID over to get the demand
    int load = 0;                                   // the demand of a route

    // for each gene in the pheno (skipping the first two for fitness and generation, and an additional one to establish the first route and the last one to finish the last route)
    for (int gene = 3; gene < pheno.size() - 1; gene++) {
        if (pheno[gene] == depot) {                              // if the current gene is a depot
            pheno.erase(pheno.begin() + gene);                   // then it is popped out of the vector
            gene -= 1;                                           // the iterator is decreased one to not skip the next element
        } else {                                                 // else the current gene is not a depot
            int demand = pheno[gene]/bitShift;                   // demand of current node is found
            if (load + demand <= capacity) {load += demand;}     // demand is added to load if capacity is not violated
            else {                                               // else adding demand violates the capacity
                pheno.insert(pheno.begin() + gene, depot);       // a depot is inserted to complete a route
                load = 0;                                        // the load is reset
            }
        }
    }

    // add depots to pad the pheno
    for (int i = pheno.size(); i < 2*N + 1; i++) {pheno.push_back(depot);}

}

// conduct two opt optimization
void two_opt(vector<int> &pheno) {
    for (int gene = 3; gene < pheno.size() - 1;) {  // iterate through the genes
        if (pheno[gene] == depot) {gene++;}        // if the current gene is a depot, then pass over it
        else {                                     // else the current gene is not a depot

            // find the number of nodes in the route
            int nodesInRoute = 1;
            while (pheno[gene + nodesInRoute] != depot && gene + nodesInRoute != pheno.size()) {nodesInRoute += 1;}

            // declare the start and end of the route
            int start = gene, end = gene + nodesInRoute;

            // two opt: continue the loop until no improvement is made
            bool improved = true;
            while (improved) {

                // set the imrpovement flag to false
                improved = false;

                // find the distance of the route
                float best = cost(pheno, start, end);

                // iterate through each possible swap
                REPEAT:
                for (int i = start; i < end - 1; i++) {
                    for (int j = i + 1; j < end; j++) {

                        // perform the swap and get the distance of the new route
                        iter_swap(pheno.begin() + i, pheno.begin() + j);
                        float distance = cost(pheno, start, end);

                        // if the new distance improves then the new best distance is recorded
                        if (distance < best) {
                            best = distance;
                            improved = true;
                            goto REPEAT;

                        // else the swap is undone
                        } else {iter_swap(pheno.begin() + j, pheno.begin() + i);}
                    }
                }
            }

            // after the two-opt, pass over the route
            gene += nodesInRoute + 1;
        }
    }
}

// 2-Point Crossover
void crossover(int m, int q, int gen, vector<int> &pairs) {
    // get a random number to see if it should be crossover
    float chance = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

    // only perform the crossover for certain pairs
    if (chance <= CROSSOVER_RATE) {

        // create offsprings and parents
        vector<int> offspring1, offspring2;
        vector<int> parent1, parent2;

        // declare the parent and offspring vectors
        int p1 = pairs[q % Q];
        int p2 = pairs[q % Q + 1];
        parent1 = population[p1];
        parent2 = population[p2];
        
        // get randomize locations of the cuts
        int cut1 = rand() % (2*N - 7) + 4;
        int cut2 = rand() % (2*N - 3 - cut1) + cut1 + 2;

        // add the first part of each parent to the respective child (upto cut 1)
        copy(parent1.begin(), parent1.begin() + cut1, back_inserter(offspring1));
        copy(parent2.begin(), parent2.begin() + cut1, back_inserter(offspring2));

        // add the second part of each parent to the respective child (between cut 1 and cut 2)
        copy(parent2.begin() + cut1, parent2.begin() + cut2, back_inserter(offspring1));
        copy(parent1.begin() + cut1, parent1.begin() + cut2, back_inserter(offspring2));

        // add the last part of each parent to the respective child (after cut 2)
        copy(parent1.begin() + cut2, parent1.end(), back_inserter(offspring1));
        copy(parent2.begin() + cut2, parent2.end(), back_inserter(offspring2));

        // input the generation it is created
        offspring1[1] = gen;
        offspring2[1] = gen;

        threadlock.lock();
        offspring.emplace_back(offspring1);
        offspring.emplace_back(offspring2);
        threadlock.unlock();
    }
}

// Inverse Mutation
void mutate(int q) {

    // get a random number to see if it should be crossover
    float chance = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

    // only perform the crossover for certain pairs
    if (chance <= MUTATION_RATE) {
        int cut1 = rand() % (2*N - 4 - static_cast<int>(2*N*U)) + 3;  // find a random location for cut
        int cut2 = cut1 + (2*N*U);			              // go out 5% of the cut for the end of cut

        // mutate the subsequence (i.e. reverse it)
        for (int i = 0; 2*i <= cut2 - cut1; i++) {swap(offspring[q][cut1 + i], offspring[q][cut2 - i]);}
    }
}

// replace duplicate nodes with missing nodes
void adjust(vector<int> &pheno) {
    vector<int> missing;

    // find missing nodes
    for (int i = 1; i < N; i++) {
        bool found = false;

        // iterate through the nodes and pheno to find if nodes are missing
        for (int j = 3; j < pheno.size() - 1; j++) {
            if (nodes[i] == pheno[j]) {
                found = true;
                break;
            }
        }

        // if the node was not found then it is added to the missing vector
        if (found == false) {missing.push_back(nodes[i]);}
    }

    // replace duplicates with missing nodes
    for (int i = 3; i < pheno.size() - 1; i++) {

        // continue if the current node is not a depot
        if (pheno[i] != depot) {

            // iterate through the remaining nodes
            for (int j = i + 1; j < pheno.size() - 1; j++) {

                // if the current node is a duplicate replace it with a missing node or depot
                if (pheno[i] == pheno[j]) {

                    // if there are still non depot nodes replace it with one of them
                    if (missing.size() > 0) {
                        pheno[j] = missing[0];
                        missing.erase(missing.begin());
                    }

                    // else replace it with a depot
                    else {pheno[j] = depot;}
                }
            }
        }
    }

    // replace depots with remaining missing nodes
    int last = 3;
    for (int i = 0; i < missing.size(); i++) {
        for (int j = last; j < pheno.size() - 1; j++) { // iterate through the nodes, starting from the last replacement made
            if (pheno[j] == depot) {                    // if the current node is a depot
                pheno[j] = missing[0];                  // then replace the depot node with a missing node
                missing.erase(missing.begin());         // delete it from the missing vector
                last = j;                               // update the last location the depot was found
                break;                                  // break out of the loop
            }
        }
    }
}

// parrallizing functions
void offspring_crossover(int m) {
    vector<int> pairs;                                                // create an array for randomly pairing parents
    for (int q = m; q < m + Q; q++) {pairs.push_back(q);}             // add the indicies for the current thread
    shuffle(pairs, 0, Q);                                             // shuffle the array so each pair is randomly assigned
    for (int q = m; q < m + Q; q+= 2) {crossover(m, q, gen, pairs);}  // perform the crossover
}

void population_routes(int m) {for (int q = m; q < m + Q; q++) {routes(population[q]);}}
void population_two_opt(int m) {for (int q = m; q < m + Q; q++) {two_opt(population[q]);}}
void population_cost(int m) {for (int q = m; q < m + Q; q++) {population[q][0] = cost(population[q], 3, 2*N + 1);}}
void offspring_mutate(int m) {for (int q = m; q < min(m + Q, S); q++) {mutate(q);}}
void offspring_adjust(int m) {for (int q = m; q < min(m + Q, S); q++) {adjust(offspring[q]);}}
void offspring_routes(int m) {for (int q = m; q < min(m + Q, S); q++) {routes(offspring[q]);}}
void offspring_two_opt(int m) {for (int q = m; q < min(m + Q, S); q++) {two_opt(offspring[q]);}}
void offspring_cost(int m) {for (int q = m; q < min(m + Q, S); q++) {offspring[q][0] = cost(offspring[q], 3, 2*N + 1);}}
void select_best(int m) {for (int q = m; q < m + Q; q++) {population[q] = offspring[rankings[q][0]];}}
void clear_threads() {for (auto &thr : threads) {thr.join();} threads.clear();}

// select the best performing offspring
void select() {
    // Combine the population and offspring
    for (int m = 0; m < M; m++) {offspring.emplace_back(population[m]);}

    // Get the order and fitness of the population and offspring
    for (int m = 0; m < offspring.size(); m++) {rankings.push_back(vector<int> {m, offspring[m][0]});}

    // Sort the population + offspring by fitness
    sort(rankings.begin(), rankings.end(), [](const vector<int> &a, const vector<int> &b) {return a[1] < b[1];});

    // cpu // Select the best performing individuals for the next generation
    //for (int thr = 0; thr < MAX_THREADS; thr++) {int m=thr*M/MAX_THREADS; select_best(m);}
    for (int thr = 0; thr < MAX_THREADS; thr++) {int m=thr*Q; threads.emplace_back(thread(select_best, thr*M/MAX_THREADS));}
    clear_threads();

    // clear the vectors for the next generation
    rankings.clear();
    offspring.clear();
}

// get the score of the generation
void getScore() {
    vector<float> scores;
    for (int m = 0; m < M; m++) {scores.push_back(population[m][0]);}
    score = *min_element(scores.begin(), scores.end());
}

// determine if the ga should continue or not
bool executing(int interval) {return gen <= MAX_GENERATIONS && score > optimal && gen < interval;}

int main(int argc, char* argv[]) {
    
    // read in the problem in
    PROBLEM = argv[1];
    read();

    // initialize some additional parameters
    initialize();

    // transform x, y, and demand into single unique number (ID)
    for (int n = 0; n < N; n++) {nodes.push_back(encode(locations[2*n], locations[2*n + 1], demand[n]));}

    // ID of the depot
    depot = nodes[0];

    // initialize population (and offspring matrix)
    for (int m = 0; m < M; m++) {population.push_back(init_population(nodes));}

    // cpu // establish routes
    //for (int thr = 0; thr < MAX_THREADS; thr++) {int m=thr*Q; population_routes(m);}
    for (int thr = 0; thr < MAX_THREADS; thr++) {int m=thr*Q; threads.emplace_back(thread(population_routes, m));}
    clear_threads();

    // cpu // two-opt optimization
    //for (int thr = 0; thr < MAX_THREADS; thr++) {int m=thr*Q; population_two_opt(m);}
    for (int thr = 0; thr < MAX_THREADS; thr++) {int m=thr*Q; threads.emplace_back(thread(population_two_opt, m));}
    clear_threads();

    // cpu // get the fitness value for each phenotype
    //for (int thr = 0; thr < MAX_THREADS; thr++) {int m=thr*Q; population_cost(m);}
    for (int thr = 0; thr < MAX_THREADS; thr++) {int m=thr*Q; threads.emplace_back(thread(population_cost, m));}
    clear_threads();

    // get the score for the 0th generation
    getScore();

    // start the iterative process of the genetic algorithm
    start = chrono::high_resolution_clock::now();
    while(executing(MAX_GENERATIONS)) {
        int briefPoint = gen + BRIEFING_INTERVAL;
        brief();

        // iterate a few summary points before briefing again
        while(executing(briefPoint)) {
            int summaryPoint = gen + SUMMARY_INTERVAL;

            // iterate a few generations before summarizing
            for (gen = gen; executing(summaryPoint); gen++) {

                // reset the Q value
                Q = M/MAX_THREADS;

                // cpu // 2-Point Crossover
                //for (int thr = 0; thr < MAX_THREADS; thr++) {int m=thr*Q; offspring_crossover(m);}
                for (int thr = 0; thr < MAX_THREADS; thr++) {int m=thr*Q; threads.emplace_back(thread(offspring_crossover, m));}
                clear_threads();

                // get the number of offsprings and reset Q according to the number of offspring
                S = offspring.size();
                Q = 1 + S/MAX_THREADS;

                // cpu // Inverse Mutation
                //for (int thr = 0; thr < MAX_THREADS; thr++) {int m=thr*Q; offspring_mutate(m);}
                for (int thr = 0; thr < MAX_THREADS; thr++) {int m=thr*Q; threads.emplace_back(thread(offspring_mutate, m));}
                clear_threads();

                // cpu // Adjust the Population
                //for (int thr = 0; thr < MAX_THREADS; thr++) {int m=thr*Q; offspring_adjust(m);}
                for (int thr = 0; thr < MAX_THREADS; thr++) {int m=thr*Q; threads.emplace_back(thread(offspring_adjust, m));}
                clear_threads();

                // cpu // Establish the routes for the new phenotypes
                //for (int thr = 0; thr < MAX_THREADS; thr++) {int m=thr*Q; offspring_routes(m);}
                for (int thr = 0; thr < MAX_THREADS; thr++) {int m=thr*Q; threads.emplace_back(thread(offspring_routes, m));}
                clear_threads();

                // cpu // Perform two opt optimization on the offspring
                //for (int thr = 0; thr < MAX_THREADS; thr++) {int m=thr*Q; offspring_two_opt(m);}
                for (int thr = 0; thr < MAX_THREADS; thr++) {int m=thr*Q; threads.emplace_back(thread(offspring_two_opt, m));}
                clear_threads();

                // cpu // Get the fitness value for each new phenotype
                //for (int thr = 0; thr < MAX_THREADS; thr++) {int m=thr*Q; offspring_cost(m);}
                for (int thr = 0; thr < MAX_THREADS; thr++) {int m=thr*Q; threads.emplace_back(thread(offspring_cost, m));}
                clear_threads();

                // select the best performing individuals for the next population
                select();

                // get the score for that generation
                
                getScore();
            }
            summary();
        }
    }
    report(false);
    save(true);
    return 0;
}
