// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
//��ı�׼�������� DLL �е������ļ��������������϶���� DBPROJ_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
//�κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ 
// DBPROJ_API ������Ϊ�ǴӴ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�

#if (defined(WIN32))
#ifdef DBPROJ_EXPORTS
#define DBPROJ_API __declspec(dllexport)
#else
#define DBPROJ_API __declspec(dllimport)
	#endif
#endif


#ifdef LINUX	
	#define DBPROJ_API 
#endif

// �����Ǵ� dbproj.dll ������
/*class DBPROJ_API Cdbproj {
public:
	Cdbproj(XVOID);
	// TODO: �ڴ�������ķ�����
};

extern DBPROJ_API XS32 ndbproj;

DBPROJ_API XS32 fndbproj(XVOID);*/
