#include <vector>
#include <list>
#include <set>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>

#include "JSON.h"

#define INF 1000000

using namespace std;

// if ref is false then we're mapping node a to node b
// otherwise we're referring to the mapping from subtree a to subtree b
struct mapT {
  int a;
  int b;
  bool ref;
};

enum matchT {DELA, DELB, MATCH};
struct resultT {
  resultT(matchT match = MATCH, int cost = INF) : match(match), cost(cost), map() {}
  matchT match;
  int cost;
  vector<mapT> map;
};

struct nodeT {
  int id;
  wstring type;
  wstring name;
  int leaf;
  bool key;
  int size;
};

typedef vector<resultT> VR;
typedef vector<VR> VVR;
typedef vector<int> VI;
typedef vector<VI> VVI;
typedef vector<JSONValue*> VJ;
typedef vector<bool> VB;
typedef vector<nodeT> VN;

VVR dp, dp2;
VN postA, postB;

set<wstring> important;

JSONValue* readJSON(const char* filename) {
  ifstream in(filename);
  in.seekg(0, ios::end);
  int length = in.tellg();
  in.seekg(0, ios::beg);
  char* buffer = new char[length+1];
  in.read(buffer, length);
  in.close();
  buffer[length] = '\0';
  JSONValue* ans = JSON::Parse(buffer);
  delete[] buffer;
  return ans;
}

int getSize(JSONValue* root) {
  while (root->AsObject().at(L"children")->AsArray().size() > 0)
    root = root->AsObject().at(L"children")->AsArray().back();
  return static_cast<int>(root->AsObject().at(L"id")->AsNumber() + 1);
}

nodeT makeNode(JSONValue* node, int leaf, bool key, int size) {
  nodeT n;
  n.type = node->AsObject().at(L"type")->AsString();  
  
  if (node->AsObject().count(L"name") == 0) {
    n.name = L"";
  } else {
    n.name = node->AsObject().at(L"name")->AsString();
  }
  
  if (n.type == L"IDENT" && important.find(n.name) == important.end()) n.name = L"";
  
  n.id = static_cast<int>(node->AsObject().at(L"id")->AsNumber());
  
  n.leaf = leaf;
  n.key = key;
  n.size = size;
  
  return n;
}

int postorder(JSONValue* node, VN& post, bool isKey) {
  const JSONArray& children = node->AsObject().at(L"children")->AsArray();
  int ans = -1;
  int size = 1;
  for (int i = 0; i < children.size(); i++) {
    int tmp = postorder(children[i], post, i > 0);
    size += post[post.size() - 1].size;
    if (ans == -1) ans = tmp;
  }
  if (ans == -1) ans = post.size();
  post.push_back(makeNode(node, ans, isKey, size));
  return ans;
}

inline int match(const nodeT& a, const nodeT& b) {
  if (a.type != b.type) return INF;
  if (a.name != b.name) return 1;
  return 0;
}


void matchTree(const VN& A, const VN& B, int a, int b) {
  dp2[0][0].cost = 0;
  int leafA = A[a].leaf, leafB = B[b].leaf;
  for (int i = leafA; i <= a; i++) {
    dp2[i-leafA+1][0].cost = dp2[i-leafA][0].cost + 1;
    dp2[i-leafA+1][0].match = DELA;
  }
  for (int j = leafB; j <= b; j++) {
    dp2[0][j-leafB+1].cost = dp2[0][j-leafB].cost + 1;
    dp2[0][j-leafB+1].match = DELB;
  }
  
  for (int i = leafA; i <= a; i++) {
    for (int j = leafB; j <= b; j++) {
      int ai = i - leafA + 1, bj = j - leafB + 1;
      int leafI = A[i].leaf, leafJ = B[j].leaf;
      if (leafI == leafA && leafJ == leafB) {
        dp2[ai][bj].cost = dp2[ai-1][bj].cost + 1;
        dp2[ai][bj].match = DELA;
        if (dp2[ai][bj].cost > dp2[ai][bj-1].cost + 1) {
          dp2[ai][bj].cost = dp2[ai][bj-1].cost + 1;
          dp2[ai][bj].match = DELB;
        }
        int m = match(A[i], B[j]);
        if (dp2[ai][bj].cost > dp2[ai-1][bj-1].cost + m) {
          dp2[ai][bj].cost = dp2[ai-1][bj-1].cost + m;
          dp2[ai][bj].match = MATCH;
        }
        dp[i][j] = dp2[ai][bj];
      } else {
        dp2[ai][bj].cost = dp2[ai-1][bj].cost + 1;
        dp2[ai][bj].match = DELA;
        if (dp2[ai][bj].cost > dp2[ai][bj-1].cost + 1) {
          dp2[ai][bj].cost = dp2[ai][bj-1].cost + 1;
          dp2[ai][bj].match = DELB;
        }
        if (dp2[ai][bj].cost > dp2[leafI - leafA][leafJ - leafB].cost + dp[i][j].cost) {
          dp2[ai][bj].cost = dp2[leafI - leafA][leafJ - leafB].cost + dp[i][j].cost;
          dp2[ai][bj].match = MATCH;
        }
      }
    }
  }
  
  for (int ci = leafA; ci <= a; ci++) {
    for (int cj = leafB; cj <= b; cj++) {
      if (A[ci].leaf != leafA || B[cj].leaf != leafB) continue;
      int i = ci, j = cj;
      while (i >= leafA || j >= leafB) {
        int ai = i - leafA + 1, bj = j - leafB + 1;
        switch (dp2[ai][bj].match) {
          case DELA: i--; break;
          case DELB: j--; break;
          case MATCH:
            mapT mapEntry;
            if (A[i].leaf == leafA && B[j].leaf == leafB && i == ci && j == cj) {
              mapEntry.a = A[i].id;
              mapEntry.b = B[j].id;
              mapEntry.ref = false;
              i--;
              j--;
            } else {
              mapEntry.a = i;
              mapEntry.b = j;
              mapEntry.ref = true;
              i -= A[i].size;
              j -= B[j].size;
            }
            dp[ci][cj].map.push_back(mapEntry);
        }
      }
    }
  }
}

void populateMatching(map<int, int> &matching, resultT &res) {
  for (unsigned int i = 0; i < res.map.size(); i++) {
    if (res.map[i].ref) {
      populateMatching(matching, dp[res.map[i].a][res.map[i].b]);
    } else {
      matching[res.map[i].a] = res.map[i].b;
    }
  }
}

int main(int argc, char** argv) {
  if (argc != 4) {
    cout << "Usage: matching src_ast dst_ast builtins" << endl;
    return 1;
  }
  
  JSONValue* A = readJSON(argv[1]);
  JSONValue* rootA = A->AsObject().at(L"root");
  JSONValue* B = readJSON(argv[2]);
  JSONValue* rootB = B->AsObject().at(L"root");
  
  wifstream keywords(argv[3]);
  wstring keyword;
  while (keywords >> keyword) important.insert(keyword);
  keywords.close();
  
  int na = getSize(rootA), nb = getSize(rootB);
  
  dp = VVR(na, VR(nb));
  dp2 = VVR(na+1, VR(nb+1));
  
  postorder(rootA, postA, true);
  postorder(rootB, postB, true);
  
  for (int i = 0; i < na; i++) {
    if (!postA[i].key) continue;
    for (int j = 0; j < nb; j++) {
      if (!postB[j].key) continue;
      matchTree(postA, postB, i, j);
    }
  }
  
  int ans = dp[na-1][nb-1].cost;
  
  map<int, int> matching;
  populateMatching(matching, dp[na-1][nb-1]);
  cout << ans << " " << matching.size() << endl;

  for (map<int, int>::iterator it = matching.begin(); it != matching.end(); ++it) {
    cout << it->first << " " << it->second << endl;
  }
  return 0;
}

