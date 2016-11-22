#include "SysEnv.h"
#include <regiontemplates/autotuning/TuningInterface.h>
#include <regiontemplates/autotuning/geneticalgorithm/GeneticAlgorithm.h>
#include <regiontemplates/autotuning/activeharmony/ActiveHarmonyTuning.h>
#include "RegionTemplateCollection.h"
#include "Segmentation.h"
#include "DiceMaskComp.h"
#include "DiceNotCoolMaskComp.h"
#include "ParameterSet.h"
#include "hclient.h"
#include "NormalizationComp.h"

#define INF    100000

void parseInputArguments(int argc, char **argv, std::string &inputFolder, std::string &AHpolicy,
                         std::string &initPercent);

RegionTemplateCollection *RTFromFiles(std::string inputFolderPath);


int multiObjectiveTuning(int argc, char **argv, SysEnv &sysEnv, int max_number_of_tests, double metricWeight,
                         double timeWeight,
                         double &tSlowest, double &tFastest, RegionTemplateCollection *rtCollection,
                         std::string tuningPolicy,
                         float *perf, float *totaldiffs, float *dicePerIteration, float *diceNotCoolPerIteration,
                         uint64_t *totalexecutiontimes);

int objectiveFunctionProfiling(int argc, char **argv, SysEnv &sysEnv, int number_of_profiling_tests, double &tSlowest,
                               double &tFastest, RegionTemplateCollection *rtCollection,
                               std::string tuningPolicy, float *perf, float *totaldiffs, float *dicePerIteration,
                               float *diceNotCoolPerIteration,
                               uint64_t *totalexecutiontimes);


int main(int argc, char **argv) {



    //Multi-Objective Tuning weights
    double metricWeight = 1;
    double timeWeight = 1;


    //Multi-Objective Tuning normalization times
    double tSlowest; //Empirical Data that will be obtained with profiling
    double tFastest; //Empirical Data that will be obtained with profiling

    int max_number_of_tests = 100;

    // Folder when input data images are stored
    std::string inputFolderPath, tuningPolicy, initPercent;
    std::vector<RegionTemplate *> inputRegionTemplates;
    RegionTemplateCollection *rtCollection;
    parseInputArguments(argc, argv, inputFolderPath, tuningPolicy, initPercent);


    float *perf = (float *) malloc(sizeof(float) * max_number_of_tests);;
    float *totaldiffs = (float *) malloc(sizeof(float) * max_number_of_tests);
    float *dicePerIteration = (float *) malloc(sizeof(float) * max_number_of_tests);
    float *diceNotCoolPerIteration = (float *) malloc(sizeof(float) * max_number_of_tests);
    uint64_t *totalexecutiontimes = (uint64_t *) malloc(sizeof(uint64_t) * max_number_of_tests);


    // Create region templates description without instantiating data
    rtCollection = RTFromFiles(inputFolderPath);

    // Handler to the distributed execution system environment
    SysEnv sysEnv;
    // Tell the system which libraries should be used
    sysEnv.startupSystem(argc, argv, "libcomponenttuningnscale.so");

    // In case of multiobjective tuning is mandatory to peform a profiling of the slowest and fastest execution times that objective function.
    objectiveFunctionProfiling(argc, argv, sysEnv, 10, tSlowest, tFastest, rtCollection,
                               "nm", perf, totaldiffs, dicePerIteration, diceNotCoolPerIteration,
                               totalexecutiontimes);

    std::cout << "\t\tProfiling:" << std::endl;
    for (int i = 0; i < 10; ++i) {

        std::cout << std::fixed << std::setprecision(6) << "\t\tProf: " << i << " \tDiff: " << totaldiffs[i] <<
        "  \tExecution Time: " << totalexecutiontimes[i] << " \tDice: " << dicePerIteration[i] << " \tDiceNC: " <<
        diceNotCoolPerIteration[i];
        std::cout << "  \tPerf(weighted): " << perf[i] << std::endl;
    }
    std::cout << std::endl << "\t\ttSlowest: " << tSlowest << std::endl << "\t\ttFastest: " << tFastest << std::endl;

    // Peform the multiobjective tuning with the profiling data (tSlowest and tFastest)
    multiObjectiveTuning(argc, argv, sysEnv, max_number_of_tests, metricWeight, timeWeight, tSlowest, tFastest,
                         rtCollection,
                         tuningPolicy, perf, totaldiffs, dicePerIteration, diceNotCoolPerIteration,
                         totalexecutiontimes);

    // If you want to peform a singleobjective tuning, just change the metric and time weights. Ex.: metricWeight=1 and timeWeight=0



    std::cout << "\t\tResults:" << std::endl;
    for (int i = 0; i < max_number_of_tests; ++i) {

        std::cout << std::fixed << std::setprecision(6) << "\t\tTest: " << i << " \tDiff: " << totaldiffs[i] <<
        "  \tExecution Time: " << totalexecutiontimes[i] << " \tDice: " << dicePerIteration[i] << " \tDiceNC: " <<
        diceNotCoolPerIteration[i];
        std::cout << "  \tPerf(weighted): " << perf[i] << std::endl;
    }

    float minPerf = std::numeric_limits<float>::infinity();
    int minPerfIndex = 0;
    for (int j = 0; j < max_number_of_tests; ++j) {

        if (perf[j] < minPerf) {
            minPerf = perf[j];
            minPerfIndex = j;
        }
    }
    std::cout << std::endl;
    std::cout << "\t\tBest: " << minPerfIndex << " \tDiff: " << totaldiffs[minPerfIndex] <<
    "  \tExecution Time: " << totalexecutiontimes[minPerfIndex] << " \tDice: " << dicePerIteration[minPerfIndex] <<
    " \tDiceNC: " << diceNotCoolPerIteration[minPerfIndex];
    std::cout << std::endl;
    std::cout << "  \tPerf(weighted): " << perf[minPerfIndex] << std::endl;
    std::cout << "\tBest answer for MultiObjective Tuning has MinPerfWeighted: " << minPerf << std::endl;
    std::cout << "\tMetric of Best anwser: " << totaldiffs[minPerfIndex] << std::endl;
    std::cout << "\tTime of Best anwser: " << totalexecutiontimes[minPerfIndex] << std::endl;
    std::cout << "\tBest anwser index: " << minPerfIndex << std::endl;

    // Finalize all processes running and end execution
    sysEnv.finalizeSystem();

    delete rtCollection;

    return 0;
}


namespace patch {
    template<typename T>
    std::string to_string(const T &n) {
        std::ostringstream stm;
        stm << n;
        return stm.str();
    }
}

void parseInputArguments(int argc, char **argv, std::string &inputFolder, std::string &AHpolicy,
                         std::string &initPercent) {
    // Used for parameters parsing
    for (int i = 0; i < argc - 1; i++) {
        if (argv[i][0] == '-' && argv[i][1] == 'i') {
            inputFolder = argv[i + 1];
        }
        if (argv[i][0] == '-' && argv[i][1] == 'o') {
            initPercent = argv[i + 1];
        }
        if (argv[i][0] == '-' && argv[i][1] == 'f') {
            AHpolicy = argv[i + 1];
        }
    }
}


RegionTemplateCollection *RTFromFiles(std::string inputFolderPath) {
    // Search for input files in folder path
    std::string referenceMaskExtension = "_mask.txt"; //In case of labeled masks in text format;
    //std::string referenceMaskExtension = ".mask.png"; //In case of binary mask in png format;


    FileUtils fileUtils(referenceMaskExtension);
    std::vector<std::string> fileList;
    fileUtils.traverseDirectoryRecursive(inputFolderPath, fileList);
    RegionTemplateCollection *rtCollection = new RegionTemplateCollection();
    rtCollection->setName("inputimage");

    std::cout << "Input Folder: " << inputFolderPath << std::endl;

    // Create one region template instance for each input data file
    // (creates representations without instantiating them)
    for (int i = 0; i < fileList.size(); i++) {

        // Create input mask data region
        DenseDataRegion2D *ddr2d = new DenseDataRegion2D();
        ddr2d->setName("RAW");
        std::ostringstream oss;
        oss << i;
        ddr2d->setId(oss.str());
        ddr2d->setInputType(DataSourceType::FILE_SYSTEM);
        ddr2d->setIsAppInput(true);
        ddr2d->setOutputType(DataSourceType::FILE_SYSTEM);
        std::string inputFileName = fileUtils.replaceExt(fileList[i], referenceMaskExtension, ".tiff");
        ddr2d->setInputFileName(inputFileName);

        // Create reference mask data region
        DenseDataRegion2D *ddr2dRefMask = new DenseDataRegion2D();
        ddr2dRefMask->setName("REF_MASK");
        ddr2dRefMask->setId(oss.str());
        ddr2dRefMask->setInputType(DataSourceType::FILE_SYSTEM_TEXT_FILE);
        ddr2dRefMask->setIsAppInput(true);
        ddr2dRefMask->setOutputType(DataSourceType::FILE_SYSTEM_TEXT_FILE);
        cout << endl << "MASK FILE: " << fileList[i] << endl;
        ddr2dRefMask->setInputFileName(fileList[i]);

        // Adding data regions to region template
        RegionTemplate *rt = new RegionTemplate();
        rt->setName("tile");
        rt->insertDataRegion(ddr2d);
        rt->insertDataRegion(ddr2dRefMask);

        // Adding region template instance to collection
        rtCollection->addRT(rt);
    }

    return rtCollection;
}

int multiObjectiveTuning(int argc, char **argv, SysEnv &sysEnv, int max_number_of_tests, double metricWeight,
                         double timeWeight,
                         double &tSlowest, double &tFastest, RegionTemplateCollection *rtCollection,
                         std::string tuningPolicy,
                         float *perf, float *totaldiffs, float *dicePerIteration, float *diceNotCoolPerIteration,
                         uint64_t *totalexecutiontimes) {

    TuningInterface *tuningClient;
    int numClients;


    //USING AH
    if (tuningPolicy.find("nm") != std::string::npos || tuningPolicy.find("NM") != std::string::npos ||
        tuningPolicy.find("pro") != std::string::npos || tuningPolicy.find("PRO") != std::string::npos) {
        numClients = 1;
        tuningClient = new ActiveHarmonyTuning(tuningPolicy, max_number_of_tests, numClients);
    } else {


        //USING GA
        int max_number_of_generations = (int) floor(sqrt(max_number_of_tests));
        int mutationchance = 30;
        int crossoverrate = 50;
        int propagationamount = 2;

        int popsize = (int) ceil(sqrt(max_number_of_tests));


        numClients = popsize; //popsize
        tuningClient = new GeneticAlgorithm(max_number_of_generations, popsize, mutationchance,
                                            crossoverrate,
                                            propagationamount,
                                            1);

    }

    double *totalExecutionTimesNormalized = (double *) malloc(sizeof(double) * max_number_of_tests);

    std::vector<int> segComponentIds[numClients];
    std::vector<int> diceComponentIds[numClients];
    std::vector<int> diceNotCoolComponentIds[numClients];
    std::map<std::string, double> perfDataBase; //Checks if a param has been tested already

    int versionNorm = 0, versionSeg = 0;
    bool executedAlready[numClients];


    if (tuningClient->initialize(argc, argv) != 0) {
        fprintf(stderr, "Failed to initialize tuning session.\n");
        return -1;
    };


    if (tuningClient->declareParam("blue", 210, 240, 10) != 0 ||
        tuningClient->declareParam("green", 210, 240, 10) != 0 ||
        tuningClient->declareParam("red", 210, 240, 10) != 0 ||
        tuningClient->declareParam("T1", 2.5, 7.5, 0.5) != 0 ||
        tuningClient->declareParam("T2", 2.5, 7.5, 0.5) != 0 ||
        tuningClient->declareParam("G1", 5, 80, 5) != 0 ||
        tuningClient->declareParam("minSize", 2, 40, 2) != 0 ||
        tuningClient->declareParam("maxSize", 900, 1500, 50) != 0 ||
        tuningClient->declareParam("G2", 2, 40, 2) != 0 ||
        tuningClient->declareParam("minSizePl", 5, 80, 5) != 0 ||
        tuningClient->declareParam("minSizeSeg", 2, 40, 2) != 0 ||
        tuningClient->declareParam("maxSizeSeg", 900, 1500, 50) != 0 ||
        tuningClient->declareParam("fillHoles", 4, 8, 4) != 0 ||
        tuningClient->declareParam("recon", 4, 8, 4) != 0 ||
        tuningClient->declareParam("watershed", 4, 8, 4) != 0) {
        fprintf(stderr, "Failed to define tuning session\n");
        return -1;
    }

    if (tuningClient->configure() != 0) {
        fprintf(stderr, "Failed to initialize tuning session.\n");
        return -1;
    };

    for (; !tuningClient->hasConverged();) {
        cout << "ITERATION: " << tuningClient->getIteration() << endl;


        //Get new param suggestions from the tuning client
        tuningClient->fetchParams();
        //Apply fitness function for each individual
        for (int i = 0; i < numClients; i++) {

            std::ostringstream oss;
            oss << "PARAMS";
            typedef std::map<std::string, double *>::iterator it_type;
            for (it_type iterator = tuningClient->getParamSet(i)->paramSet.begin();
                 iterator != tuningClient->getParamSet(i)->paramSet.end(); iterator++) {
                //iterator.first key
                //iterator.second value
                oss << " - " << iterator->first << ": " << *(iterator->second);
            }

            // / if not found in performance database
            if (perfDataBase.find(oss.str()) != perfDataBase.end()) {
                perf[tuningClient->getIteration() * numClients +
                     (i)] = perfDataBase.find(oss.str())->second;
                std::cout << "Parameters already tested: " << oss.str() << " perf: " << perf << std::endl;

                executedAlready[i] = true;
            } else {
                executedAlready[i] = false;
            }

        }


        int segCount = 0;
        // Build application dependency graph
        // Instantiate application dependency graph
        for (int i = 0; i < rtCollection->getNumRTs(); i++) {

            int previousSegCompId = 0;
            // CREATE NORMALIZATION STEP
            ParameterSet parSetNormalization;
            std::vector<ArgumentBase *> targetMeanOptions;
            ArgumentFloatArray *targetMeanAux = new ArgumentFloatArray(ArgumentFloat(-0.632356));
            targetMeanAux->addArgValue(ArgumentFloat(-0.0516004));
            targetMeanAux->addArgValue(ArgumentFloat(0.0376543));
            targetMeanOptions.push_back(targetMeanAux);
            parSetNormalization.addArguments(targetMeanOptions);
            parSetNormalization.resetIterator();
            std::vector<ArgumentBase *> argSetInstanceNorm = parSetNormalization.getNextArgumentSetInstance();
            NormalizationComp *norm = new NormalizationComp();
            // normalization parameters
            norm->addArgument(new ArgumentInt(versionNorm));
            norm->addArgument(argSetInstanceNorm[0]);
            norm->addRegionTemplateInstance(rtCollection->getRT(i), rtCollection->getRT(i)->getName());
            sysEnv.executeComponent(norm);

            for (int j = 0; j < numClients; j++) {

                if (executedAlready[j] == false) {

                    std::cout << "BEGIN: LoopIdx: " << tuningClient->getIteration() * numClients + (j);

                    typedef std::map<std::string, double *>::iterator it_type;
                    for (it_type iterator = tuningClient->getParamSet(j)->paramSet.begin();
                         iterator != tuningClient->getParamSet(j)->paramSet.end(); iterator++) {
                        std::cout << " - " << iterator->first << ": " << *(iterator->second);
                    }

                    std::cout << std::endl;

                    // Creating segmentation component
                    Segmentation *seg = new Segmentation();

                    // version of the data region red. Each parameter instance in norm creates a output w/ different version
                    seg->addArgument(new ArgumentInt(versionNorm));
                    // version of the data region generated by the segmentation stage
                    seg->addArgument(new ArgumentInt(versionSeg));

                    // add remaining (application specific) parameters from the argSegInstance
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("blue", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("green", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("red", j))));
                    seg->addArgument(
                            new ArgumentFloat((float) (tuningClient->getParamValue("T1", j))));
                    seg->addArgument(
                            new ArgumentFloat((float) (tuningClient->getParamValue("T2", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("G1", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("G2", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("minSize", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("maxSize", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("minSizePl", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("minSizeSeg", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("maxSizeSeg", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("fillHoles", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("recon", j))));
                    seg->addArgument(new ArgumentInt(
                            (int) round(tuningClient->getParamValue("watershed", j))));


                    // and region template instance that it is suppose to process
                    seg->addRegionTemplateInstance(rtCollection->getRT(i), rtCollection->getRT(i)->getName());
                    seg->addDependency(norm->getId());

                    std::cout << "Creating DiffMask" << std::endl;
                    DiceMaskComp *diceComp = new DiceMaskComp();
                    DiceNotCoolMaskComp *diceNotCoolComp = new DiceNotCoolMaskComp();

                    // version of the data region that will be read. It is created during the segmentation.
                    diceComp->addArgument(new ArgumentInt(versionSeg));
                    diceNotCoolComp->addArgument(new ArgumentInt(versionSeg));

                    // region template name
                    diceComp->addRegionTemplateInstance(rtCollection->getRT(i), rtCollection->getRT(i)->getName());
                    diceComp->addDependency(seg->getId());
                    diceNotCoolComp->addRegionTemplateInstance(rtCollection->getRT(i),
                                                               rtCollection->getRT(i)->getName());
                    diceNotCoolComp->addDependency(diceComp->getId());

                    // add to the list of diff component ids.
                    segComponentIds[j].push_back(seg->getId());
                    diceComponentIds[j].push_back(diceComp->getId());
                    diceNotCoolComponentIds[j].push_back(diceNotCoolComp->getId());

                    sysEnv.executeComponent(seg);
                    sysEnv.executeComponent(diceComp);
                    sysEnv.executeComponent(diceNotCoolComp);

                    std::cout << "Manager CompId: " << diceComp->getId() << " fileName: " <<
                    rtCollection->getRT(i)->getDataRegion(0)->getInputFileName() << std::endl;
                    segCount++;
                    versionSeg++;

                }
            }
            versionNorm++;
        }

        // End Creating Dependency Graph
        sysEnv.startupExecution();


        //==============================================================================================
        //Fetch results from execution workflow
        //==============================================================================================
        for (int j = 0; j < numClients; j++) {
            float dice = 0;
            float secondaryMetric = 0;
            float diceNotCoolValue = 0;

            std::ostringstream oss;
            oss << "PARAMS";
            typedef std::map<std::string, double *>::iterator it_type;
            for (it_type iterator = tuningClient->getParamSet(j)->paramSet.begin();
                 iterator != tuningClient->getParamSet(j)->paramSet.end(); iterator++) {
                oss << " - " << iterator->first << ": " << *(iterator->second);
            }

            if (executedAlready[j] == false) {
                for (int i = 0; i < diceComponentIds[j].size(); i++) {
                    char *diceResultData = sysEnv.getComponentResultData(diceComponentIds[j][i]);
                    char *diceNotCoolResultData = sysEnv.getComponentResultData(diceNotCoolComponentIds[j][i]);
                    std::cout << "Diff Id: " << diceComponentIds[j][i] << " \tdiceResultData: ";
                    if (diceResultData != NULL) {
                        std::cout << "size: " << ((int *) diceResultData)[0] << " \thadoopgis-metric: " <<
                        ((float *) diceResultData)[1] <<
                        " \tsecondary: " << ((float *) diceResultData)[2] << " \tdiceNotCool: " <<
                        ((float *) diceNotCoolResultData)[1] << std::endl;
                        dice += ((float *) diceResultData)[1];
                        secondaryMetric += ((float *) diceResultData)[2];
                        diceNotCoolValue += ((float *) diceNotCoolResultData)[1];
                    } else {
                        std::cout << "NULL" << std::endl;
                    }
                    char *segExecutionTime = sysEnv.getComponentResultData(segComponentIds[j][i]);
                    if (segExecutionTime != NULL) {
                        totalexecutiontimes[tuningClient->getIteration() * numClients +
                                            (j)] = ((int *) segExecutionTime)[1];
                        cout << "Segmentation execution time:" <<
                        totalexecutiontimes[tuningClient->getIteration() * numClients + (j)] << endl;
                    }
                    sysEnv.eraseResultData(diceNotCoolComponentIds[j][i]);
                    sysEnv.eraseResultData(diceComponentIds[j][i]);
                    sysEnv.eraseResultData(segComponentIds[j][i]);
                }
                diceComponentIds[j].clear();
                segComponentIds[j].clear();
                diceNotCoolComponentIds[j].clear();


                //########################################################################################
                //Multi Objective Tuning
                //########################################################################################

                float diff = (dice + diceNotCoolValue) / 2;
                if (diff <= 0) diff = FLT_EPSILON;

                double timeNormalized =
                        (tSlowest - (double) totalexecutiontimes[tuningClient->getIteration() * numClients + (j)]) /
                        (tSlowest - tFastest);

                double weightedSumOfMetricAndTime = (double) (metricWeight * diff + timeWeight * timeNormalized);
                if (weightedSumOfMetricAndTime > 0) {
                    perf[tuningClient->getIteration() * numClients + (j)] = (double) 1 /
                                                                            weightedSumOfMetricAndTime; //Multi Objective Tuning
                } else {
                    perf[tuningClient->getIteration() * numClients + (j)] = std::numeric_limits<double>::infinity();
                }



                totalExecutionTimesNormalized[tuningClient->getIteration() * numClients + (j)] = timeNormalized;
                if (perf[tuningClient->getIteration() * numClients + (j)] < 0)
                    perf[tuningClient->getIteration() * numClients + (j)] = -perf[
                            tuningClient->getIteration() * numClients + (j)];

                std::cout << "END: LoopIdx: " << tuningClient->getIteration() * numClients + (j);
                typedef std::map<std::string, double *>::iterator it_type;
                for (it_type iterator = tuningClient->getParamSet(j)->paramSet.begin();
                     iterator != tuningClient->getParamSet(j)->paramSet.end(); iterator++) {
                    //iterator.first key
                    //iterator.second value
                    std::cout << " - " << iterator->first << ": " << *(iterator->second);
                }

                cout << endl << endl << "\tDiff: " << diff << " Secondary Metric: " << secondaryMetric <<
                " Time Normalized: " << timeNormalized << " Segmentation Time: " <<
                totalexecutiontimes[tuningClient->getIteration() * numClients + (j)] << " Perf: " <<
                perf[tuningClient->getIteration() * numClients + (j)] << endl;

                totaldiffs[tuningClient->getIteration() * numClients + (j)] = diff;
                dicePerIteration[tuningClient->getIteration() * numClients + (j)] = dice;
                diceNotCoolPerIteration[tuningClient->getIteration() * numClients + (j)] = diceNotCoolValue;


                perfDataBase[oss.str()] = perf[tuningClient->getIteration() * numClients + (j)];
            } else {
                perf[tuningClient->getIteration() * numClients + (j)] = perfDataBase[oss.str()];

                std::cout << "ATTENTION! Param set executed already:" << std::endl;
                std::cout << "END: LoopIdx: " << tuningClient->getIteration() * numClients + (j);
                std::cout << oss.str() << endl;

                std::cout << " perf: " << perf[tuningClient->getIteration() * numClients + (j)] << std::endl;
            }

            // Report the performance we've just measured.
            tuningClient->reportScore(perf[tuningClient->getIteration() * numClients + (j)], j);
        }

        //Checks if at least one test in this iteration succeeded.
        bool shouldIterate = false;
        for (int k = 0; k < numClients; ++k) {
            shouldIterate |= !executedAlready[k];
        }
        //Iterates the tuning algorithm
        if (shouldIterate == true) tuningClient->nextIteration();

    }

}

int objectiveFunctionProfiling(int argc, char **argv, SysEnv &sysEnv, int number_of_profiling_tests, double &tSlowest,
                               double &tFastest, RegionTemplateCollection *rtCollection,
                               std::string tuningPolicy, float *perf, float *totaldiffs, float *dicePerIteration,
                               float *diceNotCoolPerIteration,
                               uint64_t *totalexecutiontimes) {

    multiObjectiveTuning(argc, argv, sysEnv, number_of_profiling_tests, 1, 0, tSlowest, tFastest, rtCollection,
                         tuningPolicy, perf, totaldiffs, dicePerIteration, diceNotCoolPerIteration,
                         totalexecutiontimes);

    //Peform the profiling
    //Get the slowest time for that objective function in the machine
    //Get the fastest time for that objective function in the machine
    tSlowest = 0;
    tFastest = std::numeric_limits<double>::infinity();
    for (int i = 0; i < number_of_profiling_tests; ++i) {
        if (totalexecutiontimes[i] > tSlowest) tSlowest = totalexecutiontimes[i];
        if (totalexecutiontimes[i] < tFastest) tFastest = totalexecutiontimes[i];
    }
}