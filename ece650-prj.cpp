// Compile with c++ ece650-a2cpp -std=c++11 -o ece650-a2

#include <memory>
// defines Var and Lit
#include "minisat/core/SolverTypes.h"
// defines Solver
#include "minisat/core/Solver.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <list>
#include <queue>
#include <pthread.h>
// #include <algorithm>
#include <bits/stdc++.h>
#include <math.h>
#include <errno.h>
#include <fstream>



#include <time.h>

 #define handle_error(msg) \
               do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error_en(en, msg) \
       do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

using namespace std;


unique_ptr<Minisat::Solver> solver(new Minisat::Solver());
vector<int> a4;
int nodes;
int check=0;
pthread_t cnf,vcc1,vcc2;
vector<list<int>> edgesVC1;
vector<list<int>> edgesVC2;
vector<int>result;
vector<int> vc_ap1;
vector<int>final_vertex_cover;
ifstream inFile;

vector<long double> vec_cnf_time;
vector<long double> vec_VC1_time;
vector<long double> vec_VC2_time;

vector<long double> vec_approx_cnf_size;
vector<long double> vec_approx_VC1_size;
vector<long double> vec_approx_VC2_size;


long double cnf_time;
long double vc1_time;
long double vc2_time;

typedef void * (*THREADFUNCPTR)(void *);

class Graph
{
public:
    int convert_int(string input)
    {
        int nodes=0;
        
        stringstream ss(input);
        ss>>nodes;
        return nodes;
    }

    void connecting_edges(vector<list<int>> edges,string input,int nodes);


    
    void *cnf_minisat( void *)
    {
        Minisat::lbool res;
        for(int k=1;k<=nodes;k++)
        {
            // unique_ptr<Minisat::Solver> solver(new Minisat::Solver());
            solver.reset (new Minisat::Solver());
            vector<vector<Minisat::Lit>> adj_matrix(nodes,vector<Minisat::Lit>(k));

            //creating n*k number of literals
            // cout<<"n*k created"<<endl;
            for (int i=0;i< nodes;i++)
            {
                for(int j=0;j<k;j++)
                {
                    adj_matrix[i][j]= Minisat::mkLit(solver->newVar());

                }
                        
            }

            // Clause 1
            // cout<<"clause 1 created"<<endl;
            for(int i=0;i<k;i++)
            {
                Minisat::vec<Minisat::Lit> clause1;
                for(int j=0;j<nodes;j++)
                {
                    clause1.push(adj_matrix[j][i]);
                }
                solver->addClause(clause1);
            }
           
            // Clause 2
            // cout<<"clause 2 created"<<endl;
            for(int m=0;m<nodes;m++)
            {
                for(int p=0;p<k;p++)
                {
                    for(int q=(p+1);q<k;q++)
                    {
                        solver->addClause(~adj_matrix[m][p], ~adj_matrix[m][q]);
                    }
                }
            }

            //Clause3
            // cout<<"clause 3 created"<<endl;
            for(int m=0;m<k;m++)
            {
                for(int p=0;p<nodes;p++)
                {
                    for(int q=(p+1);q<nodes;q++)
                    {
                        solver->addClause(~adj_matrix[p][m], ~adj_matrix[q][m]);
                    }
                }
            }

            //Clause 4
            // cout<<"clause 4 created"<<endl;
            for(unsigned int d=0;d< a4.size();d=d+2)
            {
                Minisat::vec<Minisat::Lit> clause4;
                for(int f=0;f<k;f++)
                {
                    clause4.push(adj_matrix[a4[d]][f]);
                    clause4.push(adj_matrix[a4[d+1]][f]);
                    
                }

                solver->addClause(clause4);

            }

            Minisat::vec<Minisat::Lit> dummy;
            res = solver->solveLimited(dummy);
             // solver->solve();

            // cout<<"RESULT "<<res<<endl;

            if (res==Minisat::l_True)
            {
                for(int i=0;i<nodes;i++)
                {
                    for(int j=0;j<k;j++)
                    {
                        if (( solver->modelValue(adj_matrix[i][j])) == Minisat::l_True)
                        {
                            result.push_back(i);
                        }
                    }
                }

                break;
            }
            else if (res==Minisat::l_Undef)
            {
                /* code */
                solver.reset(new Minisat::Solver());

                return NULL;

            }
            
           
        }

        sort(result.begin(),result.end());

        /* TIME TO GET THE CLOCK TIME FOR CNF*/
        
       //  clockid_t cid_cnf;
        
       //  int s2= pthread_getcpuclockid(cnf, &cid_cnf);
       //  if (s2 != 0)
       //     handle_error_en(s2, "pthread_getcpuclockid");
       // // pclock("Main thread CPU time:   ", cid);
       // cnf_time=pclock("Main thread CPU time:   ", cid_cnf);
    
    
    }

    void * approxVC1(void *)
    {
        int flag = 0;
       
        vector<int> remove_edge;
        // vector<int> edge_vector_copy=edge_vector;

        int max_vert=0;
        int old_vert_count=0;
        for(size_t i = 0; i < edgesVC1.size(); i++)
        {
            int vert_count=0;
            // int max_vert_count=0;
            for(auto &j : edgesVC1[i])//range based for loop to iterate through adjList[i] and use the reference j to refer to each element
            {
                //cerr << j << ' ';
                if(j != -1)
                {
                    vert_count++;
                }
                

            }
            if(vert_count>old_vert_count)
            {
                max_vert=i;
                old_vert_count=vert_count;
            }
        }
        for(auto &j : edgesVC1[max_vert])//range based for loop to iterate through adjList[i] and use the reference j to refer to each element
        {
            
            remove_edge.push_back(j);

        }
        vc_ap1.push_back(max_vert);
      
       edgesVC1[max_vert].clear();
       
      
        
       vector<list<int>> re_adj;
       re_adj.resize(nodes);
       int edgeee_count=0;
       // cout<<"count initialization"<<edgeee_count<<endl;
       
        for (int i=0 ;i<edgesVC1.size();i++)
        {
            
            // cout << i << ": ";
            if(!edgesVC1[i].empty())
            {
               
                for(auto &j : edgesVC1[i])
                {
                    if(j == max_vert)
                    {
                        j= -1;
                    }

                }
            }
            

        }

        for(int i = 0; i< edgesVC1.size();i++)
        {

            if(!edgesVC1[i].empty())
            {
                
                for(auto &j : edgesVC1[i])
                {
                    if(j != -1)
                    {
                        
                        edgeee_count++;

                    }
                }

            }
            
        }
        for(int i = 0; i< edgesVC1.size();i++)
        {
            
            
            if(!edgesVC1[i].empty())
            {
                int count_null=0;

                for(auto &j : edgesVC1[i])
                {
                    if(j == -1)
                    {
                        
                        count_null++;
                        
                    }

                }
                if( edgesVC1[i].size() == count_null)
                {
                    edgesVC1[i].clear();
                }
            }
            if(edgesVC1[i].empty())
            {
                flag++;
            }

        }

        void *test =NULL;
        if(flag!=nodes)
        {
            approxVC1(test);
        }

        else
        {
            sort(vc_ap1.begin(),vc_ap1.end());
            
        }

        /*GETTING CLOCK TIME FOR VC1*/

       //  clockid_t cid_vc1;
        
       //  int s1= pthread_getcpuclockid(vcc1, &cid_vc1);
       //  if (s1 != 0)
       //     handle_error_en(s1, "pthread_getcpuclockid");
       // // pclock("Main thread CPU time:   ", cid);
       // vc1_time=pclock("Main thread CPU time:   ", cid_vc1);
    }

    void *approxVC2(void *)
    {
        
        vector<int>vertex_cover;
        int flag1=0;

        for(int i=0;i<edgesVC2.size();i++)
        {
            if(!edgesVC2[i].empty())
            {
                for(auto &j: edgesVC2[i])
                {
                    if(j!= -1)
                    {
                        vertex_cover.push_back(i);
                        vertex_cover.push_back(j);
                        break;
                    }
                    
                }
                break;

            }
        }
        for(int i=0;i<vertex_cover.size();i++)
        {
            
            final_vertex_cover.push_back(vertex_cover[i]);
            for(int c=0;c<edgesVC2.size();c++)
            {
              edgesVC2[vertex_cover[i]].clear();  
              for(auto &j:edgesVC2[c])
              {
                if(j== vertex_cover[i])
                {
                    j=-1;
                }

              }
              
            }
            

        }
        for(int i = 0; i< edgesVC2.size();i++)
        {
            
            if(!edgesVC2[i].empty())
            {
                int count_null=0;
                for(auto &j : edgesVC2[i])
                {
                    if(j == -1)
                    {
                        
                        count_null++;
                        
                    }

                }
                if( edgesVC2[i].size() == count_null)
                {
                    edgesVC2[i].clear();
                }
            }
            if(edgesVC2[i].empty())
            {
                flag1++;
            }


        }
        void * test2=NULL;
        if(flag1 != nodes)
        {
            approxVC2(test2);
        }
        else
        {
            
            sort(final_vertex_cover.begin(),final_vertex_cover.end());
            
          
        }

        /* GETTING THE CLOCK TIME FOR APPRX VC2 METHOD*/


       //  clockid_t cid_vc2;
        
       //  int s= pthread_getcpuclockid(vcc2, &cid_vc2);
       //  if (s != 0)
       //     handle_error_en(s, "pthread_getcpuclockid");
       // // pclock("Main thread CPU time:   ", cid);
       // vc2_time=pclock("Main thread CPU time:   ", cid_vc2);



    
    }
    /* TIME PCLOCK FUNCTION TO GET TIME FOR EACH ALGORITHM */


    // static long double pclock(char *msg, clockid_t cid)
    // {
    //    struct timespec ts;

    //    if (clock_gettime(cid, &ts) == -1)
    //        handle_error("clock_gettime");
    // return(ts.tv_sec*1000000+(ts.tv_nsec/1000));
    // }

    
};

void interupt()
{
    solver-> interrupt();
}

/* STANDAARAD DEVIATION FUNCTION FOR ANALYSIS*/

// long double standaradDeviation(vector<long double> time_vector , long double mean)
// {
//     long double addition=0;
//     for(int i=0;i<time_vector.size();i++)
//     {
//         addition += pow(time_vector[i] - mean,2);

//     }
//     return sqrt(addition/time_vector.size());
// }


void print_outcome()
{
    cout<<"CNF-SAT-VC:"<<" ";
    if(result.empty())
    {
        cout<<"timeout"<<endl;
    }
    else
    {
        for(int i=0;i<result.size();i++)
        {

            cout<<result[i];
            if(i != (result.size()-1))

                cout<<",";
            

        } 
        cout<<endl;
    }
    
    

    a4.clear();
    result.clear();
    cout<<"APPROX-VC-1:"<<" ";
    for (int i=0;i<vc_ap1.size();i++)
    {

        cout<<vc_ap1[i];
        if(i != (vc_ap1.size()-1))

            cout<<",";
    }
    cout<<endl;
    vec_approx_VC1_size.push_back(vc_ap1.size());
    
    vc_ap1.clear();
    edgesVC1.clear();
  

    
    cout<<"APPROX-VC-2:"<<" ";
    for(int i=0;i<final_vertex_cover.size();i++)
    {
        cout<<final_vertex_cover[i];
        if(i != (final_vertex_cover.size()-1))

            cout<<",";
    }
    cout<<endl;
    
    final_vertex_cover.clear();
    edgesVC2.clear();


}



void Graph::connecting_edges(vector<list<int>> edges,string input,int nodes)
{
    int k=0, size=input.length(),count=0;
    int edge1,edge2;
    edges.resize(nodes);
    string dummy="";
    while(k<size)
    {
        while(k<size && input[k]!= '>')
        {
            dummy += input[k];
            ++k;
        }
        k+=2;
        sscanf(dummy.c_str(), "<%d,%d" ,&edge1, &edge2);

        a4.push_back(edge1);
        a4.push_back(edge2);
    
        if ((edge1>(nodes-1)) || (edge2>(nodes-1)))
        {
            count++;
        }
            
        edges[edge1].push_back(edge2); 
        edges[edge2].push_back(edge1);
        dummy="";
    }
    edgesVC1=edges;
    edgesVC2=edges;
    
}



void *parsing(void *)
{
   string input;
    int start,end;
    vector<list<int>> edges;
    Graph BFS;
    
    while(!cin.eof())
    {
        getline(cin,input);
      
        if (input[0]=='V')
        {
            nodes= BFS.convert_int(input.substr(2));
           
        }
        
        else if (input[0]=='E')
        {
            
            
                BFS.connecting_edges(edges,input.substr(3,input.length()-4),nodes);
                pthread_create(&cnf,NULL,(THREADFUNCPTR) &Graph::cnf_minisat,&BFS);
                pthread_create(&vcc1,NULL,(THREADFUNCPTR) &Graph::approxVC1,&BFS);
                pthread_create(&vcc2,NULL,(THREADFUNCPTR) &Graph::approxVC2,&BFS);

                sleep(2);
                interupt();

                pthread_join(cnf,NULL);
                pthread_join(vcc1,NULL);
                pthread_join(vcc2,NULL);

                print_outcome();

                /* STANDARAD DEVIATION TIME ANALYSIS FOR APPROXIMATION RATIO AND RUNNING TIME FOR VC1 AND VC2*/

            // long double sum_cnf=accumulate(vec_cnf_time.begin(),vec_cnf_time.end(),0.0);
            // long double avg_cnf=sum_cnf/vec_cnf_time.size();
            // long double std_cnf=standaradDeviation(vec_cnf_time,avg_cnf);


            // long double sum_approx_cnf=accumulate(vec_approx_cnf_size.begin(),vec_approx_cnf_size.end(),0.0);
            // long double avg_approx_cnf= sum_approx_cnf/vec_approx_cnf_size.size();
            // long double approx_ratio_cnf=avg_approx_cnf/avg_approx_cnf;
            // long double std_approx_cnf= standaradDeviation(vec_approx_cnf_size,approx_ratio_cnf);
            
            // long double sum_VC1=accumulate(vec_VC1_time.begin(),vec_VC1_time.end(),0.0);
            // long double avg_VC1=sum_VC1/vec_VC1_time.size();
            // long double std_VC1=standaradDeviation(vec_VC1_time,avg_VC1);

            // long double sum_approx_VC1=accumulate(vec_approx_VC1_size.begin(),vec_approx_VC1_size.end(),0.0);
            // long double avg_approx_VC1= sum_approx_VC1/vec_approx_VC1_size.size();
            // long double approx_ratio_VC1=avg_approx_VC1/avg_approx_cnf;
            // long double std_approx_VC1= standaradDeviation(vec_approx_VC1_size,approx_ratio_VC1);
            
            // long double sum_VC2=accumulate(vec_VC2_time.begin(),vec_VC2_time.end(),0.0);
            // long double avg_VC2=sum_VC2/vec_VC2_time.size();
            // long double std_VC2=standaradDeviation(vec_VC2_time,avg_VC2);

            // long double sum_approx_VC2=accumulate(vec_approx_VC2_size.begin(),vec_approx_VC2_size.end(),0.0);
            // long double avg_approx_VC2= sum_approx_VC2/vec_approx_VC2_size.size();
            // long double approx_ratio_VC2=avg_approx_VC2/avg_approx_cnf;
            // long double std_approx_VC2= standaradDeviation(vec_approx_VC2_size,approx_ratio_VC2);

           

            
            // cout<<nodes<<","<<avg_cnf<<","<<std_cnf<<","<<approx_ratio_cnf<<","<<std_approx_cnf<<","<<avg_VC1<<","<<std_VC1<<","<<approx_ratio_VC1<<","<<std_approx_VC1<<","<<avg_VC2<<","<<std_VC2<<","<<approx_ratio_VC2<<","<<std_approx_VC2<<endl;

            // vec_cnf_time.clear();
            // vec_VC1_time.clear();
            // vec_VC2_time.clear();

            // vec_approx_cnf_size.clear();
            // vec_approx_VC1_size.clear();
            // vec_approx_VC2_size.clear();

        
        }
        
    }

        
} 



int main()
{
 


    pthread_t parsing_thread;

    
    pthread_create(&parsing_thread,NULL,&parsing,NULL);
    pthread_join(parsing_thread,NULL);

    return 0;
}



// V 15
// E {<2,6>,<2,8>,<2,5>,<6,5>,<5,8>,<6,10>,<10,8>}
