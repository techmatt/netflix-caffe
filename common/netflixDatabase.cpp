
#include "main.h"

#include <leveldb/db.h>
#include <leveldb/write_batch.h>

#include <stdint.h>
#include <sys/stat.h>
#include <direct.h>

#include "caffe/proto/caffe.pb.h"

using namespace caffe;

void NetflixDatabase::makeLinearFeatures(const Rating &r, BYTE *output) const
{
    auto ratingToByte = [](int rating)
    {
        if (rating == 1) return (BYTE)0;
        if (rating == 2) return (BYTE)64;
        if (rating == 3) return (BYTE)128;
        if (rating == 4) return (BYTE)192;
        if (rating == 5) return (BYTE)255;
        return (BYTE)128;
    };

    BYTE *userRating = output + movieIndexCount * 0;
    BYTE *userIndicator = output + movieIndexCount * 1;
    BYTE *movieIndicator = output + movieIndexCount * 2;
    BYTE *targetRating = output + movieIndexCount * 3;

    const User &u = *allUsers.find(r.userID)->second;

    for (int i = 0; i < movieIndexCount; i++)
    {
        userRating[i] = 128;
        userIndicator[i] = 128;
        movieIndicator[i] = 128;
    }

    movieIndicator[r.movieIndex] = 255;

    for (Rating otherR : u.ratings)
    {
        if (otherR.movieIndex != r.movieIndex)
        {
            userRating[otherR.movieIndex] = ratingToByte(otherR.rating);
            userIndicator[otherR.movieIndex] = 255;
        }
    }

    targetRating[0] = ratingToByte(r.rating);
}

void NetflixDatabase::saveLevelDB(const string &outDir, int sampleCount)
{
    // leveldb
    leveldb::DB* db;
    leveldb::Options options;
    options.error_if_exists = true;
    options.create_if_missing = true;
    options.write_buffer_size = 268435456;

    // Open dbs
    std::cout << "Opening leveldb " << constants::netflixDir << endl;
    leveldb::Status status = leveldb::DB::Open(options, outDir, &db);
    if (!status.ok())
    {
        std::cout << "Failed to open " << outDir << " or it already exists" << endl;
        return;
    }

    leveldb::WriteBatch* batch = new leveldb::WriteBatch();

    // Storing to db
    const int channelCount = movieIndexCount * 3 + 1;
    BYTE *rawVector = new BYTE[channelCount];

    int count = 0;
    const int kMaxKeyLength = 10;
    char key_cstr[kMaxKeyLength];

    cout << "A total of " << sampleCount << " samples will be generated." << endl;
    int totalSampleIndex = 0;

    for (int sampleIndex = 0; sampleIndex < sampleCount; sampleIndex++)
    {
        if (totalSampleIndex % 20 == 0)
            cout << "Sample " << totalSampleIndex << " / " << sampleCount << endl;
        totalSampleIndex++;

        Datum datum;
        datum.set_label(0);

        datum.set_channels(channelCount);
        datum.set_height(1);
        datum.set_width(1);

        const Rating r = util::randomElement(trainRatings);
        makeLinearFeatures(r, rawVector);
        
        datum.set_data(rawVector, channelCount);

        sprintf_s(key_cstr, kMaxKeyLength, "%08d", totalSampleIndex);

        string value;
        datum.SerializeToString(&value);

        string keystr(key_cstr);

        // Put in db
        batch->Put(keystr, value);

        if (++count % 1000 == 0) {
            // Commit txn
            db->Write(leveldb::WriteOptions(), batch);
            delete batch;
            batch = new leveldb::WriteBatch();
        }
    }

    // write the last batch
    if (count % 1000 != 0) {
        db->Write(leveldb::WriteOptions(), batch);
    }
    delete batch;
    delete db;
    cout << "Processed " << count << " entries." << endl;
}

void NetflixDatabase::loadText()
{
    for (auto &filename : Directory::enumerateFiles(constants::trainingDir))
    {
        cout << "loading " << filename << endl;
        const string baseFilename = util::removeExtensions(filename);
        const int movieIndex = convert::toInt(util::remove(baseFilename, "mv_"));
        processMovieFile(constants::trainingDir + filename, movieIndex);
    }
}

void NetflixDatabase::saveBinary()
{
    BinaryDataStreamFile out(constants::netflixDir + "database.dat", true);
    out.writePrimitive(allRatings);
    out.closeStream();
}

void NetflixDatabase::loadBinary()
{
    cout << "Loading from disk..." << endl;
    BinaryDataStreamFile in(constants::netflixDir + "database.dat", false);
    in.readPrimitive(allRatings);
    in.closeStream();

    cout << "Adding all users..." << endl;
    movieIndexCount = 0;
    for (const Rating &r : allRatings)
    {
        if (users.count(r.userID) == 0)
        {
            User *newUser = new User(r.userID);
            users[r.userID] = newUser;
            userList.push_back(newUser);
        }

        movieIndexCount = max(movieIndexCount, r.movieIndex + 1);
        users[r.userID]->ratings.push_back(r);
    }
    cout << "done" << endl;
}

void NetflixDatabase::processMovieFile(const string &filename, int movieIndex)
{
    for (const string &line : util::getFileLines(filename, 3))
    {
        const auto words = util::split(line, ',');
        if (words.size() != 3)
            continue;

        const int userID = convert::toInt(words[0]);
        const int rating = convert::toInt(words[1]);

        allRatings.push_back(Rating(movieIndex, userID, rating));
    }
}
