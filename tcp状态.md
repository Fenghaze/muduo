# TCP状态影响服务器的情况



## （1）signal(SIGPIPE, SIG_IGN)

> 忽略SIGPIPE信号，以防止服务器程序关闭——《UNP》12章

SIGPIPE产生原因：客户端关闭（close）套接字，如果此时服务器调用了一次write，那么服务器会收到一个RST segment；如果服务器再次调用write，此时就会产生一个SIGPIPE信号，服务器程序就会shutdown

为了防止出现这种情况，就需要忽略SIGPIPE信号



## （2）TIME_WAIT状态过多

TIME_WAIT产生原因：TCP两端，谁先关闭连接谁就先进入TIME_WAIT状态

在实际应用中，定义协议时，应当让客户端主动关闭连接；而对于不活跃的连接，服务端来主动发起断开连接

