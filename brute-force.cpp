#include <iostream>
#include <fstream>
#include <string>
#include "Snap.h"
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
using namespace std;
const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

const int K = 10;

float jaccard(TNGraph::TNodeI NI1, TNGraph::TNodeI NI2) {
  int lenA = NI1.GetOutDeg();
  int lenB = NI2.GetOutDeg();
  int ct = 0;
  int j = 0;
  int i = 0;
  while (i< lenA && j < lenB){
  	if (NI1.GetOutNId(i) == NI2.GetOutNId(j)){
  		ct++; i++; j++;
  	}
  	else if (NI1.GetOutNId(i) > NI2.GetOutNId(j))
  		j++;
  	else
  		i++;
  }
  return ct*1.0/(lenA+lenB-ct);

}

void MergeNbrs(TIntV* NeighbourV, TIntV* list1, TNGraph::TNodeI NI2) {
//  cout<<"In Merge"<<endl;
	int j = 0;
  int k = 0;
  int prev = -1;
  int lenA = list1->Len();

  int lenB = NI2.GetInDeg();
//  cout<<lenA<<" "<<lenB<< endl;
  if (lenA > 0  &&  lenB > 0) {
    int v1 = (*list1)[j];
    int v2 = NI2.GetInNId(k);
    while (1) {
      if (v1 <= v2) {
        if (prev != v1) {
          NeighbourV->Add(v1);
          prev = v1;
        }
        j += 1;
        if (j >= lenA) {
          break;
        }
        v1 = (*list1)[j];
      } else {
        if (prev != v2) {
          NeighbourV->Add(v2);
          prev = v2;
        }
        k += 1;
        if (k >= lenB) {
          break;
        }
        v2 = NI2.GetInNId(k);
      }
    }
  }
  while (j < lenA) {
    int v = (*list1)[j];
    if (prev != v) {
      NeighbourV->Add(v);
      prev = v;
    }
    j += 1;
  }
  while (k < lenB) {
    int v = NI2.GetInNId(k);
    if (prev != v) {
      NeighbourV->Add(v);
      prev = v;
    }
    k += 1;
  }
}

int main(int argc,char* argv[]) {
  char src_file[] = "/dfs/scratch0/dataset/MicrosoftAcademicGraph/20160205/PaperAuthorAffiliations.txt";
  TTableContext Context;
  // Create schema.
  Schema PAA;
  // PAA.Add(TPair<TStr,TAttrType>("PaperID", atStr));
  // PAA.Add(TPair<TStr,TAttrType>("AuthorID", atStr));
  // PAA.Add(TPair<TStr,TAttrType>("AfflID", atStr));
  // PAA.Add(TPair<TStr,TAttrType>("AfflName", atStr));
  // PAA.Add(TPair<TStr,TAttrType>("AfflNameNorm", atStr));
  // PAA.Add(TPair<TStr,TAttrType>("AuthSeqNum", atInt));
  // TIntV RelevantCols;
  // RelevantCols.Add(0); RelevantCols.Add(1); RelevantCols.Add(2);
  // RelevantCols.Add(3); RelevantCols.Add(4); RelevantCols.Add(5);
  // cerr<<"loading table"<<currentDateTime()<<"\n";

  // PTable P = TTable::LoadSS(PAA, src_file, &Context, RelevantCols);
  // cerr<<"Table loaded"<<currentDateTime()<<"\n";

  // {
  // TFOut SOut("paperAuthAfflTable3.bin");
  // P->Save(SOut);
  // Context.Save(SOut);
  // }
  // cerr<<currentDateTime();
//
//
//
//  cerr<<"Table saved"<<currentDateTime()<<"\n";
//
//  // Test SaveSS by loading the saved table and testing values again.
//  //PAA.Add(TPair<TStr,TAttrType>("_id", atInt));
  cout<<currentDateTime()<<endl;
  PTable P;
  {
  TFIn SIn("/lfs/madmax6/0/mohanas/snap-dev/examples/cascadegen/paperAuthAfflTable3.bin");
  cerr<<"Sin done";
  P = TTable::Load(SIn, &Context);
  Context.Load(SIn);
  }
  cerr<<"Table loaded "<<currentDateTime()<<endl;
  TVec<TPair<TStr, TAttrType> > S = P->GetSchema();
  PNGraph Graph = TSnap::ToGraph<PNGraph>(P, S[1].GetVal1(), S[2].GetVal1(), aaFirst);

  Graph->DelNode(27961319);
  TSnap::PlotOutDegDistr (Graph, "OutDegDist", "" , false, true);
  cerr<<"graph constructed"<<currentDateTime()<<"\n";
  //cout<<"graph constructed"<<currentDateTime()<<"\n";
  cout<<Graph->GetNodes()<<endl;
  int ct = 0;
  string filename = argv[3];
  ofstream fil;
  fil.open(argv[3],ios::out);
  int start = atoi(argv[1]);
  int end = atoi(argv[2]);
  long long int sum_neighbors = 0;
  TIntV* Neighbors_old = new TIntV();
	TIntV* Neighbors = new TIntV();
	TIntV* temp;
  for (TNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++){
    if (NI.GetInDeg() > 0)
      continue;
    if (NI.GetOutDeg() == 0)
    	continue;
    ct ++;
    if (ct < start)
    	continue;
    if (ct > end)
    	break;
    //cout<<ct<<" author: "<<NI.GetId()<<endl;
    TVec<TPair<TFlt, TInt> > TopK;
    for (int i = 0; i < K; i++)
    	TopK.Add(TPair<TFlt,TInt>(0.0, 0));

    Neighbors->Clr(false);
    Neighbors_old->Clr(false);

    for (int i = 0; i < NI.GetOutDeg(); i++){
    	TNGraph::TNodeI Inst_NI = Graph->GetNI(NI.GetOutNId(i));
      //cout<<NI.GetOutNId(i)<<" "<<Inst_NI.GetInDeg()<<endl;
    	MergeNbrs(Neighbors, Neighbors_old, Inst_NI);

    	temp = Neighbors_old;
    	temp->Clr(false);
    	Neighbors_old = Neighbors;
    	Neighbors = temp;

    }
    int num = Neighbors_old->Len();
    sum_neighbors += num;
//    cout<<num<<endl;
//    for (int i = 0; i < num; i++)
//    	cout<<(*Neighbors_old)[i]<<" ";



    //cout<<"Num Neighbors: "<<Neighbors.Len()<<endl;
    //Swap neighbors and Neighbors_old

//    temp = Neighbors_old;
//    Neighbors_old = Neighbors;
//    Neighbors = temp;
//    for(int j = 0; j< Neighbors->Len(); j++){
//
//    	TNGraph::TNodeI Auth_NI = Graph->GetNI((*Neighbors)[j]);
//
//    	float similarity = jaccard(NI, Auth_NI);
//    	if (TopK[K-1].GetVal1() < similarity){
//    		int index = 0;
//    		for (int i = K-2; i >= 0; i--)
//    			if (TopK[i].GetVal1() < similarity)
//    				TopK.SetVal(i+1, TopK[i]);
//    			else{
//    				index = i+1;
//    				break;
//    			}
//    		TopK.SetVal(index, TPair<TFlt, TInt>(similarity, (*Neighbors)[j]));
//    	}
//    }
//    fil<<NI.GetId()<<" ";
//    for (int i = 0; i < K; i++)
//    	fil<<"("<<TopK[i].GetVal1()<<","<<TopK[i].GetVal2()<<") ";
//    fil<<"\n";

    if (ct%10000 == 0)
    	cout<<ct<<" avg neighbor degree = "<<sum_neighbors*1.0/ct<<" "<<currentDateTime()<<endl;

  }

  fil.close();
  return 0;
}


//#include "stdafx.h"
//#include "Snap.h"
//
//int main(int argc,char* argv[]) {
//  //Read input from file and store in table.
//  TTableContext Context;
//  //Create schema
//  //Input File Format Source,Dest,Start_Time,Duration
//  Schema TimeS;
//  TimeS.Add(TPair<TStr,TAttrType>("Source",atInt));
//  TimeS.Add(TPair<TStr,TAttrType>("Dest",atInt));
//  TimeS.Add(TPair<TStr,TAttrType>("Start",atInt));
//  TimeS.Add(TPair<TStr,TAttrType>("Duration",atInt));
//  PTable P = TTable::LoadSS(TimeS,argv[1],&Context,' ');
//  TInt W = atoi(argv[2]);
//  // Sort by Source
//  PNGraph Graph = TSnap::CascGraph(P,"Source","Dest","Start","Duration",W);
//
//  // Save the edge list in a text format
//  TSnap::SaveEdgeList(Graph, "cascades.txt");
//
//  // Save the cascade graph in a binary format
//  //TFOut FOut("cascade.graph");
//  //Graph->Save(FOut);
//
//  // Print the graph in a human readable format
//  //Graph->Dump();
//
//  // Sequential
//  TVec<TIntV> TopCascVV;
//  TSnap::CascFind(Graph,P,"Source","Dest","Start","Duration",TopCascVV,false);
//  // Print statistics about top cascasdes
//  int max = 0;
//  int totalEvents = 0;
//  for (int i = 0; i < TopCascVV.Len(); i++) {
//    if (TopCascVV[i].Len() > max) { max = TopCascVV[i].Len();}
//    totalEvents += TopCascVV[i].Len();
//  }
//  printf("TotalCasc %d, TotalEvents %d, max %d\n",TopCascVV.Len(),totalEvents,max);
//  return 0;
//}
//

//#include <iostream>
//#include <fstream>
//#include <string>
//#include "Snap.h"
//#include <stdlib.h>
//#include <time.h>
//#include <unistd.h>
//using namespace std;
//const std::string currentDateTime() {
//    time_t     now = time(0);
//    struct tm  tstruct;
//    char       buf[80];
//    tstruct = *localtime(&now);
//    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
//    // for more information about date/time format
//    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
//
//    return buf;
//}
//
//const int K = 10;
//
//float jaccard(TNGraph::TNodeI NI1, TNGraph::TNodeI NI2) {
//  int lenA = NI1.GetOutDeg();
//  int lenB = NI2.GetOutDeg();
//  int ct = 0;
//  int j = 0;
//  int i = 0;
//  while (i< lenA && j < lenB){
//  	if (NI1.GetOutNId(i) == NI2.GetOutNId(j)){
//  		ct++; i++; j++;
//  	}
//  	else if (NI1.GetOutNId(i) > NI2.GetOutNId(j))
//  		j++;
//  	else
//  		i++;
//  }
//  return ct*1.0/(lenA+lenB-ct);
//
//}
//
//int main(int argc,char* argv[]) {
//  char src_file[] = "/dfs/scratch0/dataset/MicrosoftAcademicGraph/20160205/PaperAuthorAffiliations.txt";
//  TTableContext Context;
//  // Create schema.
//  Schema PAA;
//  // PAA.Add(TPair<TStr,TAttrType>("PaperID", atStr));
//  // PAA.Add(TPair<TStr,TAttrType>("AuthorID", atStr));
//  // PAA.Add(TPair<TStr,TAttrType>("AfflID", atStr));
//  // PAA.Add(TPair<TStr,TAttrType>("AfflName", atStr));
//  // PAA.Add(TPair<TStr,TAttrType>("AfflNameNorm", atStr));
//  // PAA.Add(TPair<TStr,TAttrType>("AuthSeqNum", atInt));
//  // TIntV RelevantCols;
//  // RelevantCols.Add(0); RelevantCols.Add(1); RelevantCols.Add(2);
//  // RelevantCols.Add(3); RelevantCols.Add(4); RelevantCols.Add(5);
//  // cerr<<"loading table"<<currentDateTime()<<"\n";
//
//  // PTable P = TTable::LoadSS(PAA, src_file, &Context, RelevantCols);
//  // cerr<<"Table loaded"<<currentDateTime()<<"\n";
//
//  // {
//  // TFOut SOut("paperAuthAfflTable3.bin");
//  // P->Save(SOut);
//  // Context.Save(SOut);
//  // }
//  // cerr<<currentDateTime();
////
////
////
////  cerr<<"Table saved"<<currentDateTime()<<"\n";
////
////  // Test SaveSS by loading the saved table and testing values again.
////  //PAA.Add(TPair<TStr,TAttrType>("_id", atInt));
//  cout<<currentDateTime()<<endl;
//  PTable P;
//  {
//  TFIn SIn("paperAuthAfflTable3.bin");
//  cerr<<"Sin done";
//  P = TTable::Load(SIn, &Context);
//  Context.Load(SIn);
//  }
//  cerr<<"Table loaded "<<currentDateTime()<<endl;
//  TVec<TPair<TStr, TAttrType> > S = P->GetSchema();
//  PNGraph Graph = TSnap::ToGraph<PNGraph>(P, S[1].GetVal1(), S[2].GetVal1(), aaFirst);
//  //TSnap::PlotInDegDistr (Graph, "InDegDist", "" , false, true);
//  //TSnap::PlotOutDegDistr (Graph, "OutDegDist", "" , false, true);
//  Graph->DelNode(27961319);
//  cerr<<"graph constructed"<<currentDateTime()<<"\n";
//  //cout<<"graph constructed"<<currentDateTime()<<"\n";
//  cout<<Graph->GetNodes()<<endl;
//  int ct = 0;
//  string filename = argv[3];
//  ofstream fil;
//  fil.open(argv[3],ios::out);
//  int start = atoi(argv[1]);
//  int end = atoi(argv[2]);
//  for (TNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++){
//    if (NI.GetInDeg() > 0)
//      continue;
//    ct ++;
//    if (ct < start)
//    	continue;
//    if (ct > end)
//    	break;
//    cout<<ct<<" author: "<<NI.GetId()<<endl;
//    TVec<TPair<TFlt, TInt> > TopK;
//    for (int i = 0; i < K; i++)
//    	TopK.Add(TPair<TFlt,TInt>(0.0, 0));
//    TIntSet Neighbors;
//
//    for (int i = 0; i < NI.GetOutDeg(); i++){
//    	TNGraph::TNodeI Inst_NI = Graph->GetNI(NI.GetOutNId(i));
//      cout<<NI.GetOutNId(i)<<" "<<Inst_NI.GetInDeg()<<endl;
//    	for (int j=0; j < Inst_NI.GetInDeg(); j++)
//    		Neighbors.AddKey(Inst_NI.GetInNId(j));
//    }
//    cout<<"Num Neighbors: "<<Neighbors.Len()<<endl;
//
//    for(TIntSet::TIter It = Neighbors.BegI(); It < Neighbors.EndI(); It++){
//
//    	TNGraph::TNodeI Auth_NI = Graph->GetNI(It.GetKey());
//
//    	float similarity = jaccard(NI, Auth_NI);
//    	if (TopK[K-1].GetVal1() < similarity){
//    		int index = 0;
//    		for (int i = K-2; i >= 0; i--)
//    			if (TopK[i].GetVal1() < similarity)
//    				TopK.SetVal(i+1, TopK[i]);
//    			else{
//    				index = i+1;
//    				break;
//    			}
//    		TopK.SetVal(index, TPair<TFlt, TInt>(similarity, It.GetKey()));
//    	}
//    }
//    fil<<NI.GetId()<<" ";
//    for (int i = 0; i < K; i++)
//    	fil<<"("<<TopK[i].GetVal1()<<","<<TopK[i].GetVal2()<<") ";
//    fil<<"\n";
//
//    if (ct%100000 == 0)
//    	cout<<ct<<" "<<currentDateTime()<<endl;
//
//  }
//  fil.close();
//  return 0;
//}
//
//
////#include "stdafx.h"
////#include "Snap.h"
////
////int main(int argc,char* argv[]) {
////  //Read input from file and store in table.
////  TTableContext Context;
////  //Create schema
////  //Input File Format Source,Dest,Start_Time,Duration
////  Schema TimeS;
////  TimeS.Add(TPair<TStr,TAttrType>("Source",atInt));
////  TimeS.Add(TPair<TStr,TAttrType>("Dest",atInt));
////  TimeS.Add(TPair<TStr,TAttrType>("Start",atInt));
////  TimeS.Add(TPair<TStr,TAttrType>("Duration",atInt));
////  PTable P = TTable::LoadSS(TimeS,argv[1],&Context,' ');
////  TInt W = atoi(argv[2]);
////  // Sort by Source
////  PNGraph Graph = TSnap::CascGraph(P,"Source","Dest","Start","Duration",W);
////
////  // Save the edge list in a text format
////  TSnap::SaveEdgeList(Graph, "cascades.txt");
////
////  // Save the cascade graph in a binary format
////  //TFOut FOut("cascade.graph");
////  //Graph->Save(FOut);
////
////  // Print the graph in a human readable format
////  //Graph->Dump();
////
////  // Sequential
////  TVec<TIntV> TopCascVV;
////  TSnap::CascFind(Graph,P,"Source","Dest","Start","Duration",TopCascVV,false);
////  // Print statistics about top cascasdes
////  int max = 0;
////  int totalEvents = 0;
////  for (int i = 0; i < TopCascVV.Len(); i++) {
////    if (TopCascVV[i].Len() > max) { max = TopCascVV[i].Len();}
////    totalEvents += TopCascVV[i].Len();
////  }
////  printf("TotalCasc %d, TotalEvents %d, max %d\n",TopCascVV.Len(),totalEvents,max);
////  return 0;
////}
////
