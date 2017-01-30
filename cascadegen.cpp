#include <iostream>
#include <fstream>
#include <string>
#include "Snap.h"
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <vector>
#include <utility>
#include <algorithm>
using namespace std;
/// Node for storing author information in a tree node/////
class Node {
public:
	int AuthID;
	int deg;
	int count;
	void Save(TSOut &SOut){
		SOut.Save(AuthID);
		SOut.Save(deg);
		SOut.Save(count);
	}
	void Load(TSIn &SIn){
			SIn.Load(AuthID);
			SIn.Load(deg);
			SIn.Load(count);
		}
};

TStrHash<TVec<Node> > tree_node_hash;

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
void merge(TVec<Node> * v1, TVec<Node>* v2, TVec<Node>* v){ // assuming the two tree nodes don't have institutions in common
	int j = 0;
	int k = 0;
	int len1 = v1->Len();
	int len2 = v2->Len();
	if (len1 > 0 && len2 > 0){
		while(j < len1 && k < len2){
			Node node1 = (*v1)[j];
			Node node2 = (*v2)[k];
			if(node1.count == node1.deg){ // assuming the two tree nodes don't have institutions in common else we should also remove in other list
				j++;
			}
			else if(node2.count == node2.deg){ // assuming the two tree nodes don't have institutions in common
				k++;
			}
			else if (node1.AuthID == node2.AuthID){
				Node* node = new Node();
				node->AuthID = node1.AuthID;
				node->count = node1.count + node2.count; // assuming the two tree nodes don't have institutions in common
				node->deg = node1.deg;
				v->Add(*node);
				j++;
				k++;

			}
			else if (node1.AuthID < node2.AuthID){
				Node* node = new Node(); // need not do it, can optimize by reusing
				node->AuthID = node1.AuthID;
				node->count = node1.count; // assuming the two tree nodes don't have institutions in common
				node->deg = node1.deg;
				v->Add(*node);
				j++;

			}
			else if (node1.AuthID > node2.AuthID){
				Node* node = new Node(); // need not do it, can optimize by reusing
				node->AuthID = node2.AuthID;
				node->count = node2.count; // assuming the two tree nodes don't have institutions in common
				node->deg = node2.deg;
				v->Add(*node);
				k++;

			}
		}
		while(j < len1){
			Node node1 = (*v1)[j];
			if(node1.count == node1.deg){ // assuming the two tree nodes don't have institutions in common else we should also remove in other list
				j++;
			}
			else {
				Node* node = new Node();
				node->AuthID = node1.AuthID;
				node->count = node1.count; // assuming the two tree nodes don't have institutions in common
				node->deg = node1.deg;
				v->Add(*node);
				j++;
			}
		}
		while(k < len2){
			Node node2 = (*v2)[k];
			if(node2.count == node2.deg){ // assuming the two tree nodes don't have institutions in common else we should also remove in other list
				k++;
			}
			else {
				Node* node = new Node();
				node->AuthID = node2.AuthID;
				node->count = node2.count; // assuming the two tree nodes don't have institutions in common
				node->deg = node2.deg;
				v->Add(*node);
				k++;
			}
		}
	}

}


TStr get_hash_key(int AuthID, PNGraph G, int index_start, int index_end){
	TNGraph::TNodeI NI = G->GetNI(AuthID);
	TStr key = TStr("");
	for (int i = index_start; i < index_end; i++){
		key += TStr(TInt::GetStr(NI.GetOutNId(i)));
		key += "-";
	}
	return key;
}

void print(TVec<Node> *v){
	for (int i = 0; i < v->Len(); i ++){
		Node node = (*v)[i];
		cout<<" START "<<node.AuthID<<" "<<node.count<<" "<<node.deg<<" END    ";
	}
	cout<<endl;
}

void create_tree_node(int AuthID, int index_start, int index_end, PNGraph Graph,  TVec<Node> *v){
	TStr key = get_hash_key(AuthID, Graph, index_start, index_end); //see if it comes correctly
	  cerr<<key.CStr()<<"\n";
	  if(tree_node_hash.IsKey(key)){
		  *v = tree_node_hash.GetDat(key);
	  }
	  else if(index_end-index_start == 1){

		  TNGraph::TNodeI NI = Graph->GetNI(AuthID);
		  v = new TVec<Node>();

		  int inst = NI.GetOutNId(index_start);
		  TNGraph::TNodeI Inst_NI = Graph->GetNI(inst);
		  for ( int i = 0; i < Inst_NI.GetInDeg(); i++){
			  int auth = Inst_NI.GetInNId(i);
			  Node * node = new Node();
			  node->AuthID = auth;
			  node->count = 1;
			  node->deg = (Graph->GetNI(auth)).GetOutDeg();
			  v->Add(*node);

		  }
		  tree_node_hash.AddDat(key,*v);

	  }
	  else{
		  TVec<Node> *v1, *v2;
		  create_tree_node(AuthID, index_start, index_end-1, Graph, v1);
		  create_tree_node(AuthID, index_end-1, index_end, Graph, v2);
		  v = new TVec<Node>();
		  v->Reserve(v1->Len());

		  merge(v1,v2,v);
		  print(v1);
		  print(v2);
		  print(v);
		  tree_node_hash.AddDat(key,*v);
	  }
}

int main(int argc,char* argv[]) {
  ////////////// Load Graph /////////////
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
  cerr<<currentDateTime()<<endl;
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
  //cerr<<"graph constructed"<<currentDateTime()<<"\n";
  cerr<<Graph->GetNodes()<<endl;

  /////// Sort the authors by degree //////////
  vector< pair <int,int> > auth_deg;
  for (TNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++){
	  if (NI.GetOutDeg() > 0)
		  auth_deg.push_back(pair<int,int>(NI.GetOutDeg(), NI.GetId()));
  }
  sort(auth_deg.begin(), auth_deg.end());
  ofstream deg_fil;
  deg_fil.open("author_degree.txt",ios::out);
  int num_auth = auth_deg.size();
  for (int i = 0; i < num_auth; i++)
	  deg_fil<<auth_deg[i].first<< " "<< auth_deg[i].second<<"\n";
  deg_fil.close();
  cerr<<"authors sorted by degree"<<currentDateTime()<<"\n";
  ///////////////////////////// Create tree nodes and compute neighbors///////////////////////////////////
  int deg_prev = 0;
  for (int i = 0; i < num_auth; i++){
	  if (auth_deg[i].first != deg_prev){
		  deg_prev = auth_deg[i].first;
		  cerr<< deg_prev<<endl;
	  }
	  if (i%10000 == 0){
		  cout<< i <<" "<<currentDateTime()<<endl;
	  }
	  TVec<Node>* v;
	  create_tree_node(auth_deg[i].second, 0, auth_deg[i].first, Graph, v);
//	  TStr key = get_hash_key(auth_deg[i].second, Graph, 0, auth_deg[i].first); //see if it comes correctly
//	  cerr<<TStr<<"\n";
//	  if(!tree_node_hash.IsKey(key)){
//		  TVec<Node>* v = new TVec<Node>();
//		  create_node(auth_deg[i].second, auth_deg[i].first, Graph, v);
//		  tree_node_hash.AddDat(key, *v);
//	  }
  }
  TFOut SOut("tree_node_hash.bin");
  tree_node_hash.Save(SOut);
  return 0;
}
  /////////////////////////////////////////////////////////////////

