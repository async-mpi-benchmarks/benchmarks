## this is post treatment file for pap executions
# we first post-treat times past in non-compute sync & async transferts

import pylab as pl
type = 8 # 8 bytes per 64bits number
size = []
sync_send = []
sync_recv = []
async_send = []
async_recv = []
full_async_send = []
full_async_recv = []
# open file 
f = open("time_pap.txt", "r")
data = f.readlines(); 
for i in data : 
	line = i.rsplit(" ")
	size.append(float(line[0])*type)
	sync_send.append(float(line[4]))
	sync_recv.append(float(line[5]))
	async_send.append(float(line[6]))
	async_recv.append(float(line[7]))
	full_async_send.append(float(line[8]))
	full_async_recv.append(float(line[9]))
f.close()
#print(size)
#print(sync_send)
pl.grid()
pl.ylabel("tics")
pl.xlabel("bytes sended")
pl.loglog(size, sync_send,  'r+', label="synchrone")
pl.loglog(size, async_send,  'b+', label="asynchrone")
pl.loglog(size, full_async_send,  'g+', label="full asynchrone")
pl.legend()
pl.savefig("pap_send.png")
pl.show()
pl.close()

pl.grid()
pl.ylabel("tics")
pl.xlabel("bytes sended")
pl.loglog(size, sync_recv,  'r+', label="synchrone")
pl.loglog(size, async_recv,  'b+', label="asynchrone")
pl.loglog(size, full_async_recv,  'g+', label="full asynchrone")
pl.legend()
pl.savefig("pap_recv.png")
pl.show()
pl.close()








