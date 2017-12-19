#ifndef _XOS_MON_H_
#define _XOS_MON_H_
#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */

#ifdef XOS_LINUX

#define GET_CPU_RATE "#/bin/sh\r\n\r\ncpulog_1=$(cat /proc/stat | grep 'cpu' | awk '{print $2\" \"$3\" \"$4\" \"$5\" \
\"$6\" \"$7\" \"$8}')\r\nsysidle_1=$(echo $cpulog_1 | awk '{print $4}')\r\ntotal_1=$(echo $cpulog_1 | awk '{print \
$1+$2+$3+$4+$5+$6+$7}')\r\n\r\nsleep 4\r\n\r\ncpulog_2=$(cat /proc/stat | grep 'cpu' | awk '{print $2\" \"$3\" \"$4\" \"$5\" \"$6\" \"$7\" \
\"$8}')\r\nsysidle_2=$(echo $cpulog_2 | awk '{print $4}')\r\ntotal_2=$(echo $cpulog_2 | awk '{print \
$1+$2+$3+$4+$5+$6+$7}')\r\n\r\nsys_idle=`expr $sysidle_2 - $sysidle_1`\r\ntotal=`expr $total_2 - $total_1`\r\nsys_usage=`expr $sys_idle/$total*100 |bc -l`\r\nsys_rate=`expr \
100-$sys_usage |bc -l`\r\ndisp_sysrate=`expr \"scale=3;$sys_rate/1\" | bc`\r\n\r\necho $disp_sysrate%"

XS32 XOS_FIDMON(HANDLE hDir,XS32 argc, XS8** argv);

XS8  MON_Init(XVOID*p1,XVOID*p2);

XS8  MON_msgProc(XVOID* pMsgP,XVOID*sb );

XS8  MON_timerProc( t_BACKPARA* pParam);

#endif

#ifdef __cplusplus
}
#endif /* _ _cplusplus */

#endif /* _CLISHELL_H_ */



