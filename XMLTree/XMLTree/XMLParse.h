/**************************************************************
*
*FileName: XMLParse.h
*Creator : Zhangzhe
*   Date : 2012-11-19
*Comment : CXMLParese 封装了对XML文件的解析操作，包括取元素
*          结点的名称、该结点所拥有的所有属性信息和该结点的
*          文本内容等。
*
/**************************************************************/

#ifndef _XMLPARSE_H
#define _XMLPARSE_H

#import <msxml3.dll> 
using namespace MSXML2;

#define  FILEPATH 1
#define  STRING   2
class CXMLParse
{
private:	
	IXMLDOMDocumentPtr     m_pDoc;                 //文档对象
	IXMLDOMElementPtr      m_pRoot;                //文档根
    void initParse();
public:
	CXMLParse();
	BOOL startParse(CString path,int dataSource);  //1:Loads an XML document from the specified location.
	                                               //2:Loads an XML document using the supplied string.
	IXMLDOMElementPtr getDocumentRoot();
	void getElementName(IXMLDOMNodePtr element,CString &str);
	int  getElementAttr(IXMLDOMElementPtr element,CMapStringToString &attr);
	void getElementText(IXMLDOMNodePtr element,CString &str);
	BOOL hasChildren(IXMLDOMNodePtr element); 
	BOOL hasAttribute(IXMLDOMNodePtr element);
	BOOL isNodeText(IXMLDOMNodePtr element);
	BOOL iSNodeElement(IXMLDOMNodePtr element);
	int  getChildren(IXMLDOMElementPtr  element,IXMLDOMNodeListPtr &children);
	
	virtual ~CXMLParse();
};
#endif  //endif