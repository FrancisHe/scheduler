A Memory-Managed Round Robin Scheduling Simulator(带内存管理功能的进程调度模拟器)
描述：一个带内存管理功能的进程调度模拟程序，基于 Linux 平台，父进程模拟 scheduler, 父进程 fork 的子进程为用户进程，调度通过 scheduler 向用户进程发送 SIGINT, SIGCONT, SIGSTP等信号进行模拟，调度策略采用时间片轮询 的 FIFO 算法，内存管理采用 Next-Fit、Best-Fit 等算法。
