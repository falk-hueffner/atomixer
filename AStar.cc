/*
  atomixer -- Atomix puzzle solver
  Copyright (C) 2000 Falk Hueffner

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.
  
  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.
  
  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59 Temple
  Place, Suite 330, Boston, MA 02111-1307 USA

  $Id$
*/

#ifdef HAVE_STDINT_H
# include <stdint.h>
#else
typedef long long int64_t;
#endif

#include <deque>
#include <iostream>
#include <queue>
#include <set>

#include "AStar.hh"
#include "State.hh"
#include "State.hh"

using namespace std;

class Node;

typedef set<Node>::iterator NodePtr;

class Node {
public:
    Node() { } // to make STL happy
    Node(const State& nstate, const NodePtr npredecessor, int ndist,
	 int nminTotalDist)
	: state(nstate), predecessor(npredecessor), dist(ndist),
	  minTotalDist(nminTotalDist)  { }

    bool operator <(const Node& other) const { // for finding in the nodeSet
	return state < other.state;
    }

    State state;
    mutable NodePtr predecessor;
    mutable int dist;
    mutable int minTotalDist;

    friend ostream& operator<<(ostream& out, const Node& n);
};

class QueueNode {
public:
    QueueNode() { } // to make STL happy
    QueueNode(NodePtr nnode)
	: node(nnode), minTotalDist(nnode->minTotalDist) { }
    NodePtr node;
    int minTotalDist;

    bool operator <(const QueueNode& other) const {
	return minTotalDist > other.minTotalDist;
    }

    friend ostream& operator <<(ostream& out, const QueueNode& n);
};

deque<Move> aStar(const State& start, int maxDist) {
    static int64_t totalNodes = 0;
    long counter = 0;
    long stales = 0;

    set<Node> nodes;
    priority_queue<QueueNode> open;

    const NodePtr pStartNode = nodes.insert(Node(start, nodes.end(), 0,
						 start.minMovesLeft())).first;
						 //problem.minCostLeft(start))).first;

    //cout << "start node:\n" << *pStartNode << endl;
    open.push(QueueNode(pStartNode));
    ++totalNodes;

    while (!open.empty()) {
	QueueNode bestNode = open.top();
	open.pop();

	if (counter++ % 10000 == 0 || 0)
	    cout << "best:\n" << bestNode
		 << "\nnodes: " << nodes.size()
		 << "\nqueuenodes: " << open.size()
		 << "\nstales: " << stales
		 << endl;

	if (bestNode.minTotalDist > bestNode.node->minTotalDist && 0) {
	    // a better path to this state has already been examined
	    --stales;
	    //cout << "obsolete, disposing (stale queue members: " << stales << ")\n";
	     continue;
	}

	//deque<Problem::Move> moves = bestNode.node->state.moves();
	vector<Move> moves = bestNode.node->state.moves();
	for (vector<Move>::const_iterator m = moves.begin();
	     m != moves.end(); ++m) {
	    State newState(bestNode.node->state, *m);
	    int minMovesLeft = newState.minMovesLeft();
	    if (minMovesLeft == 0) { // special property of our heuristic...
		cout << "Found solution.\n"
		     << "\nnodes: " << nodes.size()
		     << "\nqueuenodes: " << open.size()
		     << "\ntotal nodes: " << totalNodes
		     << "\nstales: " << stales
		     << endl;
		deque<Move> solution;
		NodePtr pNode = bestNode.node, pNextNode;

		while (pNode != pStartNode) {
		    pNextNode = pNode;
		    pNode = pNode->predecessor;

		    vector<Move> moves = pNode->state.moves();

		    for (vector<Move>::const_iterator m = moves.begin();
			 m != moves.end(); ++m) {

			if (State(pNode->state, *m) == pNextNode->state) {
			    solution.push_front(*m);
			    break;
			}
		    }
		}
		solution.push_back(*m);
		return solution;
	    }
	    Node newNode(newState, bestNode.node,
			 bestNode.node->dist + 1, //(*m).cost(),
			 bestNode.node->dist + 1 /*(*m).cost() */ + minMovesLeft);
	    
	    if (newNode.minTotalDist > maxDist)
		continue;
	    //if (perfectHeuristic && newNode.minTotalDist > maxDist)
	    //continue;
	    //if (!perfectHeuristic && newNode.dist > maxDist)
	    //continue;
	    //cout << "adding\n" << newNode << endl;

	    NodePtr pNewNode = nodes.find(newNode);

	    if (pNewNode != nodes.end()) {
		if (newNode.minTotalDist >= pNewNode->minTotalDist) {
		    //cout << "Old path to this state better, disposing\n";
		    continue;
		} else {
		    ++stales;
		    //cout << "Adapting existing set entry (stale queue members: " << stales << ")\n";
		    // these all doesn't matter with regard to operator <, so
		    // it doesn't break the set changing them
		    pNewNode->predecessor = newNode.predecessor;
		    pNewNode->dist = newNode.dist;
		    pNewNode->minTotalDist = newNode.minTotalDist;
		    // the old priority queue reference remains and will be
		    // disposed when it gets to the top
		}
	    } else {
		pNewNode = nodes.insert(newNode).first;
	    }
	    
	    open.push(QueueNode(pNewNode));
	    ++totalNodes;
	}
    }

    cout << "Queue empty; no solution possible.\n";

    return deque<Move>();
}

ostream& operator <<(ostream& out, const Node& n) {
    return out << "Node {\n" << n.state << "\ndist = " << n.dist
	       << ", minTotalDist = " << n.minTotalDist << " }";
}

ostream& operator <<(ostream& out, const QueueNode& n) {
    return out << "QueueNode {\n" << n.node->state << "\ndist = " << n.node->dist
	       << ", minTotalDist = " << n.minTotalDist
	       << " (" << n.node->minTotalDist << ") }";
}
