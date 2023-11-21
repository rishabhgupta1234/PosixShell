#include "task2.h"
#include "task4.h"

#include <iostream>
#include <string>
#include <algorithm>
#include <map>
#include <limits.h>
#include <vector>

using namespace std;

string removeCharacters(string &S, char c)
{
    S.erase(remove(S.begin(), S.end(), c), S.end());
    return S;
}

static inline void ltrim(string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch)
                                    { return !isspace(ch); }));
}

static inline void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)
                         { return !isspace(ch); })
                .base(),
            s.end());
}

static inline void trim(string &s)
{
    ltrim(s);
    rtrim(s);
}

void load_history(his_trie *root, string hist_filename, int &gb_count)
{
    FILE *file = fopen(hist_filename.c_str(), "r");
    if (file != NULL)
    {
        char line[128];
        int no = 0;
        string comm;
        while (fgets(line, sizeof(line), file) != NULL)
        {
            gb_count += 1;
            comm = string(line);
            comm = removeCharacters(comm, '\n');
            comm = removeCharacters(comm, '\t');
            root->insert(root, comm, gb_count);
            no++;
        }
        fclose(file);
    }
    else
    {
        cout << "Loading " << hist_filename << " failed" << endl;
        exit(EXIT_FAILURE);
    }
    return;
}

void save_history(his_trie root, string hist_filename)
{
    FILE *file = fopen(hist_filename.c_str(), "w");
    vector<pair<int, string>> save_vec;
    save_vec.clear();
    his_trie *curr = &root;
    root.is_valid_prefix(curr, "", save_vec);
    sort(save_vec.begin(), save_vec.end());
    for (int i = 0; i < save_vec.size(); i++)
    {
        trim(save_vec[i].second);
        if (!save_vec[i].second.empty())
        {
            fprintf(file, "%s\n", save_vec[i].second.c_str());
        }
    }
    fclose(file);
}

void print_history(his_trie root, int limit, string &recordFilePath)
{
    vector<pair<int, string>> print_vec;
    print_vec.clear();
    his_trie *curr = &root;
    root.is_valid_prefix(curr, "", print_vec);
    sort(print_vec.begin(), print_vec.end());
    reverse(print_vec.begin(), print_vec.end());
    int size;
    if (limit >= print_vec.size())
        size = print_vec.size();
    else
        size = limit;
    for (int i = 0; i < size; i++)
    {
        cout << endl
             << print_vec[i].second;
        handleRecordingOutput(recordFilePath, print_vec[i].second);
    }
    cout << endl;
}
