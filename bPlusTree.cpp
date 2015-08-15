#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <limits>
#include <sstream>
#include <vector>
#include <glob.h>
#include <cmath>
#include <algorithm>

#ifdef WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#include <ctime>
#endif

using namespace std;

// const string ROOT_FILE_NAME = "root.txt";

/* 
    This assignment includes b-plus tree insertion, point query and range query.

        ------ Utsav Sinha, 12775
*/ 

const string ROOT_NODE_PATTERN = "root*";
const string LEAF_NODE_PATTERN = "leaf*";
const string INTERNAL_NODE_PATTERN = "internal*";


const string MAX_NUM_KEYS_FILE_NAME = "bplustree.config"; 
const string BPLUS_TREE_DUMP_FILE = "assgn2_bplus_data.txt";
const string QUERY_FILE = "querysample.txt";

const string QUERY_RESULTS_FILE = "query_results.txt";
const string QUERY_STATS_FILE = "query_stats.txt";

const string BUILD_RESULTS_FILE = "build_results.txt";
const string BUILD_STATS_FILE = "build_stats.txt";

const string REBUILD_BPLUS_TREE_DUMP_FILE = "bplus_dump_file.txt";

const string LEAF_NODE_MARKER = "leaf";
const string FILE_EXTENSION = ".txt";
const string INTERNAL_NODE_MARKER = "internal";
const string ROOT_NODE_MARKER = "root";

const int QUERY_INSERT = 0;
const int QUERY_POINT_SEARCH = 1;
const int QUERY_RANGE_SEARCH = 2;

const int BUILD_TREE_FROM_SCRATCH = 1;
const int RUN_QUERY_FROM_FILE = 2;
const int DIRECT_QUERY_ENTER = 3;
const int DIRECT_QUERY_EXIT = -1;

const int FAILURE = -1;
const int SUCCESS = 1;
const int REALLY_LEAF = 1;
const int INTERNAL_NODE = -1;

const double KEY_MIN = 0.0;    // the minimum value of key
const double KEY_MAX = 1.0;   // the maximum value of key

const double DOUBLE_MAX  = std::numeric_limits<double>::infinity();

int MAX_NUM_KEYS;   // the maximum number of keys a node in b-plus tree can have
string ROOT_FILE_NAME;

int NUM_LEAF_NODES;
int NUM_INTERNAL_NODES;

typedef unsigned long uint64;

uint64 GetTimeMs64();

void create_root_file();

string find( double answer_key, string &data, vector<string> &parent_stack );

vector<pair<double,string> > range_query( double answer_low, double answer_high );

int get_max_degree(string fileName);

int insert_in_bplus_tree( double insert_key, string insert_data, string &fileWhereInserted );

int insert_in_node( string insertFileName, double insert_key, string insert_data, int ifLeaf );

string get_new_file_name( string fileType );

int insert_in_parent( string firstFileName, double key_for_parent, string secondFileName, vector<string> parent_stack );

string get_node_type(string fileName);

vector<string> glob( string pat );

int rebuild_bplus_tree( string rootfile );

int build_bplus_tree_from_dump( string fileName );

void delete_files( vector<string> files_to_delete );

int run_query_from_file( string fileName );


class statObj
{

    public:

        double total_time;
        int num_query;
        double avg_time;

        double std_dev_time;
        
        double min_time;
        double max_time;
    
        int num_find_rows;  // number of rows found. Useful in case of range queries to sum up number of rows returned
        // also stores number of successful key finds in point_query

        int queryType;  // can be QUERY_INSERT, QUERY_POINT_SEARCH or QUERY_RANGE_SEARCH. It is used for printing purposes

    // public:

        statObj()
        {
            total_time = avg_time = min_time = max_time = std_dev_time = 0;
            num_query = 0;
            num_find_rows = 0;
            queryType = -1;
        }

        statObj(double tot, int num, double avg, double dev, double mini, double maxi, int findr, int q)
        {
            total_time = tot;
            num_query = num;
            avg_time = avg;
            std_dev_time = dev;
            min_time = mini;
            max_time = maxi;
            num_find_rows = findr;
            queryType = q;
        }

        void print_statObj()
        {
            if ( queryType == QUERY_INSERT )
            {
                cout << "Printing stats for bplus-tree insertions ... \n" << endl;
            }
            else if ( queryType == QUERY_POINT_SEARCH )
            {
                cout << "Printing stats for point search queries on bplus-tree ... \n" << endl;
            }
            else if ( queryType == QUERY_RANGE_SEARCH )
            {
                cout << "Printing stats for range search queries on bplus-tree ... \n" << endl;
            }
            else
            {
                cout << "Printing stats for unknown query type ...\n" << endl;
            }

            cout << "Total time: \t" << total_time <<  " ms" << endl;
            cout << "Number of Queries: \t" << num_query << endl;
            cout << "Average Time per query: \t" << avg_time << " ms" << endl;
            cout << "Minimum Time of any query: \t" << min_time << " ms" << endl;
            cout << "Maximum Time of any query: \t" << max_time << " ms" << endl;
            cout << "Standard Deviation: \t" << std_dev_time << " ms" << endl;

            if ( queryType == QUERY_POINT_SEARCH || queryType == QUERY_RANGE_SEARCH )
            {
                cout << "\n" << "Number of successful rows found in find operation: \t" << num_find_rows << endl;
                if ( num_find_rows > 0 )
                {
                    cout << "Average Time to find a row: " << total_time/(double)num_find_rows << " ms" << endl;
                }
            }

            cout << endl;
        }

};


statObj generate_stats( vector<double> time_vec, int num_find_rows, int queryType );

void print_stat_to_file( vector<statObj> all_statObj, string fileName );



int main()
{
    double key, key_low, key_high;
    double range_centre, range;
    int i, result;
    string data;
    int input;

    uint64 time_before_exec, time_after_exec;
    double time_diff;

    int num_insertions = 0, num_point_query = 0, num_range_query = 0;

    int num_point_query_rows, num_range_query_rows;
    num_point_query_rows = num_range_query_rows = 0;

    vector<double> insertion_time_vec, point_query_time_vec, range_query_time_vec;

    // clock_t clock_before_exec, clock_after_exec;
    // float clock_diff;

    // get_new_file_name(LEAF_NODE_MARKER);

    create_root_file();

    cout << "\nWelcome !" << " Please enter " << BUILD_TREE_FROM_SCRATCH << " to build tree from file " << BPLUS_TREE_DUMP_FILE << "\n" << endl;
        
    cout << "Enter " << RUN_QUERY_FROM_FILE << " to run queries from file " << QUERY_FILE << "\n" << endl;
    
    cout << "Enter " << DIRECT_QUERY_ENTER << " to enter a query loop" << endl;

    cin >> input;

    if ( input == BUILD_TREE_FROM_SCRATCH )
    {

        build_bplus_tree_from_dump( BPLUS_TREE_DUMP_FILE );
        cout << "Enter " << RUN_QUERY_FROM_FILE << " to run queries from file " << QUERY_FILE << "\n" << endl;
        cout << "Enter " << DIRECT_QUERY_ENTER << " to enter a query loop" << endl;
        cin >> input;
    }

    if ( input == RUN_QUERY_FROM_FILE )
    {
        run_query_from_file( QUERY_FILE );
    } 

    if ( input == DIRECT_QUERY_ENTER )
    {
        cout << "Enter " << QUERY_INSERT << " <key> " <<  " <data> " << " to insert key-data pair " << endl;
        cout << "Enter " << QUERY_POINT_SEARCH << " <key> " <<  " to search data of key <key> " << endl;
        cout << "Enter " << QUERY_RANGE_SEARCH << " <key center> " <<  " <key range> " << " to search data in a range of keys " << endl;


        cout << "\n" << "Enter " << DIRECT_QUERY_EXIT << " to exit query loop\n" << endl;

        cin >> input;

        while ( input != DIRECT_QUERY_EXIT )
        {

            if ( input == QUERY_INSERT )
            {
                num_insertions++;

                string fileWhereInserted;
                // cout << "Enter key data pair to insert" << endl;
                cin >> key >> data;
                // cout << "Input read is: " << "key: " << key << " and data " << data << endl;

                time_before_exec = GetTimeMs64();
                // clock_before_exec = clock();
                result = insert_in_bplus_tree(key, data, fileWhereInserted);
                // clock_after_exec = clock();
                time_after_exec = GetTimeMs64();

                time_diff = (double) ((double)time_after_exec - (double)time_before_exec);
                insertion_time_vec.push_back(time_diff);

                // cout << "Time before exec: " << time_before_exec << " and time after exec: " << time_after_exec << endl;
                cout << "Inserted " << key << " in " << fileWhereInserted << "\t(" << time_diff << "ms)" << endl;
                
                // clock_diff = ( (float)clock_after_exec - (float)clock_before_exec );
                // cout << "Clock before exec: " << clock_before_exec << " and clock after exec: " << clock_after_exec << endl;
                // cout << "Inserted " << key << " in " << fileWhereInserted << " in " << time_diff << " s" << endl;
                
            }
            else if ( input == QUERY_POINT_SEARCH )
            {
                num_point_query++;

                vector<string> parent_stack;
                string desired_file;

                cin >> key;
                
                time_before_exec = GetTimeMs64();
                desired_file = find(key, data, parent_stack);
                time_after_exec = GetTimeMs64();

                time_diff = (double) ((double)time_after_exec - (double)time_before_exec);
                point_query_time_vec.push_back(time_diff);

                if ( data.compare("") == 0 )
                {
                    cout << "Key " << key << " NOT FOUND" << endl;
                    cout << "Probable leaf to insert key is " << desired_file << "\t(" << time_diff << "ms)" << endl;
                }
                else
                {
                    num_point_query_rows++; // number of successful point query rows

                    cout << "Found Key: " << key << " Data: " << data << " found in " << desired_file << "\t(" << time_diff << "ms)" << endl;
                    cout << "\nTracing the path from the root to the leaf." << endl;
                    for ( i = 0; i < parent_stack.size(); i++ )
                    {
                        cout << i+1 << ": " << parent_stack[i] << endl;
                    }
                    cout << i+1 << ": " << desired_file << endl;
                }
            }
            else if ( input == QUERY_RANGE_SEARCH )
            {

                num_range_query++;
                vector<pair<double,string> > rangeQueryResult;

                cin >> range_centre >> range;
                

                key_low = range_centre - range;
                key_high = range_centre + range;

                time_before_exec = GetTimeMs64();
                rangeQueryResult = range_query(key_low, key_high);
                time_after_exec = GetTimeMs64();
        
                time_diff = (double) ((double)time_after_exec - (double)time_before_exec);
                range_query_time_vec.push_back(time_diff);

                num_range_query_rows += rangeQueryResult.size();

                for ( i = 0; i < rangeQueryResult.size(); i++ )
                {
                    cout << i+1 << ": " << rangeQueryResult[i].first << "\t" << rangeQueryResult[i].second << endl;
                }
                cout << rangeQueryResult.size() << " rows found between key " << key_low << " and " << key_high << "\t(" << time_diff << "ms)" << endl;
            }
            else
            {
                cout << "ERROR: " << "unknown query command " << input << endl;
                
                cout << endl;
                cout << "Enter " << QUERY_INSERT << " <key> " <<  " <data> " << " to insert key-data pair " << endl;
                cout << "Enter " << QUERY_POINT_SEARCH << " <key> " <<  " to search data of key <key> " << endl;
                cout << "Enter " << QUERY_RANGE_SEARCH << " <key center> " <<  " <key range> " << " to search data in a range of keys " << endl;

                cout << "\n" << "Enter " << DIRECT_QUERY_EXIT << " to exit query loop\n" << endl;
            }
            cin >> input;
        } 
     
        statObj insert_statObj, point_query_statObj, range_query_statObj;

        insert_statObj = generate_stats(insertion_time_vec, -1, QUERY_INSERT); // -1 since it is not a find query
        point_query_statObj = generate_stats(point_query_time_vec, num_point_query_rows, QUERY_POINT_SEARCH);
        range_query_statObj = generate_stats(range_query_time_vec, num_range_query_rows, QUERY_RANGE_SEARCH);

        insert_statObj.print_statObj();
        point_query_statObj.print_statObj();
        range_query_statObj.print_statObj();

    }

    cout << "\n Program Exiting...." << endl;
    return 0;
}


int insert_in_bplus_tree( double insert_key, string insert_data, string &fileWhereInserted )
{
    int num_keys;
    string insertFileName;
    string data, line;
    string dummy;
    string leaf_status;
    vector<string> parent_stack;

    // Checking for absurdities
    if ( insert_key < KEY_MIN || insert_key > KEY_MAX )
    {
        cout << "ERROR: " << "Trying to insert " << insert_key << " which is not between " << KEY_MIN << " and " << KEY_MAX << endl;
        return FAILURE;
    }

    // fstream rootfile;
    // rootfile.open (ROOT_FILE_NAME.c_str());

    // getline(rootfile, line);    // first line stores if it is a leaf or an internal node

    // getline(rootfile, line);
    // num_keys = atoi(line.c_str());

    insertFileName = find(insert_key, dummy, parent_stack);
    // cout << "FileName to insert " << insert_key << " is " << insertFileName << endl;

    fileWhereInserted = insertFileName;

    fstream insertfile;
    insertfile.open (insertFileName.c_str());

    getline(insertfile, line);    // first line stores if it is a leaf or an internal node
    leaf_status = line;

    getline(insertfile, line);
    num_keys = atoi(line.c_str());

    // cout << "num_keys: " << num_keys << endl;

    if ( num_keys < MAX_NUM_KEYS )
    {
        insert_in_node( insertFileName, insert_key, insert_data, REALLY_LEAF );
    }
    else    // the leaf node should be split
    {
        // cout << "Inside INSERT function. We need to split the leaf" << endl;

        // string parent_file_name;

        // getline(inserfile, line);
        // parent_file_name = line;

        string secondLeafFileName = get_new_file_name(LEAF_NODE_MARKER);

        // Now we need to allocate a block of memory to copy all data of insertfile into it.
        // Then we will add the insert_key along with its data to this block of memory in its correct soreted position.
        // Then we will divide this memory into 2 parts and write half back to insertfile
        // while the other half to secondFile.
        // The smallest key in secondfile would need to be inserted into the parent.

        int i;
        double key;
        string key_string;
        double key_for_parent;
        vector<pair<double,string> > fileBuffer;
        string next_file_name;

        getline(insertfile, line);
        next_file_name = line;

        getline(insertfile, line);
        stringstream linestream(line);
        std::getline(linestream, key_string, '\t');
        key = atof(key_string.c_str());
        linestream >> data;

        for ( i = 0; i < num_keys && insert_key > key; i++ )
        {

            fileBuffer.push_back( make_pair(key, data) );
            getline(insertfile, line);
            stringstream linestream(line);

            std::getline(linestream, key_string, '\t');
            key = atof(key_string.c_str());
            linestream >> data;            
        }

        // our insert_key should become the ith data entry in this block of memory
        fileBuffer.push_back( make_pair(insert_key, insert_data) );

        for ( i = i; i < num_keys; i++ )
        {
            fileBuffer.push_back( make_pair(key, data) );
            getline(insertfile, line);
            stringstream linestream(line);

            std::getline(linestream, key_string, '\t');
            key = atof(key_string.c_str());
            linestream >> data;            
        }

        insertfile.close();

        // Now we need to write MAX_NUM_KEYS/2 number of rows of fileBuffer along with 
        // leaf_status, num_keys and next_file_name into the same file i.e. insertfile

        // firstly, increase the number of keys in this leaf. This should become equal to 
        // MAX_NUM_KEYS+1 at this stage
        num_keys++;

        fstream afterinsertfile;
        afterinsertfile.open (insertFileName.c_str(), ios::out); // over-writting the previous file

        // cout << "Insertion in new file" << endl;

        afterinsertfile << leaf_status << endl;
        // cout << "leaf_status: " << leaf_status << endl;

        // the number of keys in this file is fileBuffer.size()/2
        afterinsertfile << fileBuffer.size()/2 << endl;
        // cout << "num_keys: " << fileBuffer.size()/2 << endl;

        // afterinsertfile << next_file_name << endl;
        // cout << "parent_file_name: " << parent_file_name << endl;

        // next file name of this leaf should be the name of the next leaf which would be generated
        // after this step to store the remaining fileBuffer
        afterinsertfile << secondLeafFileName << endl;
        // cout << "next_file_name: " << secondLeafFileName << endl;

        // at this stage, num_keys = MAX_NUM_KEYS+1
        // fileBuffer.size() = num_keys

        for ( i = 0; i < fileBuffer.size()/2; i++ )
        {
            afterinsertfile << fileBuffer[i].first << "\t" << fileBuffer[i].second << endl;
            // cout << "key: " << fileBuffer[i].first << " data: " << fileBuffer[i].second << endl;
        }
        
        afterinsertfile.close();


        // the key now residing in fileBuffer[i].first is the key that needs to be sent to the parent
        key_for_parent = fileBuffer[i].first;


        fstream secondLeafFile;
        secondLeafFile.open (secondLeafFileName.c_str(), ios::out); // over-writting the previous file

        // cout << "Insertion in second file" << endl;

        secondLeafFile << leaf_status << endl;
        // cout << "leaf_status: " << leaf_status << endl;

        secondLeafFile << fileBuffer.size()-fileBuffer.size()/2 << endl;
        // cout << "num_keys: " << fileBuffer.size()-fileBuffer.size()/2 << endl;

        // secondLeafFile << next_file_name << endl;
        // cout << "parent_file_name: " << parent_file_name << endl;

        secondLeafFile << next_file_name << endl;
        // cout << "next_file_name: " << next_file_name << endl;

        // at this stage, num_keys = MAX_NUM_KEYS+1
        // fileBuffer.size() = num_keys

        for ( i = i; i < fileBuffer.size(); i++ )
        {
            secondLeafFile << fileBuffer[i].first << "\t" << fileBuffer[i].second << endl;
            // cout << "key: " << fileBuffer[i].first << " data: " << fileBuffer[i].second << endl;
        }
        
        secondLeafFile.close();

        insert_in_parent(insertFileName, key_for_parent, secondLeafFileName, parent_stack);

    }

    return SUCCESS;
}


/*
    Takes as arguments:
    firstFileName:      the first file name of the split child nodes 
    secondFileName:     the second file name of the split child nodes
    key_for_parent:     the key that needs to be inserted into the parent
                        This key was the smallest key of the secondFile which 
                        was propogated up the tree for the parent to store.
    parent_stack:       the stack of parent file names that stores the path to the root file.
                        if this node also needs to be split, then it will pass the new key to
                        its parent and will use this stack to find out who its parent was.

    Returns SUCCESS or FAILURE as per the result of insertion operation.
*/

int insert_in_parent( string firstFileName, double key_for_parent, string secondFileName, vector<string> parent_stack )
{

    /*
        If firstFileName is the root of the tree, then create a new node R containing just one key,
        that is key_for_parent. Store firstFileName and secondFileName as the 2 "pointers" to the 
        left and right respectively.
        Make R the new root of the tree.
        Return.
    */

    int i;

    // cout << "Inside INSERT_IN_PARENT " << endl;
    // cout << "Split children are: " << firstFileName << " and " << secondFileName << endl;
    // cout << "Key to Insert in Parent is: " << key_for_parent << endl;
    // cout << "The parent_stack contains: " << endl;

    // for ( i = 0; i < parent_stack.size(); i++ )
    // {
    //     cout << i << ": " << parent_stack[i] << endl;
    // }
    // cout << endl;


    if ( parent_stack.size() == 0 ) // that is the parent stack has become empty, so we have reached the root
    {
        string newfirstFileName = get_new_file_name( get_node_type(firstFileName) );
        // now since firstFileName is root of the tree, we have to create a new root.
        // so we have to rename firstFileName to newfirstFileName
        // then we will open the ROOT_FILE_NAME or firstFileName and overwrite it with 
        // new contents as explained above.

        // rename( oldname , newname ) returns 0 if successful 

        if ( rename( firstFileName.c_str(), newfirstFileName.c_str() ) != 0 )
        {
            cout << "ERROR: " << "unable to rename " << firstFileName << " to " << newfirstFileName << " while insertion in parent node" << endl;
        }

        fstream rootfile;
        rootfile.open (firstFileName.c_str(), ios::out);
        rootfile << INTERNAL_NODE_MARKER << endl;   // since our parent has split children, so it has to be an internal node
        rootfile << "1" << endl;
        rootfile << newfirstFileName << endl;
        rootfile << key_for_parent << "\t" << secondFileName << endl;

        rootfile.close();
        return SUCCESS;
    }

    // Now we are sure that firstFileName is not the root of the bplus tree.

    string parent_file_name;

    parent_file_name = parent_stack.back(); // this is the parent of the firstFileName and the secondFileName
    parent_stack.pop_back();    // delete the last element of the parent stack


    string line, data, key_string, leaf_status;
    double key;
    int num_keys;

    fstream parentfile;
    parentfile.open (parent_file_name.c_str());

    getline(parentfile, line);    // first line stores if it is a leaf or an internal node
    leaf_status = line;

    getline(parentfile, line);
    num_keys = atoi(line.c_str());


    // if parent has less than MAX_NUM_KEYS, then insert the key_for_parent in the appropriate position
    // else we need to split the parent

    // cout << "Parent " << parent_file_name << " has " << num_keys << " keys " << endl;

    if ( num_keys < MAX_NUM_KEYS )
    {
        return insert_in_node( parent_file_name, key_for_parent, secondFileName, INTERNAL_NODE );;
    }

    // else    // now we need to split the parent
    // cout << "Inside INSERT_IN_PARENT function. We need to split the parent" << endl;

    // string parent_file_name;

    // getline(inserfile, line);
    // parent_file_name = line;

    string firstInternalFileName = parent_file_name;
    string secondInternalFileName = get_new_file_name(INTERNAL_NODE_MARKER);

    // Now we need to allocate a block of memory to copy all data of parentfile into it.
    // Then we will add the key_for_parent along with its data to this block of memory in its correct sorted position.
    // Then we will divide this memory into 2 parts and write half back to parentfile
    // while the other half to secondFile.
    // The smallest key in secondfile would need to be inserted into the parent of parentfile.


    double key_for_parents_parent;
    vector<pair<double,string> > fileBuffer;
    string next_file_name;

    getline(parentfile, line);
    next_file_name = line;

    getline(parentfile, line);
    stringstream linestream(line);
    std::getline(linestream, key_string, '\t');
    key = atof(key_string.c_str());
    linestream >> data;

    for ( i = 0; i < num_keys && key_for_parent > key; i++ )
    {

        fileBuffer.push_back( make_pair(key, data) );
        getline(parentfile, line);
        stringstream linestream(line);

        std::getline(linestream, key_string, '\t');
        key = atof(key_string.c_str());
        linestream >> data;            
    }

    // our key_for_parent should become the ith data entry in this block of memory
    fileBuffer.push_back( make_pair(key_for_parent, secondFileName) );

    for ( i = i; i < num_keys; i++ )
    {
        fileBuffer.push_back( make_pair(key, data) );
        getline(parentfile, line);
        stringstream linestream(line);

        std::getline(linestream, key_string, '\t');
        key = atof(key_string.c_str());
        linestream >> data;            
    }

    parentfile.close();

    // Now we need to write MAX_NUM_KEYS/2 number of rows of fileBuffer along with 
    // leaf_status, num_keys and next_file_name into the same file i.e. parentfile

    // firstly, increase the number of keys in this internal node. This should become equal to 
    // MAX_NUM_KEYS+1 at this stage
    num_keys++;

    fstream afterinsertfile;
    afterinsertfile.open (firstInternalFileName.c_str(), ios::out); // over-writting the previous file

    // cout << "Insertion in new file" << endl;

    afterinsertfile << leaf_status << endl;
    // cout << "leaf_status: " << leaf_status << endl;

    // the number of keys in this file is fileBuffer.size()/2
    afterinsertfile << fileBuffer.size()/2 << endl;
    // cout << "num_keys: " << fileBuffer.size()/2 << endl;

    // afterinsertfile << parent_file_name << endl;
    // cout << "parent_file_name: " << parent_file_name << endl;

    afterinsertfile << next_file_name << endl;
    // cout << "next_file_name: " << next_file_name << endl;

    // at this stage, num_keys = MAX_NUM_KEYS+1
    // fileBuffer.size() = num_keys

    for ( i = 0; i < fileBuffer.size()/2; i++ )
    {
        afterinsertfile << fileBuffer[i].first << "\t" << fileBuffer[i].second << endl;
        // cout << "key: " << fileBuffer[i].first << " data: " << fileBuffer[i].second << endl;
    }
    
    afterinsertfile.close();


    // the key now residing in fileBuffer[i].first is the key that needs to be sent to the parent
    key_for_parents_parent = fileBuffer[i].first;


    fstream secondInternalFile;
    secondInternalFile.open (secondInternalFileName.c_str(), ios::out); // over-writting the previous file

    // cout << "Insertion in second file" << endl;

    secondInternalFile << leaf_status << endl;
    // cout << "leaf_status: " << leaf_status << endl;

    // that -1 is there since the internal nodes form a b-tree like structure instead of a 
    // bplus tree like structure. By that, I mean that a bplus-tree node always store the 
    // pointer of its next sibling node while a b-tree does not.
    // Since in this bplus-tree implementation, the internal nodes need not store the pointer
    // to its next sibling internal node, so we need to remove that key_for_parents_parent 
    // from the secondInternalFile altogether and transfer it to its parent node
    secondInternalFile << fileBuffer.size()-fileBuffer.size()/2-1 << endl;
    // cout << "num_keys: " << fileBuffer.size()-fileBuffer.size()/2-1 << endl;

    // secondInternalFile << next_file_name << endl;
    // cout << "parent_file_name: " << parent_file_name << endl;

    // the next file name should be the file pointed to by key_for_parents_parent which is 
    // in fileBuffer[i].second
    secondInternalFile << fileBuffer[i].second << endl;
    // cout << "next_file_name: " << fileBuffer[i].second << endl;

    // at this stage, num_keys = MAX_NUM_KEYS+1
    // fileBuffer.size() = num_keys

    for ( i = i+1; i < fileBuffer.size(); i++ )
    {
        secondInternalFile << fileBuffer[i].first << "\t" << fileBuffer[i].second << endl;
        // cout << "key: " << fileBuffer[i].first << " data: " << fileBuffer[i].second << endl;
    }
    
    secondInternalFile.close();

    return insert_in_parent(firstInternalFileName, key_for_parents_parent, secondInternalFileName, parent_stack);

}


/*
    Inserts the key insert_key and its corresponding data insert_data into the file
    insertFileName at the appropraite sorted position.

    Returns SUCCESS or FAILURE of the insertion.
*/

int insert_in_node( string insertFileName, double insert_key, string insert_data, int ifLeaf )
{

    // cout << "Inside INSERT_IN_NODE " << endl;
    // cout << "To insert in file: " << insertFileName  << endl;
    // cout << "Key to Insert in Node is: " << insert_key << " with data " << insert_data << endl;
    // cout << "IsLeaf status is: " << ifLeaf << endl;



    int i, num_keys;
    double key;
    string data, key_string, line, leaf_status, next_file_name;
    // string parent_file_name;
    vector<pair<double,string> > fileBuffer;    // stores only the real key value pairs of the file and not meta data of a file

    fstream insertfile;
    insertfile.open (insertFileName.c_str());

    getline(insertfile, line);    // first line stores if it is a leaf or an internal node
    leaf_status = line;


    // If we called this function as if we are really inserting data into a leaf node, then we 
    // must check at this stage that the leaf status we read really points to a leaf node

    // Note that this function can also be called from insert_in_parent which inserts a 
    // key into a parent node. In that case, we may be inserting in an internal node
    // so the following check can be skipped

    if ( ifLeaf == REALLY_LEAF )
    {
        if ( line.compare(LEAF_NODE_MARKER) != 0 )
        {
            cout << "ERROR: trying to insert in file " << insertFileName << " which is not a leaf node" << endl;
            return FAILURE;
        }
    }
    

    getline(insertfile, line);
    num_keys = atoi(line.c_str());
    
    if ( num_keys >= MAX_NUM_KEYS )
    {
        cout << "ERROR: trying to insert in file " << insertFileName << " which is already full" << endl;
        return FAILURE;
    }

    // getline(insertfile, line);
    // parent_file_name = line;

    getline(insertfile, line);
    next_file_name = line;

    getline(insertfile, line);
    stringstream linestream(line);

    std::getline(linestream, key_string, '\t');
    key = atof(key_string.c_str());
    linestream >> data;

    for ( i = 0; i < num_keys && insert_key > key; i++ )
    {

        fileBuffer.push_back( make_pair(key, data) );
        getline(insertfile, line);
        stringstream linestream(line);

        std::getline(linestream, key_string, '\t');
        key = atof(key_string.c_str());
        linestream >> data;            
    }

    // our insert_key should become the ith data entry in this leaf node
    fileBuffer.push_back( make_pair(insert_key, insert_data) );

    for ( i = i; i < num_keys; i++ )
    {

        fileBuffer.push_back( make_pair(key, data) );
        getline(insertfile, line);
        stringstream linestream(line);

        std::getline(linestream, key_string, '\t');
        key = atof(key_string.c_str());
        linestream >> data;            
    }
    // fileBuffer.push_back( make_pair(key, data) );   // for the last line read

    insertfile.close();

    // Now we need to write this entire fileBuffer along with leaf_status, num_keys and next_file_name
    // into the same file i.e. insertFileName

    // firstly, increase the number of keys in this leaf
    num_keys++;

    fstream afterinsertfile;
    afterinsertfile.open (insertFileName.c_str(), ios::out); // over-writting the previous file

    // cout << "Insertion in new file" << endl;

    afterinsertfile << leaf_status << endl;
    // cout << "leaf_status: " << leaf_status << endl;

    afterinsertfile << num_keys << endl;
    // cout << "num_keys: " << num_keys << endl;

    // afterinsertfile << next_file_name << endl;
    // cout << "parent_file_name: " << parent_file_name << endl;

    afterinsertfile << next_file_name << endl;
    // cout << "next_file_name: " << next_file_name << endl;

    for ( i = 0; i < fileBuffer.size(); i++ )
    {
        afterinsertfile << fileBuffer[i].first << "\t" << fileBuffer[i].second << endl;
        // cout << "key: " << fileBuffer[i].first << " data: " << fileBuffer[i].second << endl;
    }
    

    afterinsertfile.close();

    return SUCCESS;
}


/* 
    Returns a vector of key, data pairs that have keys in the range answer_low to answer_high
 */

vector<pair<double,string> > range_query( double answer_low, double answer_high )
{
    // cout << "Range Query Results: ";

    vector<pair<double,string> > answer;// = new vector<pair<double,string> >();
    vector<string> parent_stack;

    if ( answer_high < KEY_MIN || answer_low > KEY_MAX )
    {
        // cout << "0 rows " << endl;
        return answer;
    }

    // Additional checks for absurdity
    if ( answer_high < answer_low )
    {
        // cout << "0 rows " << endl;
        return answer;
    }

    int num_rows = 0;
    string low_file, low_data;
    low_file = find(answer_low, low_data, parent_stack);

    /*
        Now read this file, go to the point where we start finding keys >= answer_low
        follow pointers along that leaf file to add key, data pairs to vector
        until key becomes > answer_high or all leaf files are read

        Now 5 cases are there since our keys in database are always between KEY_MIN and KEY_MAX:
        
        answer_high <  KEY_MIN                               : Remove this case in pre-processing itself as it will always be null
        answer_low  <  KEY_MIN and answer_high <= KEY_MAX    : We will land up in first leaf file to find answer_low and then start adding to vector as usual
        answer_low  >= KEY_MIN and answer_high <= KEY_MAX    : Normal case, no special care 
        answer_low  >= KEY_MIN and answer_high > KEY_MAX     : We read from low_file and follow pointers till all leaf files are read
        answer_low  >  KEY_MAX                                : Remove this case in pre-processing itself as it will always be null
    */

    string data, line, key_string;
    double key;
    int i, num_keys;
    string fileName = low_file;
    bool done = false;

    while( done == false )
    {
        fstream datafile;
        datafile.open(fileName.c_str());
        getline(datafile, line);    // line containing leaf/internal node meta data


        // cout << endl;
        // cout << "Reading file " << fileName << endl;
        getline(datafile, line);
        num_keys = atoi(line.c_str());
        // cout << "num_keys " << num_keys << endl;

        // getline(datafile, line);
        // this line contains the name of the parent node file

        getline(datafile, line);    
        // since this is a leaf file, this line contains file name of the next leaf node to the right,
        //  i.e. the one that stores keys greater than this file
        data = line;
        fileName = data;

        getline(datafile, line);    // pinging the first data line to get its key for comparison
        stringstream linestream(line);

        std::getline(linestream, key_string, '\t');
        key = atof(key_string.c_str());
        linestream >> data;

        for ( i = 0; i < num_keys && answer_low > key; i++ )
        {
            // cout << "SKIPPING " << "key: " << key << " data: " << data << endl;

            getline(datafile, line);
            stringstream linestream(line);

            std::getline(linestream, key_string, '\t');
            key = atof(key_string.c_str());
            linestream >> data;            
        }
        // Thw above for loop is meaningful in first iteration only when fileName is low_file
        // after coming out of the previous for loop, we have skipped all values < answer_low
        // now key >= answer_low

        for ( i = i; i < num_keys && answer_high >= key; i++ )
        {
            num_rows++;
            // cout << "key: " << key << " data: " << data << endl;
            answer.push_back(make_pair(key, data));
            getline(datafile, line);
            stringstream linestream(line);

            std::getline(linestream, key_string, '\t');
            key = atof(key_string.c_str());
            linestream >> data;
        }
        // cout << "answer_high: " << answer_high << " key " << key <<  " i is: " << i << " num_keys: " << num_keys << " data: " << data  << " fileName: " << fileName << endl;

        // this "i" is the smallest number such that answer_key <= key
        if ( answer_high >= key )
        {
            // we have to read the neighbouring file to the right now
            if ( fileName.compare("") == 0 )    // that is, there is a no leaf file to the right
            {
                done = true;
            }
            else
            {
                // cout << endl;
                // cout << "fileName jumping to: " << fileName << endl;
            }
            
        }
        else
        {
            done = true;
        }
    }
    // cout << "Total " << num_rows << " rows " << endl;
    return answer;
}



/* 
    Returns that leaf node i.e. file name (string) corresponding to where the answer_key is found.
    It also stores the data corresponding to the answer_key in pointer *data.

    The path from the root to the leaf node is stored in the parent_stack to be used to trace
    back the path to the root while insertion.

    If answer_key is not found it returns empty string "" as answer_data. 
    But it returns the file name where the answer_key should be inserted 
    if the answer_key was found.
    This filename would later be used for insertion purposes.
 */

string find( double answer_key, string &answer_data, vector<string> &parent_stack )
{
    int i, num_keys;
    double key;
    string line, key_string, data, data_prev;
    string fileName (ROOT_FILE_NAME.c_str());

    fstream rootfile;
    rootfile.open (ROOT_FILE_NAME.c_str());

    bool isLeaf = false;
    getline(rootfile, line);    // first line stores if it is a leaf or an internal node

    // parent_stack.push_back(fileName);   // pushing the rootfilename, but there is no need!

    if ( line.compare(LEAF_NODE_MARKER) == 0 )
        isLeaf = true;
    
    // isLeaf = false;
    while( isLeaf == false )
    {
        fstream datafile;
        datafile.open(fileName.c_str());
        getline(datafile, line);
        if ( line.compare(LEAF_NODE_MARKER) == 0 )
        {
            isLeaf = true;
            break;
        }

        parent_stack.push_back(fileName);

        // cout << endl;
        // cout << "Reading file " << fileName << endl;
        getline(datafile, line);
        num_keys = atoi(line.c_str());
        // cout << "num_keys " << num_keys << endl;

        // getline(datafile, line);
        // This line contains the name of the parent node file.

        getline(datafile, line);    
        // this line contains the link to the node that stores keys less than the smallest key stored in this file
        // But for leaf nodes, this line contains file name of the next leaf node to the right, i.e. the one that stores keys greater than this file
        data = line;
        data_prev = data;

        key = KEY_MIN;
        for ( i = 0; i < num_keys && answer_key > key; i++ )
        {
            getline(datafile, line);
            stringstream linestream(line);

            std::getline(linestream, key_string, '\t');
            key = atof(key_string.c_str());
            data_prev = data;
            linestream >> data;
            // cout << "key_string: " << key_string << " its double is " << key << endl;
            // cout << "key: " << key << " data: " << data << endl;
        }
        // cout << "answer_key: " << answer_key << " key " << key <<  " i is: " << i << " num_keys: " << num_keys << " data: " << data << " data_prev " << data_prev << " fileName: " << fileName << endl;

        // this "i" is the smallest number such that answer_key <= key
        if ( answer_key >= key )
        {
            // getline(datafile, line);
            // fileName = line;
            fileName = data;
        }
        else
        {
            fileName = data_prev;
        }
        // cout << endl;
        // cout << "fileName jumping to: " << fileName << endl;
    }

    // cout << "desired key is in file " << fileName << endl;
    // Now, fileName is a leaf node file.
    fstream datafile;
    datafile.open(fileName.c_str());
    getline(datafile, line);
    getline(datafile, line);
    num_keys = atoi(line.c_str());
    // cout << "num_keys " << num_keys << endl;

    // getline(datafile, line);
    // this line stores the name of the parent file name.

    getline(datafile, line);    
    // this line contains link to neighbouring left leaf file (the leaf file which stores key less than this file)
    data = line;

    key = KEY_MIN;
    for ( i = 0; i < num_keys && answer_key > key; i++ )
    {
        getline(datafile, line);
        stringstream linestream(line);

        std::getline(linestream, key_string, '\t');
        key = atof(key_string.c_str());
        linestream >> data;
        // cout << "key_string: " << key_string << " its double is " << key << endl;
        // cout << "key: " << key << " data: " << data << endl;
    }
    if ( answer_key == key )
    {
        answer_data = data;
        // cout << "found key" << endl;
    }
    else
    {
        answer_data = "";
        // cout << "did not find key" << endl;
    }

    return fileName;
}


/*
    Reads the file that contains the maximum number of keys that the bplus-tree can have
    and returns that number
*/

int get_max_degree(string fileName)
{
    int num_keys;
    string line;

    if ( std::ifstream(fileName.c_str()) )
    {
        fstream configfile;
        configfile.open (fileName.c_str());
    
        getline(configfile, line); 
        num_keys = atoi(line.c_str());
        return num_keys;
    }

    cout << fileName << " does not exist." << endl;
    return FAILURE;  // the file fileName does not exist 
}


/*
    Returns a new file name. fileType can be LEAF_NODE_MARKER or INTERNAL_NODE_MARKER
*/

string get_new_file_name( string fileType )
{
    int i;
    int counter = 0, max_counter = 0;
    string fileName;
    bool done = false;

    if ( fileType == LEAF_NODE_MARKER )
    {
        // that is the program has already done glob below once before 
        // and stored the maximum value of leaf counter in NUM_LEAF_NODES.
        // so we just need to increment NUM_LEAF_NODES and return its value
        if ( NUM_LEAF_NODES > 0 ) 
        {
            NUM_LEAF_NODES++;
            max_counter = NUM_LEAF_NODES;
            done = true;
        }
    }
    else if ( fileType == INTERNAL_NODE_MARKER )
    {
        if ( NUM_INTERNAL_NODES > 0 ) 
        {
            NUM_INTERNAL_NODES++;
            max_counter = NUM_INTERNAL_NODES;
            done = true;
        }
    }

    if ( done == false )    
    {
        // so we are not done finding the value of the next file counter
        // so do glob or equivalent to find this max counter

        // cout << "Doing glob to find file name " << " of type " << fileType << endl;

        // The following method pings each file sequentially to find the next non existing file
        // But it is inefficient as the number of leaf nodes increase in number
        // as for say to create a leaf node after 4000 leaves already exist, we check if leaf1, 
        // leaf2, leaf3 .... leaf3999, leaf4000 exist or not.
        // This is very time and resource consuming

        /*
        while ( done == false )
        {
            stringstream sstm;
            sstm << fileType << counter << FILE_EXTENSION;
            fileName = sstm.str();

            if (std::ifstream(fileName.c_str()))
            {
                 // this leaf file already exists, try a new one
                counter++;
            }
            else
            {
                done = true;
            }
        }
        */

        vector<string> possibleFileNames;

        string file_node_pattern = fileType + "*";  // the pattern to search for glob is fileType followed by regex *
        string filename_without_extension;
        string counter_string;

        possibleFileNames = glob( file_node_pattern );

        for ( i = 0; i < possibleFileNames.size(); i++ )
        {
            stringstream linestream(possibleFileNames[i]);

            std::getline(linestream, filename_without_extension, '.');

            counter_string = filename_without_extension.substr ( fileType.size() ); // reading the number from the filename. skipping past fileType.size() characters

            counter = atoi(counter_string.c_str());
            
            if ( max_counter < counter )
            {
                max_counter = counter;
            }

            // cout << "File: " << possibleFileNames[i] << " File name without extension: " << filename_without_extension << " with counter_string " << counter_string << " and counter " << counter << endl;
        }

        max_counter++;

        if ( fileType == LEAF_NODE_MARKER )
        {
            NUM_LEAF_NODES = max_counter;
        }
        else if ( fileType == INTERNAL_NODE_MARKER )
        {
            NUM_INTERNAL_NODES = max_counter;
        }
    }
    
    stringstream sstm;
    sstm << fileType << max_counter << FILE_EXTENSION;
    fileName = sstm.str();

    // cout << "filename created " << fileName << endl;

    return fileName;
}

/*
    Takes a file and returns the node type of the file, i.e whether it is an internal
    node or a leaf node.
    Returns LEAF_NODE_MARKER if it is a leaf node and INTERNAL_NODE_MARKER if it is 
    an internal node.
*/

string get_node_type(string fileName)
{
    string line;
    if ( std::ifstream(fileName.c_str()) )
    {
        fstream nodefile;
        nodefile.open (fileName.c_str());
    
        getline(nodefile, line); 
        return line;    // the first line itself contains if it is a leaf node or an internal node
    }

    cout << "ERROR: " << "node type checking failed." << endl;
    cout << fileName << " does not exist." << endl;

}



/* 
    Returns as a vector of strings all file names that match the string pat

    Courtesy: http://stackoverflow.com/questions/8401777/simple-glob-in-c-on-unix-system
*/

vector<string> glob( string pat )
{
    glob_t glob_result;
    glob(pat.c_str(), GLOB_TILDE, NULL, &glob_result);
    vector<string> ret;
    for(unsigned int i = 0; i < glob_result.gl_pathc; i++ )
    {
        ret.push_back(string(glob_result.gl_pathv[i]));
    }
    globfree(&glob_result);
    return ret;
}


/* 
    Checks if the root node of bplus-tree exists. If it does not, it creates that file
    and enter the default values into the file.

    It also checks that the MAX_NUM_KEYS of the bplus tree has not changed.
    If it has, then it creates a dump of the database in REBUILD_BPLUS_TREE_DUMP_FILE 
    and then rebuilds the bplus tree using the new value of MAX_NUM_KEYS
*/

void create_root_file()
{
    MAX_NUM_KEYS = get_max_degree(MAX_NUM_KEYS_FILE_NAME);
    int max_degree = MAX_NUM_KEYS;
    stringstream sstm;
    sstm << ROOT_NODE_MARKER << max_degree << FILE_EXTENSION;
    string fileName = sstm.str();

    ROOT_FILE_NAME = fileName;

    glob_t glob_result;
    vector<string> possibleRoots;
    int i;

    if (std::ifstream(fileName.c_str()))
    {
         cout << "Root File " << fileName << " already exists. Opening it to read." << endl;
         return;
    }

    possibleRoots = glob(ROOT_NODE_PATTERN.c_str());

    // This step is to check that if someone has cunningly changed the value of 
    // MAX_NUM_KEYS in MAX_NUM_KEYS_FILE_NAME after the tree was already built.
    // In that case, we have to rebuild the tree. 

    // So to rebuild the tree, we need to delete all previous internal and leaf
    // nodes and create fresh nodes. But we do need to preserve the data which was
    // previously stored in the leaf nodes.
    // so we create a copy of it in REBUILD_BPLUS_TREE_DUMP_FILE

    if ( possibleRoots.size() > 0 )
    {
        cout << "WARNING: " << "Rootfile " << fileName << " does not exist. " << endl;
        cout << "Someone changed the value of Maximum number of keys in " << MAX_NUM_KEYS_FILE_NAME << endl;
        cout << "Possible root file(s) can be: " << endl;
        for ( i = 0; i < possibleRoots.size(); i++ )
        {
            cout << i+1 << ": " << possibleRoots[i] << endl;
        }
        cout << endl;
        cout << "Assuming " << possibleRoots[0] << " to be the previous root file node " << endl;
        cout << "Ignoring rest of these possible rootfiles and rebuilding the tree." << endl;
        cout << "The rebuilt tree will have data from previous tree." << endl;
        rebuild_bplus_tree(possibleRoots[0]);
        return;
    }
    else
    {
         cout << "Creating Root File " << fileName << endl;

        std::ofstream file(fileName.c_str());
        file << LEAF_NODE_MARKER << endl;
        file << "0" << endl;

        return;
    }
}


/*
    This function builds the bplus-tree from scratch. It opens file fileName
    and uses it as the database dump to create the bplus-tree.

    It returns SUCCESS or FAILURE as per success of the operation
*/

int build_bplus_tree_from_dump( string fileName )
{
    int i;
    int returnValue = SUCCESS;

    cout << "Building B-plus Tree from dump file " << fileName << endl;
    cout << "Deleting all existing leaf nodes, internal nodes and root node files..." << endl;

    vector<string> possibleRoots;
    vector<string> possibleInternalNodes;
    vector<string> possibleLeaves;

    possibleRoots = glob(ROOT_NODE_PATTERN.c_str());
    // cout << "Possible roots ... " << endl;
    // for ( i = 0; i < possibleRoots.size(); i++ )
    // {
    //     cout << i+1 << ": " << possibleRoots[i] << endl;
    // }
    delete_files(possibleRoots);

    possibleInternalNodes = glob(INTERNAL_NODE_PATTERN.c_str());
    // cout << "Possible internal nodes ... " << endl;
    // for ( i = 0; i < possibleInternalNodes.size(); i++ )
    // {
    //     cout << i+1 << ": " << possibleInternalNodes[i] << endl;
    // }
    delete_files(possibleInternalNodes);

    possibleLeaves = glob(LEAF_NODE_PATTERN.c_str());
    // cout << "Possible leaves ... " << endl;
    // for ( i = 0; i < possibleLeaves.size(); i++ )
    // {
    //     cout << i+1 << ": " << possibleLeaves[i] << endl;
    // }
    delete_files(possibleLeaves);

    create_root_file();

    int num_keys;
    string line, key_string, data;
    double key;
    int result, counter = 0;
    

    if ( !std::ifstream(fileName.c_str()) )
    {
        cout << "ERROR: " << "Building bplus-tree failed." << fileName << " does not exist." << endl;
        return FAILURE;
    }

    vector<double> insertion_time_vec;
    double time_diff, time_before_exec, time_after_exec;


    // creating the build result file
    std::ofstream build_result_file(BUILD_RESULTS_FILE.c_str());


    fstream dumpfile;
    dumpfile.open (fileName.c_str());

    while( getline(dumpfile, line) )
    {
        time_before_exec = GetTimeMs64();

        counter++;
        stringstream linestream(line);

        std::getline(linestream, key_string, '\t');
        key = atof(key_string.c_str());
        linestream >> data;

        string fileWhereInserted;

        // cout << "Key: " << key << " Data: " << data << endl;

        // Now we have key, data pair. We should insert this into our bplus-tree

        
        result = insert_in_bplus_tree(key, data, fileWhereInserted);
        time_after_exec = GetTimeMs64();

        time_diff = (double)( (double)time_after_exec - (double)time_before_exec );
        insertion_time_vec.push_back(time_diff);

        if ( result == FAILURE )
        {
            build_result_file << "ERROR: " << "insertion in bplus-tree failed at line " << counter << " of " << fileName << endl;
            
            cout << "ERROR: " << "insertion in bplus-tree failed at line " << counter << " of " << fileName << endl;
            returnValue = FAILURE;
        }
        else
        {
            build_result_file << "Key: " << key << " Data: " << data << " successfully added in " << fileWhereInserted << " (" << time_diff << " ms)" << endl;

            cout << "Key: " << key << " Data: " << data << " successfully added in " << fileWhereInserted << " (" << time_diff << " ms)" << endl;
        }
    } 

    build_result_file.close();

    statObj insert_statObj;
    insert_statObj = generate_stats(insertion_time_vec, -1, QUERY_INSERT );
    insert_statObj.print_statObj();

    vector<statObj> all_statObj;
    all_statObj.push_back(insert_statObj);

    print_stat_to_file(all_statObj, BUILD_STATS_FILE);

    cout << "Building tree from " << fileName << " completed\n" << endl;
    cout << "Results are in " << BUILD_RESULTS_FILE << endl;
    cout << "Statistics are in " << BUILD_STATS_FILE << endl;

    return returnValue;

}

/*
    This takes a list of files as arguments and deletes all those files in the current directory
*/

void delete_files( vector<string> files_to_delete )
{
    int i;
    for ( i = 0; i < files_to_delete.size(); i++ )
    {
        if( remove( files_to_delete[i].c_str() ) != 0 )
        {
            cout << "ERROR: " << files_to_delete[i] << " cannot be deleted " << endl;
        }
        else
        {
            // cout << i+1 << ": " << files_to_delete[i] << " deleted " << endl;
        }
    }

}

/*
    This function is used to rebuild the bplus-tree from the dump file. This function is
    only called when we suspect that someone has changed MAX_NUM_KEYS.

    A possible rootfile is given to it as argument. This function uses that rootfile and
    traverses the entire bplus-tree while storing all its key data pairs in REBUILD_BPLUS_TREE_DUMP_FILE

    Then it calls build_bplus_tree_from_dump() on this dump_file where the new value
    of MAX_NUM_KEYS would be used to create the index

    Returns SUCCESS or FAILURE as per the success of the operation
*/  

int rebuild_bplus_tree( string rootFileName )
{
    vector< pair<double,string> > dbDumpBuffer;

    int i;

    if ( !std::ifstream(rootFileName.c_str()) )
    {
        cout << "ERROR: " << "Rebuilding bplus-tree failed." << rootFileName << " does not exist." << endl;
        return FAILURE;
    }

    // override the ROOT_FILE_NAME with this rootFileName for find() to get this rootFileName

    ROOT_FILE_NAME = rootFileName;

    cout << "Rebuilding bplus-tree ....." << endl;
    
    dbDumpBuffer = range_query(KEY_MIN, KEY_MAX);

    cout << "Dumping database to file " << REBUILD_BPLUS_TREE_DUMP_FILE << "...." << endl;

    fstream dumpfile;
    dumpfile.open (REBUILD_BPLUS_TREE_DUMP_FILE.c_str(), ios::out);

    for ( i = 0; i < dbDumpBuffer.size(); i++ )
    {
        dumpfile << dbDumpBuffer[i].first << "\t" << dbDumpBuffer[i].second << endl;
    }
    dumpfile.close();

    // Now, we have written our entire database to a dump-file.
    // Now rebuilding the database with the new value of MAX_NUM_KEYS

    return build_bplus_tree_from_dump( REBUILD_BPLUS_TREE_DUMP_FILE );
}



/*
    This function runs queries from file fileName and stores the results in QUERY_RESULTS_FILE
    and the associated timing statistics in QUERY_STATS_FILE
    */

int run_query_from_file( string fileName )
{

    if (!std::ifstream(fileName.c_str()))
    {
         cout << "ERROR: " << "Query File " << fileName << " does not exist" << endl;
         return FAILURE;
    }

    vector<double> insertion_time_vec;
    vector<double> point_query_time_vec;
    vector<double> range_query_time_vec;

    // creating the query result file
    std::ofstream query_result_file(QUERY_RESULTS_FILE.c_str());


    string data, key_string, line, query_type_string;
    string range_centre_string, range_string;
    double key_low, key_high;
    double key, range_centre, range;
    int counter = 0, result, returnValue = SUCCESS;
    int queryType, i;
    string point_search_file_name;

    int num_insertions, num_point_query, num_range_query;
    int num_range_query_rows = 0;   // the number of rows returned by range query
    // note that simply summing range query timings is absurd since one range query
    // can be from KEY_MIN to KEY_MAX while another can be having a very small range
    // So returning all rows is incomparable

    int num_point_query_rows = 0;   // number of successful key finds

    double time_diff, time_before_exec, time_after_exec;

    num_insertions = num_point_query = num_range_query = 0;

    fstream queryfile;
    queryfile.open (fileName.c_str());

    while( getline(queryfile, line) )
    {
        counter++;
        
        query_result_file << "\nQuery #" << counter << " : " << line << endl;
        
        cout << "\nQuery #" << counter << " : " << line << endl;

        // We have to report timing from reading of a query to solving it (excluding printing time)
        // We habve read the query in while loop, so starting the clock

        stringstream linestream(line);
        time_before_exec = GetTimeMs64();

        std::getline(linestream, query_type_string, '\t');
        queryType = atoi(query_type_string.c_str());

        if ( queryType == QUERY_INSERT )
        {
            num_insertions++;
            std::getline(linestream, key_string, '\t');
            key = atof(key_string.c_str());
            linestream >> data;

            string fileWhereInserted;

            // time_before_exec = GetTimeMs64();
            result = insert_in_bplus_tree(key, data, fileWhereInserted);
            time_after_exec = GetTimeMs64();    // stopping the clock since computation is done

            time_diff = (double)( (double)time_after_exec - (double)time_before_exec );
            insertion_time_vec.push_back(time_diff);

            if ( result == FAILURE )
            {
                query_result_file << "ERROR: " << "insertion in bplus-tree failed at line " << counter << " of " << fileName << endl;
                
                cout << "ERROR: " << "insertion in bplus-tree failed at line " << counter << " of " << fileName << endl;
                returnValue = FAILURE;
            }
            else
            {
                query_result_file << "Key: " << key << " Data: " << data << " successfully added in " << fileWhereInserted << " (" << time_diff << " ms)" << endl;

                cout << "Key: " << key << " Data: " << data << " successfully added in " << fileWhereInserted << " (" << time_diff << " ms)" << endl;
            }
        }
        else if ( queryType == QUERY_POINT_SEARCH )
        {
            num_point_query++;

            linestream >> key_string;
            key = atof(key_string.c_str());
            vector<string> parent_stack;    // always declare a new parent stack to delete the old one automatically

            // time_before_exec = GetTimeMs64();
            point_search_file_name = find(key, data, parent_stack);
            time_after_exec = GetTimeMs64();    // stopping the clock since computation is done

            time_diff = (double)( (double)time_after_exec - (double)time_before_exec );
            point_query_time_vec.push_back(time_diff);

            if ( data.compare("") != 0 )    // so some data has been found for this key
            {
                query_result_file << "Key: " << key << " has Data: " << data << " found in " << point_search_file_name << " (" << time_diff << " ms)" << endl;
                
                cout << "Key: " << key << " has Data: " << data << " found in " << point_search_file_name << " (" << time_diff << " ms)" << endl;
            }
            else
            {
                num_point_query_rows++;

                query_result_file << "Key: " << key << " not found. ";
                query_result_file << "Possible file to insert this key: "<< point_search_file_name << " (" << time_diff << " ms)" << endl;

                cout << "Key: " << key << " not found. ";
                cout << "Possible file to insert this key: "<< point_search_file_name << " (" << time_diff << " ms)" << endl;
            }
        }
        else if ( queryType == QUERY_RANGE_SEARCH )
        {
            num_range_query++;

            std::getline(linestream, range_centre_string, '\t');
            range_centre = atof(range_centre_string.c_str());
            linestream >> range_string;
            range = atof(range_string.c_str());

            key_low = range_centre - range;
            key_high = range_centre + range;

            vector< pair<double, string> > rangeQueryResult; // always declare a new rangeQueryResult to delete the old one automatically
            
            // time_before_exec = GetTimeMs64();
            rangeQueryResult = range_query(key_low, key_high);
            time_after_exec = GetTimeMs64();    // stopping the clock since computation is done

            num_range_query_rows += rangeQueryResult.size();

            time_diff = (double)( (double)time_after_exec - (double)time_before_exec );
            range_query_time_vec.push_back(time_diff);

            for ( i = 0; i < rangeQueryResult.size(); i++ )
            {
                query_result_file << i+1 << ": " << rangeQueryResult[i].first << "\t" << rangeQueryResult[i].second << endl;

                cout << i+1 << ": "  << rangeQueryResult[i].first << "\t" << rangeQueryResult[i].second << endl;
            }
            query_result_file << rangeQueryResult.size() << " rows found between key " << key_low << " and " << key_high << " (" << time_diff << " ms)" << endl;

            cout << rangeQueryResult.size() << " rows found between key " << key_low << " and " << key_high << " (" << time_diff << " ms)" << endl;
        
        }
        else
        {
            query_result_file << "ERROR: " << "unknown query type " << queryType << " at line " << counter << " of " << fileName << endl;

            cout << "ERROR: " << "unknown query type " << queryType << " at line " << counter << " of " << fileName << endl;
        }
    }

    query_result_file << "\n" << "Entire " << fileName << " processed." << endl;
    query_result_file.close();

    cout << "\n" << "Entire " << fileName << " processed." << endl;
    cout << "Results are in " << QUERY_RESULTS_FILE << endl;
    cout << "Statistics are in " << QUERY_STATS_FILE << endl;


    statObj insert_statObj, point_query_statObj, range_query_statObj;

    insert_statObj = generate_stats(insertion_time_vec, -1, QUERY_INSERT );    // -1 since it is not a find operation
    point_query_statObj = generate_stats(point_query_time_vec, num_point_query_rows, QUERY_POINT_SEARCH);
    range_query_statObj = generate_stats(range_query_time_vec, num_range_query_rows, QUERY_RANGE_SEARCH);

    insert_statObj.print_statObj();
    
    point_query_statObj.print_statObj();

    range_query_statObj.print_statObj();

    vector<statObj> all_statObj;
    all_statObj.push_back(insert_statObj);
    all_statObj.push_back(point_query_statObj);
    all_statObj.push_back(range_query_statObj);

    print_stat_to_file(all_statObj, QUERY_STATS_FILE);

    return returnValue;
}


/* Returns the amount of milliseconds elapsed since the UNIX epoch. Works on both
   windows and linux. 

// Courtesy: http://stackoverflow.com/questions/1861294/how-to-calculate-execution-time-of-a-code-snippet-in-c

*/

uint64 GetTimeMs64()
{
    #ifdef _WIN32
     /* Windows */
         FILETIME ft;
         LARGE_INTEGER li;

         /* Get the amount of 100 nano seconds intervals elapsed since January 1, 1601 (UTC) and copy it
          * to a LARGE_INTEGER structure. */
         GetSystemTimeAsFileTime(&ft);
         li.LowPart = ft.dwLowDateTime;
         li.HighPart = ft.dwHighDateTime;

         uint64 ret = li.QuadPart;
         ret -= 116444736000000000LL; /* Convert from file time to UNIX epoch time. */
         ret /= 10000; /* From 100 nano seconds (10^-7) to 1 millisecond (10^-3) intervals */

         return ret;
    #else
         /* Linux */
         struct timeval tv;

         gettimeofday(&tv, NULL);

         uint64 ret = tv.tv_usec;
         /* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
         ret /= 1000;

         /* Adds the seconds (10^0) after converting them to milliseconds (10^-3) */
         ret += (tv.tv_sec * 1000);

         return ret;
    #endif
}


/*
    This function takes as argument the vector of timings, the query type and number of 
    rows which where successfully fetched from the database while query.

    It then generates a statObj and returns it.
*/

statObj generate_stats( vector<double> time_vec, int num_find_rows, int queryType )
{
    int i;
    double avg_time = 0;
    double std_dev_time = 0;
    double total_time = 0, std_dev_total_time = 0;
    int num_rows = time_vec.size();

    double min_time = DOUBLE_MAX, max_time = 0;

    // cout << "inside gen stats with num_find_rows " << num_find_rows << endl;


    vector<double>::const_iterator it;

    if ( num_rows > 0 )
    {
        it = min_element(time_vec.begin(), time_vec.end());
        min_time = *it;

        it = max_element(time_vec.begin(), time_vec.end());
        max_time = *it;

    }
    

    for ( i = 0; i < num_rows; i++ )
    {
        total_time += time_vec[i];
    }

    if ( num_rows > 0 )
    {
        avg_time = total_time/(double)num_rows;
    }

    for ( i = 0; i < num_rows; i++ )
    {
        std_dev_total_time += ( time_vec[i] - avg_time )*( time_vec[i] - avg_time );
    }

    if ( num_rows > 0 )
    {
        std_dev_time = sqrt( std_dev_total_time/(double)num_rows );
    }

    statObj stat_obj = statObj(total_time, num_rows, avg_time, std_dev_time, min_time, max_time, num_find_rows, queryType);

    return stat_obj;

}


/*
    This function takes a vector of statObj's and print those statistics to fileName
*/

void print_stat_to_file( vector<statObj> all_statObj, string fileName )
{

    std::ofstream stat_file(fileName.c_str());

    int i;

    for ( i = 0; i < all_statObj.size(); i++ )
    {
        if ( all_statObj[i].queryType == QUERY_INSERT )
        {
            stat_file << "Printing stats for bplus-tree insertions ... \n" << endl;
        }
        else if ( all_statObj[i].queryType == QUERY_POINT_SEARCH )
        {
            stat_file << "Printing stats for point search queries on bplus-tree ... \n" << endl;
        }
        else if ( all_statObj[i].queryType == QUERY_RANGE_SEARCH )
        {
            stat_file << "Printing stats for range search queries on bplus-tree ... \n" << endl;
        }

        stat_file << "Total time: \t" << all_statObj[i].total_time <<  " ms" << endl;
        stat_file << "Number of Queries: \t" << all_statObj[i].num_query << endl;
        stat_file << "Average Time per query: \t" << all_statObj[i].avg_time << " ms" << endl;
        stat_file << "Minimum Time of any query: \t" << all_statObj[i].min_time << " ms" << endl;
        stat_file << "Maximum Time of any query: \t" << all_statObj[i].max_time << " ms" << endl;
        stat_file << "Standard Deviation: \t" << all_statObj[i].std_dev_time << " ms" << endl;

        if ( all_statObj[i].queryType == QUERY_POINT_SEARCH || all_statObj[i].queryType == QUERY_RANGE_SEARCH )
        {
            stat_file << "\n" << "Number of successful rows found in find operation: \t" << all_statObj[i].num_find_rows << endl;
            if ( all_statObj[i].num_find_rows > 0 )
            {
                stat_file << "Average Time to find a row: " << all_statObj[i].total_time/(double)all_statObj[i].num_find_rows << " ms" << endl;
            }
        }

        stat_file << endl;
    }
}