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

#ifndef HASH_TABLE_HH
#define HASH_TABLE_HH

#include <vector>

using namespace std;

// a hash table with linear probing.
template<typename Element>
class HashTable {
public:
    typedef typename vector<Element>::iterator Iterator;
    typedef typename vector<Element>::const_iterator ConstIterator;

    Iterator begin() { return elements.begin() + 1; }
    Iterator end() { return elements.end(); }

    ConstIterator begin() const { return elements.begin() + 1; }
    ConstIterator end() const { return elements.end(); }

    size_t size() const { return elements.size() - 1; }
    size_t capacity() const { return elements.capacity() - 1; }
    
    HashTable(int numElements, double nloadFactor) {
	clear(numElements, nloadFactor);
    }

    HashTable() {
	clear(256, 1.5);
    }

    void clear(int initialSize, double nloadFactor = 1.5) {
	loadFactor = nloadFactor;
	elements.clear();
	cout << "Reserving space for " << initialSize + 1 << " elems.\n";
	elements.reserve(initialSize + 1);
	cout << "capacity is: " << elements.capacity() << endl;
	cout << "capacity is: " << capacity() << endl;
	elements.push_back(Element()); // 0 reserved for 'empty'
	hashTable.clear();
	hashTable.resize(int((initialSize + 1) * loadFactor));
    }

    Element* find(const Element& element) { // should be const...
	int hash = element.hash() % hashTable.size();
	while (true) {
	    if (hashTable[hash] == 0) 
		return NULL;
	    else if (elements[hashTable[hash]] == element)
		return &elements[hashTable[hash]];
	    if (++hash >= hashTable.size())
		hash = 0;
	}	
    }

    /*
    ConstIterator find(const Element& element) const {
	int hash = element.hash() % hashTable.size();
	while (true) {
	    if (hashTable[hash] == 0) 
		return end();
	    else if (elements[hashTable[hash]] == element)
		return elements.begin() + hashTable[hash];
	    if (++hash >= hashTable.size())
		hash = 0;
	}
    }
    */

    void insertNew(const Element& element) {
	int hash = element.hash() % hashTable.size();
	while (true) {
	    if (hashTable[hash] == 0) {
		elements.push_back(element);
		if (elements.size() * loadFactor > hashTable.size())
		    rehash();
		hashTable[hash] = elements.size() - 1;
		return;
	    }
	    if (++hash >= hashTable.size())
		hash = 0;
	}
    }

    void insert(const Element& element) {
	int hash = element.hash() % hashTable.size();
	while (true) {
	    if (hashTable[hash] == 0) {
		elements.push_back(element);
		if (elements.size() * loadFactor > hashTable.size())
		    rehash();
		hashTable[hash] = elements.size() - 1;
		return;
	    } else {
		if (element == elements[hashTable[hash]]) {
		    elements[hashTable[hash]].update(element);
		    return;
		}
	    }
	    if (++hash >= hashTable.size())
		hash = 0;
	}
    }

    void insertIfBetter(const Element& element) {
	int hash = element.hash() % hashTable.size();
	while (true) {
	    if (hashTable[hash] == 0) {
		elements.push_back(element);
		if (elements.size() * loadFactor > hashTable.size())
		    rehash();
		hashTable[hash] = elements.size() - 1;
		return;
	    } else {
		Element& oldElement = elements[hashTable[hash]];
		if (oldElement == element) {
		    if (element.better(oldElement))
			oldElement.update(element);
		    else
			return;
		}
	    }
	    if (++hash >= hashTable.size())
		hash = 0;
	}
    }

    void rehash() {
	int newSize = int(elements.size() * loadFactor * 2.0);
	hashTable.clear();
	hashTable.resize(newSize);

	for (int i = 1; i < elements.size(); ++i) {
	    unsigned int hash = elements[i].hash() % hashTable.size();
	    while (hashTable[hash] != 0)
		if (++hash >= hashTable.size())
		    hash = 0;

	    hashTable[hash] = i;
	}
    }

private:
    double loadFactor;

    vector<Element> elements;
    vector<int> hashTable;
};

#endif
