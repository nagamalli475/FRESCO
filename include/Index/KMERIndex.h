#ifndef KMERINDEX_H_INCLUDED
#define KMERINDEX_H_INCLUDED

//#define MAXBUCKETS 5000000 //removed

int hashforkmer(const char *str, long kml, long mb)
{
    int h = 0;
    long pcount=0;
    while (*str and pcount<kml)
    {
       h = h << 2 ^ *str++;
       pcount++;
    }
    return abs(h%mb);
}

void getKMERParameters(char*p, long filesize, long &kmerlength, long &maxbuckets)
{
    long sigmasize=SystemTools::getSymbolSize(p, filesize);


    if (sigmasize<10)
    {
        if (filesize>1000000)
        {
            kmerlength=24;
            maxbuckets=1000000;
        }else
        {
            kmerlength=10;
            maxbuckets=100000;
        }

    }else{

        if (filesize>100000)
        {
            kmerlength=4;
            maxbuckets=100000;
        }else
        {
            kmerlength=3;
            maxbuckets=10000;
        }

    }

}



class KMERIndex: public AbstractIndex
{
public:

    KMERIndex(long _KMERLENGTH=16, long _MAXBUCKETS=500000): MAXBUCKETS(_MAXBUCKETS)
    {
        KMERLENGTH=_KMERLENGTH;



    }

    ~KMERIndex()
    {
        delete[] refcontent;

        for (long l=0;l<MAXBUCKETS;l++)
            delete (kmermap[l]);
    }


    void initialize(string referencePath)
    {
        StopWatch sw;

        long filesize=SystemTools::getFileSize(referencePath);

        string reference=loadFromRaw(referencePath);
        reflength=reference.length();


        //long kmerlength, maxbuckets;
        getKMERParameters((char*)reference.c_str(), filesize, KMERLENGTH, MAXBUCKETS);
        cout<<"K="<<KMERLENGTH<<", maxbuckets="<<MAXBUCKETS<<"\n";



        for (long l=0;l<MAXBUCKETS;l++)
        {
            vector<int>* newv=new vector<int>;
            //newv->reserve(20);
            kmermap.push_back(newv);
        }

        refcontent=new char[reflength+16];
        strcpy(refcontent, reference.c_str());
        for (long l=0;l<16;l++)
            refcontent[reflength+l]=0;

        StopWatch sw2;
        for (long pos=0;pos<reflength-KMERLENGTH+1;pos++)
            kmermap[hashforkmer(refcontent+pos,KMERLENGTH,MAXBUCKETS)]->push_back(pos);
        sw2.stop(false);

        sw.stop(false);
        //cout<<"Indexing speed: "<<((double)reflength/(1024*1024)/(sw.getDiff()/1000))<<" MB/s\n";
    }

    void initializeFromMainMemory(string reference)
    {
        StopWatch sw;

        //string reference=loadFromRaw(referencePath);
        reflength=reference.length();
        refcontent=new char[reflength+16];
        strcpy(refcontent, reference.c_str());
        for (long l=0;l<16;l++)
            refcontent[reflength+l]=0;

        //StopWatch sw2;
        for (long pos=0;pos<reflength-KMERLENGTH+1;pos++)
            kmermap[hashforkmer(refcontent+pos,KMERLENGTH,MAXBUCKETS)]->push_back(pos);
        //sw2.stop();

        sw.stop(false);
        //cout<<"Indexing speed: "<<((double)reflength/(1024*1024)/(sw.getDiff()/1000))<<" MB/s\n";
    }

    void findLongestMatch(char* cur, long &pos, long &length)
    {
        long maxlen=0;
        long maxlenpos=-1;

        long h=hashforkmer(cur,KMERLENGTH,MAXBUCKETS);

        //cout<<kmermap[h][0].size()<<"...vsize\n";
        for (vector<int>::const_iterator iterator = kmermap[h]->begin(), end = kmermap[h]->end(); iterator != end; ++iterator)
        {
            long matchlen=getMatchLength(cur, refcontent+*iterator);
            if (matchlen>maxlen)
            {
                maxlen=matchlen;
                maxlenpos=*iterator;
            }
        }
        length=maxlen;
        pos=maxlenpos;
    }

    void getMatchLengthInRef(char* s1, long refdelta, long &length)
    {
        length=getMatchLength(s1, refcontent+refdelta);
    }

//private:
    vector<vector<int>*> kmermap;
    long KMERLENGTH;

    long MAXBUCKETS;
};


#endif // KMERINDEX_H_INCLUDED
