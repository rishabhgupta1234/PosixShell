#include <vector>
#include <string>
#include <map>
#include <dirent.h>
#include <limits.h>
using namespace std;
void insertWordInTrie(string);
void populateTrie(void);
static inline void trim(string &);
class TrieNode
{
public:
    bool isLeaf;
    string word;
    map<char, TrieNode *> children;

    TrieNode()
    {
        this->isLeaf = false;
        this->word = "";
    }
};

class Trie
{
public:
    TrieNode *root;

    Trie()
    {
        root = new TrieNode();
    }

    void insert(string data) // priyank done
    {
        TrieNode *curr = this->root;
        int lm=data.size();
        for (int i = 0; i < lm; i++)
        {
            if (curr->children.find(data[i]) == curr->children.end()) // if anything with char word[i] not found in map 
                curr->children[data[i]] = new TrieNode();

            curr = curr->children[data[i]];
        }
        curr->isLeaf = true;
        curr->word = data;

    }

    bool isLastNode(TrieNode *root)  // done priyank
    {
        return (root->children.size() == 0);
    }

    void autocompleteRecUtil(TrieNode *curr, string &n_pre, vector<string> &op) // dfs here done priyank
    {
        if (curr->isLeaf) 
            op.push_back(n_pre);

        if (isLastNode(curr))
            return;

        for (auto idef = curr->children.begin(); idef != curr->children.end(); idef++) 
        {
            TrieNode *n_ch = idef->second;

            n_pre.push_back(idef->first);

            autocompleteRecUtil(n_ch, n_pre, op);

            n_pre.pop_back();
        }

    }

    void autocomplete(string &prefix, vector<string> &op)  // done priyank 
    {
        TrieNode *curr = this->root;

        if (curr == NULL)
            return;

        //see if prefix  is there in trie
        int lm=prefix.size();
        for (int i = 0; i < lm; i++)
        {
            if (curr->children[prefix[i]] != NULL)
                curr = curr->children[prefix[i]];
            else
                return;
        }

        // prefix is found to be valid data
        if (isLastNode(curr) && curr->isLeaf)
            op.push_back(prefix);

        // prefix is valid data , but last data of trie
        if (isLastNode(curr))
        {
            return;
        }

        string n_pre = prefix;
        autocompleteRecUtil(curr, n_pre, op);

        return;
    }


    void populateTrie(vector<string> &path_variable) /// priyank  done    moved
    {
        vector<string> basicCommands; 
        DIR *dr; 
        struct dirent *en;
 
        for(unsigned int i=0;i<path_variable.size();i++) 
        { 
            dr = opendir(path_variable[i].c_str()); //open all directory 
            if (dr)  
            { 
                while ((en = readdir(dr)) != NULL)  
                { 
                    string type="other"; 
                    if(en->d_type==DT_REG) 
                    { 
                        basicCommands.push_back(string(en->d_name)); 
                    }
                } 
                closedir(dr); //close all directory 
            } 
 
        } 

        for (int i = 0; i < basicCommands.size(); i++)
        {
            insert(basicCommands[i]);
        }
    }
    // END autocorrect
};
class his_trie
{
    public:
        his_trie *ref[256]={NULL};
        bool flag=false;
        int count=0;

    void insert(his_trie *curr, string word, int &gb_count)
    {
        char ch;
        his_trie *curr1 = curr;
        for(int i=0;i<word.size();i++)
        {
            ch=word[i];
            int ascii=0;
            ascii=int(ch);
            if(curr1->ref[ascii]==NULL)
            {
                his_trie *temp=new his_trie();
                curr1->ref[ascii]=temp;
            }
            curr1=curr1->ref[ascii];
        }
        curr1->flag=true;
        curr1->count=gb_count;
    }

    bool char_exist(char ch)
    {
        return ref[int(ch)]!=NULL;
    }

    bool check(his_trie * curr,string word)
    {
        bool is_there=true;
        char ch;
        for(int i=0;i<word.size();i++)
        {
            ch=word[i];
            if(curr->char_exist(ch))
                is_there=true;
            else
                return false;

            curr=curr->ref[int(ch)];

        }
        if(curr->flag==true)
            return true;
        return false;
    }

    bool is_valid_prefix(his_trie *curr,string word,vector<pair<int,string> >&prefix)
    {
        char ch;
        for(int i=0;i<word.size();i++)
        {
            ch=word[i];

            if(!curr->char_exist(ch))
            {
                return false;
            }
            curr=curr->ref[int(ch)];
        }
        collect_strings(prefix,curr,word);
        return true;
    }
    // void collect_strings(vector<string>&prefix,his_trie * curr,char *c_arr,string word,int pos=0)
    void collect_strings(vector< pair<int,string> >&prefix,his_trie * curr,string word)
    {
        if(curr->flag==true)
        {
            prefix.push_back({curr->count,word});
        }
        for(int i=0;i<256;i++)
        {
            if(curr->ref[i]!=NULL)
            {
                char x=i;
                word+=x;
                collect_strings(prefix,curr->ref[i],word);
                word.pop_back();
            }
        }
    }
};

void load_history(his_trie*, string, int &);
void save_history(his_trie, string);
void print_history(his_trie, int, string &);
