
struct Rating
{
    Rating() {}
    Rating(int _movieIndex, int _userID, int _rating)
    {
        movieIndex = _movieIndex;
        userID = _userID;
        rating = _rating;
    }
    int movieIndex;
    int userID;
    int rating;
};

struct User
{
    User(int _id)
    {
        id = _id;
    }
    int id;
    vector<Rating> ratings;
};

struct NetflixDatabase
{
    void loadText();
    void loadBinary();
    void saveBinary();
    void processMovieFile(const string &filename, int movieIndex);
    void saveLevelDB(const string &outDir, int sampleCount);
    void makeLinearFeatures(const Rating &r, BYTE *output) const;

    unordered_map<int, User*> allUsers;
    
    vector<const User*> trainUsers;
    vector<const User*> testUsers;

    vector<Rating> trainRatings;

    vector<Rating> trainRatings;
    vector<Rating> testRatings;

    int movieIndexCount;
};
