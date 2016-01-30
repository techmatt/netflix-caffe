
class NetworkProcessor
{
public:
    void init();
    void evaluateAllUsers(const NetflixDatabase &database);
    void outputUsers(const string &filename) const;

    void evaluateRating(const NetflixDatabase &database, const Rating &rating);

private:
    
    Netf net;
};
