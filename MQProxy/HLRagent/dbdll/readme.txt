1�����ݺϲ�oss�汾ʱ���������⣬�޸�dbproj��������޸İ汾  2011.12.26
2���޸Ĳ�������ʱ����������������ʱ���������������         2011.12.29
3����Բ�������ʱ���������ȳ���16�ֽ�ʱ��������������⣬���setDbChar���� 
    dbproj_v3     2011.12.29
4�����⣺��ѯ����ʱ�������д����ȳ���16�ֽ�ʱ��getDBstring�޷����������������������
   ��������XS8* GetDbChar 
   �汾��dbproj_v4  2011.12.30
5����xostype.h�е�xu64��xs64��Ϊlong long
  
6���޸�XS8* GetDbChar(nCol) Ϊ void GetDbChar(nCol,XS8*)
   �汾��dbproj_v5  2011.12.30
7����xostype.h������#define XNOFOUNDDATA		(-2)
   ��xostype.h�޸�����Ϊpublictype.h
   �汾:dbproj_v6  2011.12.31
8����createstatement����Ӷ�sqlΪNULLʱ����ж�
   �޸�GetDbDouble()����ֵ�޸�Ϊdouble

   �޸�SetDbDouble()�������ֵ�޸�Ϊdouble
 
   ���	void SetDbLong(XU32 nCol, long value);
	void SetDbULong(XU32 nCol, unsigned long value);
	long GetDbLong(XU32 nCol);
	unsigned long GetDbULong(XU32 nCol);
	XU32 GetDbUInt(XU32 nCol);
	void SetDbUInt(XU32 nCol, XU32 value);
   �汾��dbproj_v7 2012.1.5
9����Ӵ����봦����GetErrorCode();
   �޸�publictype.h�ж���Ĵ����� 
   ��publictype.h��ɾ��occi�Ŀ�����
   �޸����Ӻ� ��1��ʼ������ԭ��Ϊ��0��ʼ������
    �汾��dbproj_v8 2012.1.11
10���޸�ExecuteQurey()�ķ���ֵΪ������
    �汾��dbproj_v9 2012.03.15
11����serviceOracle�� ��Ӷ�map �� connid��������
    2012.03.28