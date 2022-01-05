## this is post treatment file for pap executions
# we first post-treat times past in non-compute sync & async transferts

import pylab as pl

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
	size.append(float(line[0]))
	sync_send.append(float(line[4]))
	sync_recv.append(float(line[5]))
	async_send.append(float(line[6]))
	async_recv.append(float(line[7]))
	full_async_send.append(float(line[8]))
	full_async_recv.append(float(line[9]))
f.close()
#print(size)
#print(sync_send)
pl.loglog(size, sync_send, '+')
pl.show()
pl.savefig("test.png")








