#include "StdAfx.h"
#include "XMLParse.h"

/************************************************************************/
/*description:  Constructor
/*  arguments:  void
/*    return:   none                                                   
/************************************************************************/
CXMLParse::CXMLParse()
{
	initParse();
}
/************************************************************************/
/*description:  init the Dynamic Link Library and create the 
/*              IXMLDOMDocumentPtr object.
/*  arguments:  void
/*    return:   void                                                   
/************************************************************************/
void CXMLParse::initParse()
{
	::CoInitialize(NULL);   
	HRESULT hr = m_pDoc.CreateInstance(__uuidof(DOMDocument30));
	if (!SUCCEEDED(hr))
		return;
}
/************************************************************************/
/*description:  init the Dynamic Link Library and create the 
/*              IXMLDOMDocumentPtr object.
/*  arguments:        path:  the File path or the string used to parse. 
/*              dataSource:  the data source category---File or String
/*    return:         TRUE:  parse successfully.          
/*                   FALSE:  Error.                                                
/************************************************************************/
BOOL CXMLParse::startParse(CString path,int dataSource)
{
    char buffer[1024];
    BOOL isSuccess = FALSE;
	if (path.GetLength()<=0)
			return FALSE;
	switch (dataSource)
	{
	case FILEPATH:
        strncpy(buffer,(LPCTSTR)path,sizeof(buffer));
		isSuccess = m_pDoc->load((_bstr_t)(char*)buffer);
		return (isSuccess == -1);
	case STRING:
		strncpy(buffer,(LPCTSTR)path,sizeof(buffer));
		isSuccess = m_pDoc->loadXML(buffer);
		return (isSuccess == -1);
	}

	return FALSE;
}
/************************************************************************/
/*description:  get the doucument root.
/*             
/*  arguments:               root:  the root string. 
/*               
/*    return:   IXMLDOMElementPtr: the root pointer.                                             
/************************************************************************/
IXMLDOMElementPtr CXMLParse::getDocumentRoot()
{
	if (m_pDoc)
	{
		m_pRoot = m_pDoc->GetdocumentElement();
        return m_pRoot;
	}

	return NULL;
}
/************************************************************************/
/*description:  get the element name.
/*             
/*  arguments:  element: get the element's node name. 
/*                  str: store the element's node name.
/*    return:   void                                             
/************************************************************************/
void CXMLParse::getElementName(IXMLDOMNodePtr element,CString &str)
{
	if (element)
	{
	  str = (char*)(_bstr_t)element->GetnodeName();
	}
}
/************************************************************************/
/*description:  get the element attributes.
/*             
/*  arguments:  element: get the element's node attributes. 
/*                 attr: store the element's node attributes.
/*    return:       int: the number of the element's attribute.                                             
/************************************************************************/
int CXMLParse::getElementAttr(IXMLDOMElementPtr element,CMapStringToString &attr)
{
    IXMLDOMNamedNodeMapPtr pAttr    = NULL;	
	IXMLDOMNodePtr       pAttrItem  = NULL;
	
	element->get_attributes(&pAttr); 
    int length  = pAttr->Getlength();
	
	for (int i=0;i<length;++i)
	{
		pAttr->get_item(i, &pAttrItem);
	    CString	strAttrName = (char*)(_bstr_t)pAttrItem->nodeName;
        CString StrAttrValue = (char*)(_bstr_t)pAttrItem->text;

		attr.SetAt(strAttrName,StrAttrValue);
	}
	return length;  
}
/************************************************************************/
/*description:  get element's text..
/*             
/*  arguments: element: the node to be find.
/*                 str: store the element's text.
/*    return:   void                                        
/************************************************************************/
void CXMLParse::getElementText(IXMLDOMNodePtr element,CString &str)
{
	str = (char*)(_bstr_t)element->text;
}
/************************************************************************/
/*description: find and return the element's children.
/*             
/*  arguments: element: the element to be find.
/*            children: store the element's children.         
/*    return:      int: the number of element's childern.                                       
/************************************************************************/
int CXMLParse::getChildren(IXMLDOMElementPtr element,IXMLDOMNodeListPtr &children)
{
	children   = element->GetchildNodes();
 	int length = children->Getlength();

	return length;
}
/************************************************************************/
/*description: Judge whether the node has children.
/*             
/*  arguments: element:the node to be judged.
/*               
/*    return:     TRUE: YES
/*               FALSE:  NO                                        
/************************************************************************/
BOOL CXMLParse::hasChildren(IXMLDOMNodePtr element)
{
    int hasChildren = element->hasChildNodes();
	if (0 == hasChildren)
		return FALSE;
	return TRUE;
}
/************************************************************************/
/*description: Judge whether the node has attributes.
/*             
/*  arguments: element:the node to be judged.
/*               
/*    return:     TRUE: YES
/*               FALSE:  NO                                        
/************************************************************************/
BOOL CXMLParse::hasAttribute(IXMLDOMNodePtr element)
{
	IXMLDOMNamedNodeMapPtr pAttr    = NULL;	

	element->get_attributes(&pAttr); 
    return (pAttr->Getlength()>0);
}
/************************************************************************/
/*description: Judge whether the node is a text node.
/*             
/*  arguments: element:the node to be judged.
/*               
/*    return:     TRUE: YES
/*               FALSE:  NO                                        
/************************************************************************/
BOOL CXMLParse::isNodeText(IXMLDOMNodePtr element)
{
	DOMNodeType   nodeType;
	element->get_nodeType(&nodeType);
     
	return (nodeType == NODE_TEXT);

}
/************************************************************************/
/*description: Judge whether the node is a element node.
/*             
/*  arguments: element:the node to be judged.
/*               
/*    return:     TRUE: YES
/*               FALSE:  NO                                        
/************************************************************************/
BOOL CXMLParse::iSNodeElement(IXMLDOMNodePtr element)
{
	DOMNodeType nodeType;
	element->get_nodeType(&nodeType);
	
	return (nodeType == NODE_ELEMENT);
}
/************************************************************************/
/*description: destructor and unload the Dynamic Link Library.
/*             
/*  arguments: void
/*               
/*    return:  none                                          
/************************************************************************/
CXMLParse::~CXMLParse()
{
	::CoUninitialize(); 
}

