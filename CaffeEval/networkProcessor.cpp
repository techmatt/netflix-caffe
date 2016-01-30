
#include "main.h"

void NetworkProcessor::init()
{
    const string baseDir = R"(D:\datasets\Netflix\caffe\)";
    const string netFilename = baseDir + "netflix-net.prototxt";
    const string modelFilename = baseDir + "netflix.caffemodel";

    net = Netf(new Net<float>(netFilename, caffe::TEST));
    net->CopyTrainedLayersFrom(modelFilename);
}

const bool saveNetsMode = true;
int globalUserIndex = 0;
void NetworkProcessor::evaluateRating(const NetflixDatabase &database, const Rating &rating)
{
    Grid3f inputData;
    const int channelCount = database.movieIndexCount * 3 + 1;
    vector<BYTE> rawVector(channelCount);

    database.makeLinearFeatures(rating, rawVector.data());

    inputData = Grid3f(1, 1, channelCount);
    for (auto &c : inputData)
    {
        const vec3i coord(c.x, c.y, c.z);
        const BYTE b = rawVector[c.z];

        const float scale = 1.0f / 255.0f;
        c.value = ((float)b - 128.0f) * scale;
    }
    
    if (saveNetsMode)
    {
        net->ForwardFrom(0);
        CaffeUtil::saveNetToDirectory(net, "netBefore" + to_string(globalUserIndex) + "/");
    }
    CaffeUtil::runNetForward(net, "slicer", "data", inputData);
    if (saveNetsMode) CaffeUtil::saveNetToDirectory(net, "netAfter" + to_string(globalUserIndex) + "/");
    
    globalUserIndex++;

    if (saveNetsMode && globalUserIndex == 6)
        exit(0);
    
    /*auto grid = CaffeUtil::getBlobAsGrid(net, "ip4");
    
    patient.netOutcome = CaffeUtil::gridToVector(grid);
    for (auto &f : iterate(patient.netOutcome))
    {
        //f = (B - mean) / 255.0f
        const float rescaledValue = f.value * 255.0f + meanValues(vec3i(0, 0, constants::featureCount * constants::coordsPerFeature + f.index));
        f.value = rescaledValue / 255.0f;
    }*/
}

void NetworkProcessor::evaluateAllUsers(const NetflixDatabase &database)
{
    for (auto &r : database.allRatings)
    {
        cout << "Evaluating rating " << r.userID << endl;
        evaluateRating(database, r);
    }
}
/*
void NetworkProcessor::outputPatients(const string &filename) const
{
    ofstream file(filename);

    file << "netoutcome: " << patients[0].netOutcome.size() << endl;
    
    file << "index,unstim,stim,status,label,survival time,truth,pred,diff";
    for (int i = 0; i < constants::survivalIntervals; i++)
        file << ",i" << i;
    file << endl;

    double trainingError = 0.0;
    int trainingCount = 0;
    double testError = 0.0;
    int testCount = 0;

    for (auto &p : patients)
    {
        ostringstream s;
        s << p.patient.index;
        s << "," << p.patient.fileUnstim;
        s << "," << p.patient.fileStim;
        s << "," << p.patient.status;
        s << "," << p.patient.label;
        s << "," << p.patient.survivalTime;

        file << s.str();
        for (auto &v : p.patient.makeOutcomeVector())
            file << "," << v;
        file << ",";
        for (auto &v : p.netOutcome)
            file << "," << setprecision(3) << v;
        file << ",";
        for (int i = 0; i < p.netOutcome.size(); i++)
            file << "," << setprecision(3) << p.patient.makeOutcomeVector()[i] - p.netOutcome[i];

        file << endl;

        double errorSum = 0.0;
        for (int i = 0; i < p.netOutcome.size(); i++)
            errorSum += math::abs(p.netOutcome[i] - p.patient.makeOutcomeVector()[i]);

        if (p.test)
        {
            testError += errorSum;
            testCount++;
        }
        else
        {
            trainingError += errorSum;
            trainingCount++;
        }
    }

    file << "Training error," << trainingError / trainingCount << endl;
    file << "Test error," << testError / testCount << endl;
}
*/