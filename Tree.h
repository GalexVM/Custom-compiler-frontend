#pragma once

#include <vector>
#include <string>
using namespace std;

struct token {
    string parseValue;
    string originalValue;
    token(string v1, string v2) :parseValue(std::move(v1)), originalValue(std::move(v2)) {}
    token() = default;
};

struct TokenComparator {
    bool operator()(const token& t1, const token& t2) const {
        return t1.parseValue < t2.parseValue;
    }
};

class NodeTree {
public:
    int numHijos = 0;
    unsigned int priority = 0;
    string type = "";
    bool isfirst = false;
    vector<NodeTree*>* hijos;
    NodeTree* padre = nullptr;
    token name;
    NodeTree(string n) { name.parseValue = n;
        hijos = new vector<NodeTree*>;
    }
    NodeTree (token n):name(n){
        hijos = new vector<NodeTree*>;
    }
    ~NodeTree() { for (auto i : *hijos) delete i; delete hijos;}
    void addChild(string n);
    void addChild(token n);
    int getNumBrothers();
    void printNode(const string& prefix, bool _isfirst);
    void preOrderTraversal(NodeTree* node);

};

class Tree {
public:
    NodeTree* root = nullptr;
    NodeTree* piv = nullptr;
    string name = "";
    void printTree();
    void preOrderTraversal();
    Tree(string n):name(n) { root = new NodeTree(name); piv = root; }
    ~Tree() { delete root; }
};


void Tree::printTree()
{
    root->printNode("", true);
}
void Tree::preOrderTraversal()
{
    root->preOrderTraversal(root);
}
void NodeTree::addChild(string n)
{
    NodeTree* nt = new NodeTree(n);
    this->hijos->push_back(nt);
    nt->padre = this;
    if (this->numHijos == 0) nt->isfirst = true;
    this->numHijos++;
    if (nt->name.parseValue == "+" || nt->name.parseValue == "-")
        nt->priority = 1;
    else if (nt->name.parseValue == "*" || nt->name.parseValue == "/")
        nt->priority = 2;
    else if(nt->name.parseValue == "%")
        nt->priority = 3;
    if(nt->name.parseValue == "NUMBER" || nt->name.parseValue == "FLOAT" || nt->name.parseValue == "STRING" || nt->name.parseValue == "ID"){
        nt->type = "variable";
    }

}
void NodeTree::addChild(token n)
{
    NodeTree* nt = new NodeTree(n);
    this->hijos->push_back(nt);
    nt->padre = this;
    if (this->numHijos == 0) nt->isfirst = true;
    this->numHijos++;
    if (nt->name.parseValue == "*" || nt->name.parseValue == "/")
        nt->priority = 1;
    else if(nt->name.parseValue == "%")
        nt->priority = 2;
    if(nt->name.parseValue == "NUMBER" || nt->name.parseValue == "FLOAT" || nt->name.parseValue == "STRING" || nt->name.parseValue == "ID"){
        nt->type = "variable";
    }
}
inline int NodeTree::getNumBrothers()
{
    return this->padre->hijos->size()-1;
}
inline void NodeTree::printNode(const string& prefix, bool _isfirst)
{
    cout << prefix;
    cout << (_isfirst ? "|--" : "L--");
    // print the value of the node
    cout << ((!name.originalValue.empty()) ? name.originalValue : name.parseValue) << endl;
    if (numHijos > 0) {
        //Imprimimos a la inversa porque se guardaron así
        for (int i = hijos->size() - 1; i > 0; i--)
        {
            (*hijos)[i]->printNode(prefix + (_isfirst ? "|   " : "    "), true);
        }
        (*hijos)[0]->printNode(prefix + (_isfirst ? "|   " : "    "), false);
    }
}
void NodeTree::preOrderTraversal(NodeTree* node) {
    if (node != nullptr)
    {
        /*if (node->name == ">" || node->name == "<" || node->name == "<=" || node->name == ">=" ||
            node->name == "==" ||)*/
        cout << node->name.parseValue << "\n"; // Realiza la operación deseada con el nodo (en este caso, imprimir el nombre).

        for (NodeTree* child : *node->hijos)
        {
            preOrderTraversal(child); // Llama recursivamente a la función para cada hijo.
        }
    }
}
