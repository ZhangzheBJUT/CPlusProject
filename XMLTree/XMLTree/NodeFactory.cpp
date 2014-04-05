#include "StdAfx.h"
#include "NodeFactory.h"
#include "UniversalNode.h"
#include "ElementNode.h"
#include "AttrNode.h"
#include "TextNode.h"

/************************************************************************/
/*description:  Constructor
/*  arguments:  void
/*    return:   none                                                   
/************************************************************************/
CNodeFactory::CNodeFactory()
{
}
/************************************************************************/
/*description:  produce a node for displaying node's name.
/*  arguments:  content: the text to be display on the node.  
/*                    x: the x coordinates   
/*                    y: the y coordinates
/*    return:   CUiversalNode*: the produced node.                                                   
/************************************************************************/
CUniversalNode* CNodeFactory::getElementNode(CString content,int x,int y)
{
	return new CElementNode(content,x,y);
}
/************************************************************************/
/*description:  produce a node for displaying node's attributes.
/*  arguments:  content: the text to be display on the node.  
/*                    x: the x coordinates   
/*                    y: the y coordinates
/*    return:   CUiversalNode*: the produced node.                                                   
/************************************************************************/
CUniversalNode* CNodeFactory::getAttrNode(CString content,int x,int y)
{
	return new CAttrNode(content,x,y);
}
/************************************************************************/
/*description:  produce a node for displaying text.
/*  arguments:  content: the text to be display on the node.  
/*                    x: the x coordinates   
/*                    y: the y coordinates
/*    return:   CUiversalNode*: the produced node.                                                   
/************************************************************************/
CUniversalNode* CNodeFactory::getTextNode(CString content,int x,int y)
{
	return new CTextNode(content,x,y);
}
CNodeFactory::~CNodeFactory()
{
}
