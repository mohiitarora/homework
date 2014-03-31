
#ifndef Collection_H
#define Collection_H
#endif

#include <iostream>
#include <vector>
#include <string> 
#include <stdlib.h>
#include <algorithm>

using namespace std;

class Collection
{
public:
	int size_;

	Collection();
	Collection(int);
	
	virtual ~Collection(void);
	
	virtual void add(int, int) = 0;			//pure virtual function
	virtual void remove(int) = 0;		//pure virtual function
	virtual Collection& operator=(Collection&) = 0;	//pure virtual function
	virtual int& operator[](int) = 0;	//pure virtual function. Returns integer by reference so that it can be used as an l-value.

	virtual int size() = 0;
	
	bool contains(int); 
	void iterate(void (*userFunction) (int));

	virtual Collection* copy();

	
	
	virtual int* getCollection()=0;
	

};


// CLASS ORDEREDCOLLECTION



class OrderedCollection :
	public Collection
{
public:
	int first, last;
	int* oc;

	OrderedCollection(void);
	OrderedCollection(const OrderedCollection&);//deep copy
	~OrderedCollection(void);

	virtual void add(int, int);
	virtual void remove(int);
	virtual OrderedCollection& operator=(Collection&);	//deep copy
	virtual int& operator[] (int); //Returns integer by reference so that it can be used as an l-value.

	virtual OrderedCollection* copy();

	virtual int size();
	void grow();
	virtual int* getCollection();
	

	private: int isEmpty; 
};


//CLASS ARRAYCOLLECTION


class ArrayCollection :
	public Collection
{
public:
	int* ac;

	ArrayCollection(void);
	ArrayCollection(int);
	ArrayCollection(const ArrayCollection&);
	~ArrayCollection(void);

	virtual void add(int,int) ;		
	virtual void remove(int);	
	virtual ArrayCollection& operator=(Collection&);
	virtual int& operator[](int) ;

	virtual ArrayCollection* copy();

	virtual int size();

	virtual int* getCollection();
	
};




