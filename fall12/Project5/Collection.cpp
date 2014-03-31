#include "Collection.h"

Collection::Collection(void)
{
	size_ = 0;
}
Collection::Collection(int n)
{
	size_ = n;
}



Collection::~Collection(void)
{
	//made no use of new operator.
}

Collection* Collection::copy()
{
	return this;
}

bool Collection::contains(int a)
{
	int* temp = this->getCollection();	//retrieves integer array from base class.
	for(int i=0; i < size() ; i++)
		if(temp[i] == a) 
			return true;
	
	return false;
}


void Collection::iterate(void (*userFunction) (int))
{
	int* temp = this->getCollection();

	for(int i=0; i < size() ; i++)
		(*userFunction)(temp[i]);
}



// CLASS ORDEREDCOLLECTION


OrderedCollection::OrderedCollection(void) : Collection()
{
	first = 0;
	last = 0;
	oc= new int[4];	// initializes the vector.
	size_= 4;
	isEmpty = true;

}

OrderedCollection::OrderedCollection(const OrderedCollection& r) : Collection()
{
		
		oc = new int[r.size_];
		this->first = r.first;
		this->last = r.last;
		this->size_ = r.size_;
		this->isEmpty = r.isEmpty;
		for(int i = 0; i< r.size_ ; i++)
			this->oc[i] = r.oc[i];
		
}

OrderedCollection::~OrderedCollection(void)
{
	delete [] oc;
}

OrderedCollection* OrderedCollection::copy()
{
	OrderedCollection* temp = new OrderedCollection(*this);
	return temp;
}

void OrderedCollection::grow()
{
	int a = 2 * size_ ;
	int* temp = new int[a] ;
	for(int i =0;i < size_ ; i++)
		temp[i] = oc[i];
	delete[] oc;
	size_ = a; 
	oc = new int[size_];

	for(int i =0;i < size_ ; i++)
		oc[i] = temp[i];
	delete[] temp;
	
}

void OrderedCollection::add(int a, int i)
{
	if(isEmpty && i!=0)
	{
		cout<<"Please insert 1st element at 0 index"<<endl;
	}
	else if(i>last+1 || i < first - 1)
		cout<<"WARNING: index out of bound. elements can only be stored in a contiguous manner"<<endl;
	else
	{
		if( size_ == (last-first+1))
			grow(); // doubles the allocated size for the elements.

		if(last == 0 && i == 0 && isEmpty)
		{
			oc[i] = a;
			isEmpty = false;
		}
		else if((i == first - 1 && i >=0))	//if element is inserted before first
		{
			oc[i]=a;
			if(i != 0)
				first -= 1;

		}
		else if((i == last+1 && i <= size_))	//if element is inserted after last
		{
			this->oc[i] = a;
			last += 1;
		}
		else
		{
			for( int j =last; j >=i ; j--)	//make room for incoming element
				oc[j+1] = oc[j];

			oc[i] = a;
			last += 1;
		}
	}
}


void OrderedCollection::remove(int a)
{
	bool present = false;
	int pos = -1;
	for(int i = first; i<= last; i++)
		if(oc[i] == a)
		{
			present = true;
			pos = i;
			break;
		}
	if(present)
	{
		
		for(int i = pos ; i<= last; i++)
			oc[i] = oc[i+1];
		last -= 1;
		if(last<0)// check if removed element was the last element in the ordered collection.
		{
			last += 1;
			isEmpty = true;
		}
	}
}



int& OrderedCollection::operator[] (const int i)
{
	if(isEmpty)
	{
		cout<<"WARNING: Empty Collection";
		//int a = -1000; 
		//return a;	This is incompatible with gcc compiler
	}
	else if(i<first || i > last )	// index 0 does not exist and element at 1 returns the First element in the collection.
	{
		cout<<" WARNING : The index does not exist in this OrderedCollection. [Index out of Bounds].";
		//int a = -1000; 
		//return a;	This is incompatible with gcc compiler
	}
		
	else
	{
		if(!isEmpty)
			return oc[i];
	}
	}


OrderedCollection& OrderedCollection::operator=(Collection& rhs)
{	 	
	//(OrderedCollection&) rhs;
	OrderedCollection* r;	// declared on stack of this function since it is not required after the function returns.
	
	r = dynamic_cast<OrderedCollection*>(&rhs);	//returns NULL if casting does not take place.
	
	if(r != NULL)	// check if the cast was correct.
	{
		if(this == r)	// check to see if user passes the same class as the one to be copied.. eg rhs->Base::operator=rhs. if this is the case then assignment is not required.
			return *this;	// no copying takes place.
		else
		{
			// allocates the same space as the rhs.
			delete[] oc;
			oc = new int[r->size_];	
			
			// deep copy.
			this->first = r->first;
			this->last = r->last;
			this->size_ = r ->size_;
			this->isEmpty = r->isEmpty;

			for(int i = 0; i < size_; i++)
				this->oc[i] = r->oc[i];
			
			return *this;	
		}
	}
	return *this;
}

int OrderedCollection::size()
{
	int a = last - first + 1;
	return a;
}

 int* OrderedCollection::getCollection()
{
	return oc;
}






	//CLASS ARRAYCOLLECTION


ArrayCollection::ArrayCollection(void)
{
	
	ac = new int[20];		//if size of array is not mentioned then by default an array of size 20 is created.
	for(int i = 0; i<20 ; i++)
		ac[i] = 0;
	size_ = 20;
	cout<<"Array created with default size= 20";
	
}

ArrayCollection::ArrayCollection(int n) : Collection(n)
{
	ac = new int[n];		//array of size n is created.
	for(int i = 0; i<n ; i++)
		ac[i] = 0;
	//size_ = n; This is initialized in Collection constructor called in initialization list.
}

ArrayCollection::ArrayCollection(const ArrayCollection& r) : Collection()
{
	// copy constructor.
		ac = new int[r.size_];
		this->size_ = r.size_;
		for(int i = 0; i< r.size_ ; i++)
			this->ac[i] = r.ac[i];
}


ArrayCollection::~ArrayCollection(void) 
{
	delete[] ac;	//deallocates ac.
}

void ArrayCollection::add(int, int)
{
	cout<<"You cannot add an element with this method to an array."<<endl;

}

void ArrayCollection::remove(int)
{
	cout<<"You cannot remove an element with this method to an array."<<endl;
}

ArrayCollection* ArrayCollection::copy()
{
	ArrayCollection* temp = new ArrayCollection(*this);
	return temp;
	// this temp needs to be deallocated by the programmer explictly.
}


int& ArrayCollection::operator[] (int i)
{
	if(i<0 || i >= size_ )
	{
		cout<<" WARNING : The index does not exist in this OrderedCollection. [Index out of Bounds].";
		//int a = -1000; 
		//return a;	This is incompatible with gcc compiler
	}
	else
		return ac[i];
}

ArrayCollection& ArrayCollection::operator= (Collection& rhs)
{	 	
	ArrayCollection* r;	// declared on stack of this function since it is not required after the function returns.
	r = dynamic_cast<ArrayCollection*>(&rhs);	//returns NULL if improper object is passed.
	
	if(r != NULL)	// check if the cast was correct.
	{
		if(this == r)	// check to see if user passes the same class as the one to be copied.. eg rhs->Base::operator=rhs. if this is the case then assignment is not required.
			return *this;	// no copying takes place.
		else
		{
			// allocates the same space as the rhs.
			delete[] ac;	// deallocates as since we are copying to an already existent object.
			ac = new int[r->size_];	
			
			// deep copy.
			
			this->size_ = r ->size_;

			for(int i = 0; i < size_; i++)
				this->ac[i] = r->ac[i];
				
			return *this;	
		}
	}
	return *this;
}

int ArrayCollection::size()
{
	return size_;
}

int* ArrayCollection::getCollection()
{
	return ac;
}

