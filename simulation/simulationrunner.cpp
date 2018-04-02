//
// Created by jon on 02/08/17.
//

#include <random>
#include "simulationrunner.h"

SimulationRunner::SimulationRunner(std::vector<std::shared_ptr<SimulationManager>> mans, std::vector<clDevice> devs) : managers(mans)
{
    dev_list = devs;
}

void SimulationRunner::runSimulations()
{
    for (auto m : managers)
        runSingle(m);
}

void SimulationRunner::runSingle(std::shared_ptr<SimulationManager> sim_pointer)
{
    auto jobs = SplitJobs(sim_pointer);

    t_pool = std::make_unique<ThreadPool>(dev_list, jobs.size());

    for (int i = 0; i < jobs.size(); ++i)
    {
        // enqueue the jobs here using the thread pool
        t_pool->enqueue(jobs[i]);
    }
}

std::vector<std::shared_ptr<SimulationJob>> SimulationRunner::SplitJobs(std::shared_ptr<SimulationManager> simManager)
{
    auto mode = simManager->getMode();
    unsigned long nJobs = simManager->getTotalParts();

    std::vector<std::shared_ptr<SimulationJob>> jobs(nJobs);

    // make the jobs, I'll have to implement a system for the simulation to recognise when it is done and to
    // export the files. That at least makes this simple, just create a list of jobs

    // later on might want a way to combine (some of) the TDS runs into individual jobs.
    if (mode == SimulationMode::CTEM)
        jobs[0] = std::shared_ptr<SimulationJob>(new SimulationJob(simManager));
    else if (mode == SimulationMode::CBED)
        for (int i = 0; i < nJobs; ++i)
            jobs[i] = std::shared_ptr<SimulationJob>(new SimulationJob(simManager));
    else if (mode == SimulationMode::STEM)
    {
        int StemParallel = simManager->getParallelPixels();
        //TODO: avoid most of this if StemParallel is set to 1
        // generate an array of all our pixels (just increment them)
        std::vector<int> pixelIndices((unsigned long) simManager->getStemArea()->getNumPixels());
        std::iota(std::begin(pixelIndices), std::end(pixelIndices), 0); // this is the bit that fills the vector with incrementing integers

        // random generator stuff from https://stackoverflow.com/a/6926473
        static std::random_device rd;
        static std::mt19937 rng(rd());

        unsigned int jobCount = 0;
        for (int i = 0; i < simManager->getTdsRuns(); ++i)
        {
            // this works in place (?) so we are continuously shuffling the same array, probably a good thing??
            std::shuffle(pixelIndices.begin(), pixelIndices.end(), rng);

            // now divvy up the array into out StemParallel segments
            // here I will even out the load (so there isn't one short simulation just doing a last few pixels at the end

            // get the number of sims we need to do, redivide it to work out how to split this evenly
            // and calculate the straggling remainders
            unsigned int nSims = (unsigned int) std::ceil(simManager->getStemArea()->getNumPixels() / StemParallel);
            unsigned int pxPerSim = (unsigned int) std::floor(simManager->getStemArea()->getNumPixels() / nSims );
            int remainder = simManager->getStemArea()->getNumPixels() % nSims;

            unsigned int current = 0;
            for (int j = 0; j < nSims; ++j)
            {
                // account for the remainding pixels
                int n = pxPerSim + 1 * (remainder > 1);
                --remainder; // just decrement always, ignore it when it is negative

                // copy the indices for one job
                std::vector<int> temp(pixelIndices.begin() + current, pixelIndices.begin() + current + n);
                current += n; // TODO: check I don't need to increment this by an extra 1

                // make that job
                jobs[jobCount] = std::shared_ptr<SimulationJob>(new SimulationJob(simManager, temp));
                jobCount++;
            }

        }
    }

    return jobs;
}
