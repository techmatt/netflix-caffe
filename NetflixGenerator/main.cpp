
#include "main.h"

void goB()
{
    NetflixDatabase database;
    //database.loadText();
    //database.saveBinary();
    database.loadBinary();

    cout << "movies: " << database.movieIndexCount << endl;

    database.saveLevelDB(constants::netflixDir + "caffe/LevelDBTrain", 1000000);
    database.saveLevelDB(constants::netflixDir + "caffe/LevelDBTest", 10000);
}

void main()
{
    goB();
    //goA();
    
    cout << "done!" << endl;
    cin.get();
}
