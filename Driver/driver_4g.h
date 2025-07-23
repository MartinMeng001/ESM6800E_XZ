#ifndef DRIVER_4G_H
#define DRIVER_4G_H
/**
 *	启用拨号，断线自动重连
 *
 *	返回值说明：
 *	 0 表示运行正常，因为程序执行关闭，所以退出返回。
 *	-1 表示模块无响应，建议检查硬件连接和驱动是否正常加载。
 *	-2 表示读取信号差，建议检查天线和SIM卡是否连接正确。
 *
 */
int pppconnt_start();

/**
 *	查询连接状态
 *
 *	返回值说明：
 *	 0 表示运行连接正常。
 *	-1 表示连接断开。
 *
 */
int pppconnt_check();

/**
 *	关闭连接
 *
 *	关闭库函数提供的功能，并关闭已建立的连接。
 *
 */
void pppconnt_stop();
#endif // DRIVER_4G_H
