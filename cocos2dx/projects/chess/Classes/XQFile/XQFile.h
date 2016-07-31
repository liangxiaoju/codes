#ifndef __XQFILE_H__
#define __XQFILE_H__

#include <iostream>
#include <vector>

class XQHeader
{
public:
    int type;
    std::string title;
    std::string matchName;
    std::string matchAddr;
    std::string whiteName;
    std::string blackName;
    std::string author;
    std::string annotator;
    std::string ecco;
    std::string result;
    std::string fen;
};

class XQNode
{
public:
    std::string mv;
    std::string comment;

    XQNode()
    {
        pParent = nullptr;
        pFirstChild = nullptr;
        pCurChild = nullptr;
        pLeft = nullptr;
        pRight = nullptr;
    }

    virtual ~XQNode()
    {

    }

protected:
    XQNode *pParent;
    XQNode *pFirstChild;
    XQNode *pCurChild;
    XQNode *pLeft;
    XQNode *pRight;

    friend class XQData;
    friend class XQFile;
    friend class XQJsonFile;
    friend class XQFFile;
};

class XQData
{
public:
    XQHeader header;
    XQNode *pRoot;
    XQNode *pCurNode;

    XQData()
    {
        pRoot = nullptr;
        pCurNode = nullptr;
    }

    virtual ~XQData()
    {
        deleteXQNode(pRoot);
    }

private:
    void deleteXQNode(XQNode *node)
    {
        if (node == nullptr)
            return;
        if (node->pRight)
            deleteXQNode(node->pRight);
        if (node->pFirstChild)
            deleteXQNode(node->pFirstChild);
        delete node;
    }
};

class XQFile
{
public:
    virtual void load(std::string name) = 0;
    virtual XQHeader getHeader() { return _data.header; };
    virtual std::string getInitFen() { return _data.header.fen; };
    virtual void reset()
    {
        _data.pCurNode = nullptr;
    }
    virtual XQNode *nextStep()
    {
        if (_data.pCurNode == nullptr) {
            _data.pCurNode = _data.pRoot;
            return _data.pCurNode;
        }
        XQNode *node = _data.pCurNode->pCurChild;
        if (node == nullptr)
            return nullptr;
        _data.pCurNode = node;
        return node;
    };
    virtual XQNode *prevStep()
    {
        if (_data.pCurNode == nullptr)
            return nullptr;
        XQNode *node = _data.pCurNode->pParent;
        if (node == nullptr)
            return nullptr;
        _data.pCurNode = node;
        return node;
    };
    virtual std::vector<XQNode*> getNextAlts()
    {
        std::vector<XQNode*> v;
        if (_data.pCurNode == nullptr) {
            return v;
        }
        XQNode *node = _data.pCurNode->pFirstChild;
        for (; node != nullptr; node = node->pRight)
        {
            v.push_back(node);
        }
        return v;
    }
    virtual void selectNextAlt(XQNode *n)
    {
        XQNode *node = _data.pCurNode->pFirstChild;
        for (; node != nullptr; node = node->pRight)
        {
            if (node == n) {
                _data.pCurNode->pCurChild = node;
                break;
            }
        }
    }

    virtual ~XQFile() {}

protected:
    XQData _data;
};

#endif
