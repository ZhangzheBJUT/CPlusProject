/**************************************************************
*
*FileName: Geometry.h
*Creator : Zhangzhe
*   Date : 2012-11-20
*Comment : 图元基类，定义公共接口。
*
/**************************************************************/

#ifndef _CGEOMETRY_H
#define _CGEOMETRY_H

#include "StdAfx.h"

class CGeometry : public CObject                   
{ 
public:
	virtual BOOL clickTest(CPoint point)=0;         //点击检测
	virtual void Draw(CDC *pdc=NULL) = 0;           //绘图
    virtual void offset(int cx=0,int cy=0)=0;       //位移
};

#endif//endif