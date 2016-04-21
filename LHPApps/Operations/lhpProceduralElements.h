#include "vtkMath.h"

typedef double point[3];

typedef struct
{
	vtkIdType key;
	point x;
} Node;


class Element 
{
public:
	Element(int nNodes);
	~Element();
	void SetKey(vtkIdType key) { this->m_Key = key; };
	void GetKey(vtkIdType &key) { key = this->m_Key; };
	vtkIdType GetKey() { return  this->m_Key; };
	int SetNode(vtkIdType id, Node *node);  
	int GetNode(vtkIdType id, Node *node);
	Node *GetNode(vtkIdType id) { return m_Nodes[id];};
	void SetNodes(Node **nodes) { this->m_Nodes = nodes;};
	virtual vtkIdType GetNumberOfNodes() {return m_NodesNumber;};
	virtual void Integrate(double (*integrand)(double[3]), int numSteps, double &integral, double &volume) = 0;
	virtual int GetType() = 0;
	void SetMatKey(vtkIdType key) { this->m_MatKey = key; };
	vtkIdType GetMatKey() { return  this->m_MatKey; };

protected:
	int m_NodesNumber;
	vtkIdType m_Key;  
	vtkIdType m_MatKey;
	Node **m_Nodes;
};

class Tetra : public Element 
{
public:
	Tetra():Element(4){};
	static Tetra *New() { return new Tetra; };
	void Integrate(double (*integrand)(double[3]), int numSteps, double &integral, double &volume);
	int GetType() { return 5;};
};

class Tetra10: public Element 
{
public:
	Tetra10():Element(10){};
	static Tetra10 *New() { return new Tetra10; };
	void Integrate(double (*integrand)(double[3]), int numSteps, double &integral, double &volume);
	int GetType() { return 5;};
};

class Wedge: public Element 
{
public:
	Wedge():Element(6){};
	static Wedge *New() { return new Wedge; };
	void Integrate(double (*integrand)(double[3]), int numSteps, double &integral, double &volume);
	int GetType() { return 7;};
};

class Hexa: public Element {
public:
	Hexa():Element(8){};
	static Hexa *New() { return new Hexa; };
	void Integrate(double (*integrand)(double[3]), int numSteps, double &integral, double &volume);
	int GetType() { return 8;};
};


