//
// Created by jon on 02/08/17.
//

#include <memory>
#include <random>
#include <utility>
#include "simulationrunner.h"

SimulationRunner::SimulationRunner(std::vector<std::shared_ptr<SimulationManager>> mans, std::vector<clDevice> devs, bool double_precision) : managers(
        std::move(mans)), start(true), use_double_precision(double_precision)
{
    dev_list = std::move(devs);
}

void SimulationRunner::runSimulations()
{
    start = true;
    CLOG(DEBUG, "gui") << "Running through " << managers.size() << " managers";
    for (const auto &m : managers)
        runSingle(m);
}

void SimulationRunner::runSingle(std::shared_ptr<SimulationManager> sim_pointer)
{
    CLOG(DEBUG, "gui") << "Splitting jobs";
    auto jobs = SplitJobs(std::move(sim_pointer));

    CLOG(DEBUG, "gui") << "Making threadpool";
    t_pool = std::make_unique<ThreadPool>(dev_list, jobs.size(), use_double_precision);

    if (!start)
        return;

    std::vector<std::future<void>> results;

    // enqueue the jobs here using the thread pool
    for (const auto &job : jobs) {
        try {
            results.emplace_back(t_pool->enqueue(job));
        } catch (std::runtime_error & e)
        {
            CLOG(ERROR, "cmd") << "Could not run all jobs: " << e.what();
            return;
        }
    }

    for (auto && result: results)
        result.get();
}

std::vector<std::shared_ptr<SimulationJob>> SimulationRunner::SplitJobs(std::shared_ptr<SimulationManager> simManager)
{
    auto mode = simManager->mode();
    unsigned long nJobs = simManager->totalParts();

    std::vector<std::shared_ptr<SimulationJob>> jobs(nJobs);

    // make the jobs, I'll have to implement a system for the simulation to recognise when it is done and to
    // export the files. That at least makes this simple, just create a list of jobs

    // later on might want a way to combine (some of) the TDS runs into individual jobs.
    if (mode == SimulationMode::CTEM)
        for (int i = 0; i < nJobs; ++i)
            jobs[i] = std::make_shared<SimulationJob>(simManager, i);
    else if (mode == SimulationMode::CBED)
        for (int i = 0; i < nJobs; ++i)
            jobs[i] = std::make_shared<SimulationJob>(simManager, i);
    else if (mode == SimulationMode::STEM)
    {
        unsigned int StemParallel = simManager->parallelPixels();
        //TODO: avoid most of this if StemParallel is set to 1
        // generate an array of all our pixels (just increment them)
        std::vector<int> pixelIndices((unsigned long) simManager->stemArea()->getNumPixels());
        //std::iota(std::begin(pixelIndices), std::end(pixelIndices), 0); // this is the bit that fills the vector with incrementing integers

        int counter = 0;
        int px = simManager->stemArea()->getPixelsX();
        int py = simManager->stemArea()->getPixelsY();
        for (int i = 0; i < py; ++i)
            for (int j = 0; j < px; ++j) {
                // this modifies the pixels to scan from top to bottom (like a real STEM)
                int ind = j + (py - 1 - i) * px;
                pixelIndices[counter] = ind;
                ++counter;
            }


        // random generator stuff from https://stackoverflow.com/a/6926473
//        std::random_device rd;
        std::mt19937_64 rng(std::mt19937_64(std::chrono::system_clock::now().time_since_epoch().count()));

        unsigned int jobCount = 0;
        unsigned int inelastic_iterations = simManager->incoherenceEffects()->iterations(mode);
        for (int i = 0; i < inelastic_iterations; ++i)
        {
            // this works in place (?) so we are continuously shuffling the same array, probably a good thing??
            if (simManager->incoherenceEffects()->enabled(mode) && simManager->parallelPixels() > 1)
                std::shuffle(pixelIndices.begin(), pixelIndices.end(), rng);

            // now divvy up the array into out StemParallel segments
            // here I will even out the load (so there isn't one short simulation just doing a last few pixels at the end

            // get the number of sims we need to do, redivide it to work out how to split this evenly
            // and calculate the straggling remainders
            auto total_pixels = simManager->stemArea()->getNumPixels();
            auto nSims = (unsigned int) std::ceil( (double) total_pixels / (double) StemParallel );
//            unsigned int pxPerSim = (unsigned int) std::floor(simManager->stemArea()->getNumPixels() / nSims );
//            int remainder = simManager->stemArea()->getNumPixels() % nSims;

            unsigned int current = 0;
            for (int j = 0; j < nSims; ++j)
            {
                // account for the remainding pixels
                unsigned int n = StemParallel;

                if (current + n > total_pixels) {
                    n = total_pixels - current;
                }

                // copy the indices for one job
                std::vector<int> temp(pixelIndices.begin() + current, pixelIndices.begin() + current + n);
                current += n; // TODO: check I don't need to increment this by an extra 1

                // make that job
                jobs[jobCount] = std::make_shared<SimulationJob>(simManager, temp, jobCount);
                jobCount++;
            }

        }
    }

    return jobs;
}
