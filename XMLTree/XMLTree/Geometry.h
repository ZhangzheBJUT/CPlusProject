/**************************************************************
*
*FileName: Geometry.h
*Creator : Zhangzhe
*   Date : 2012-11-20
*Comment : ͼԪ���࣬���幫���ӿڡ�
*
/**************************************************************/

#ifndef _CGEOMETRY_H
#define _CGEOMETRY_H

#include "StdAfx.h"

class CGeometry : public CObject                   
{ 
public:
	virtual BOOL clickTest(CPoint point)=0;         //������
	virtual void Draw(CDC *pdc=NULL) = 0;           //��ͼ
    virtual void offset(int cx=0,int cy=0)=0;       //λ��
};

#endif//endif