#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
// libraries
/*
Scenario test file format
base_armour
min_angle_deg
max_angle_deg
min_penetration_power
max_penetration_power
critical_hit_probability

Example scenario.txt:
100
0
70
80
160
0.25

Inputs have a range for monte carlo
*/

/*
struct to store one vehicle specifications
*/
typedef struct {
    double base_armour;
    double min_angle_deg;
    double max_angle_deg;
    double min_penetration_power;
    double max_penetration_power;
    double critical_hit_probability;
} Scenario;

/*
Name: usage
Purpose: print correct command line usage
Parameters: prog = argv[0] program name
*/
static void usage(const char *prog)
{
    fprintf(stderr, "Usage:\n  %s test_file trials\n", prog);
}

/*
Name: read_scenario
Purpose: read scenario values from txt file
Parameters:
filename = scenario input file
S = pointer to Scenario struct
*/
static void read_scenario(const char *filename, Scenario *S)
{
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror(filename); // error
        exit(1);
    }

    // read all 6 vehiclevalues
    if (fscanf(f, "%lf", &S->base_armour) != 1 ||
        fscanf(f, "%lf", &S->min_angle_deg) != 1 ||
        fscanf(f, "%lf", &S->max_angle_deg) != 1 ||
        fscanf(f, "%lf", &S->min_penetration_power) != 1 ||
        fscanf(f, "%lf", &S->max_penetration_power) != 1 ||
        fscanf(f, "%lf", &S->critical_hit_probability) != 1) {
        fprintf(stderr, "Error: test file is missing values.\n"); // error
        fclose(f);
        exit(1);
    }

    fclose(f);

    // checks for realistic values
    if (S->base_armour <= 0.0) {
        fprintf(stderr, "Error: any armour must be > 0.\n"); // error print
        exit(1);
    }

    if (S->min_angle_deg < 0.0 || S->max_angle_deg >= 90.0 || S->min_angle_deg > S->max_angle_deg) {
        fprintf(stderr, "Error: angle range must be 0 <= min <= max < 90.\n"); // error print
        exit(1);
    }

    if (S->min_penetration_power <= 0.0 || S->max_penetration_power < S->min_penetration_power) {
        fprintf(stderr, "Error: penetration power range is invalid.\n"); // error print
        exit(1);
    }

    if (S->critical_hit_probability < 0.0 || S->critical_hit_probability > 1.0) {
        fprintf(stderr, "Error: critical hit probability must be between 0 and 1.\n"); // error print
        exit(1);
    }
}

/*
Name: rand_unit
Purpose: return a random double
Parameters:
seed = thread-local seed for open mp
*/
static double rand_unit(unsigned int *seed)
{
    return (double)rand_r(seed) / ((double)RAND_MAX + 1.0);
}

/*
Name: rand_range
Purpose: return a random double in [low, high)
Parameters:
low = lower bound
high = upper bound
seed = thread-local seed
*/
static double rand_range(double low, double high, unsigned int *seed)
{
    return low + (high - low) * rand_unit(seed);
}

/*
Name: simulate_one_hit
Purpose:
simulate one attack on test vehicle and classify  result

Return values:
0 = no penetration
1 = penetration
2 = catastrophic kill / crew member unconscious / vehicle destroyed

generate random angle
compute effective armour using angle
generate random projectile power
compare projectile vs armour
if penetration, check catastrophic probability
*/
static int simulate_one_hit(const Scenario *S, unsigned int *seed)
{
    // random impact angle
    double angle_deg = rand_range(S->min_angle_deg, S->max_angle_deg, seed);
    double angle_rad = angle_deg * 3.14159265358979323846 / 180.0;

    // simple ww2 armor
    // more angle -> larger effective armour
    double effective_armour = S->base_armour / cos(angle_rad);

    // simple ap projectile penetration power
    double penetration_power = rand_range(S->min_penetration_power, S->max_penetration_power, seed);

    // compare projectile vs armour
    if (penetration_power <= effective_armour) {
        return 0; // no penetration
    }

    // if penetration happens, maybe crew member unconsious
    if (rand_unit(seed) < S->critical_hit_probability) {
        return 2; // crew member unconscious
    }

    return 1; // penetration but not catastrophic
}

//////////////////////////////////////// MAIN
/*
Purpose:
read scenario, run Monte Carlo simulation, print timing and results

argv[1] = scenario test file
argv[2] = number of trials

Steps:
read scenario
run simulation loop
measure execution time
print results and probabilities
*/
int main(int argc, char **argv)
{
    if (argc != 3) {
        usage(argv[0]);
        return 1;
    }

    const char *scenario_file = argv[1];

    // convert trials input
    char *endp = NULL;
    long trials = strtol(argv[2], &endp, 10);
    if (!endp || *endp != '\0' || trials <= 0) {
        fprintf(stderr, "Error:trials must be a positive integer.\n");
        usage(argv[0]);
        return 1;
    }

    Scenario S;
    read_scenario(scenario_file, &S);

    // counters for results
    long no_penetration = 0;
    long penetration = 0;
    long catastrophic_kill = 0;
    long survived = 0;
    // start timing
    double start = omp_get_wtime();

    /*
    openmp parallel Monte Carlo loop
    each thread uses its own seed
    */
    #pragma omp parallel
    {
        unsigned int seed = 12345u + (unsigned int)(omp_get_thread_num() * 1000 + 7);

        #pragma omp for reduction(+:no_penetration,penetration,catastrophic_kill,survived)
        for (long i = 0; i < trials; i++) {
            int result = simulate_one_hit(&S, &seed);

            if (result == 0) {
                no_penetration++;
                survived++;
            } else if (result == 1) {
                penetration++;
                survived++;
            } else {
                catastrophic_kill++;
            }
        }
    }
    // end timing
    double end = omp_get_wtime();
    double elapsed_sec = end - start;
    // print outputs
    printf("Test file: %s\n", scenario_file);
    printf("Trials: %ld\n", trials);
    printf("Threads: %d\n", omp_get_max_threads());
    printf("Elapsed Time seconds: %.6f\n", elapsed_sec);
    // print input values
    printf("\nTest values:\n");
    printf("Base armour = %.2f\n", S.base_armour);
    printf("Angle range = %.2f to %.2f degrees\n", S.min_angle_deg, S.max_angle_deg);
    printf("Penetration power range = %.2f to %.2f\n", S.min_penetration_power, S.max_penetration_power);
    printf("Critical hit probability = %.4f\n", S.critical_hit_probability);
    // print counts
    printf("\nSimulation results:\n");
    printf("No penetration = %ld\n", no_penetration);
    printf("Penetration = %ld\n", penetration);
    printf("Catastrophic kill = %ld\n", catastrophic_kill);
    printf("Survived = %ld\n", survived);
    // print probabilities
    printf("\nProbabilities:\n");
    printf("No penetration probability = %.6f\n", (double)no_penetration / (double)trials);
    printf("Penetration probability = %.6f\n", (double)penetration / (double)trials);
    printf("Catastrophic kill probability = %.6f\n", (double)catastrophic_kill / (double)trials);
    printf("Survivability probability = %.6f\n", (double)survived / (double)trials);

    /*
    one line outptut for easier plot
    */
    printf("\nCSV:\n");
    printf("trials,threads,time,no_pen_prob,pen_prob,kill_prob,survive_prob\n");
    printf("%ld,%d,%.6f,%.6f,%.6f,%.6f,%.6f\n", trials, omp_get_max_threads(), elapsed_sec, (double)no_penetration / (double)trials, (double)penetration / (double)trials, (double)catastrophic_kill / (double)trials, (double)survived / (double)trials);

    return 0;
}