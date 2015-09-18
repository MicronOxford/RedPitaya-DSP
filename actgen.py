from __future__ import print_function

dt = 1e4#ns
f = open('acttable', 'w')

n = 100000
t = 0
for l in range(n):
    print('{} {} {} {} {}'.format( t, 0, 0, 0, l*4000./n ), file=f)
    t += dt
    print('{} {} {} {} {}'.format( t, 0x000000FF, 0x000000FF, 4000, l*4000./n ), file=f)
    t += dt

#print('{} {} 13 0'.format( repr(int(t/1e9)), repr(int(t%1e9)) ), file=f, end='')
f.close()
!scp acttable root@192.168.1.100:/tmp/
!rm -f acttable

dt = 1e3#ns
f = open('acttable', 'w')

n = 1000000
t = 0
for l in range(n):
    print('{} {} {} {}'.format( t, 255 if l%2 else 0, 0, 4000*math.sin(l/100.) ), file=f)
    t += dt

#print('{} {} 13 0'.format( repr(int(t/1e9)), repr(int(t%1e9)) ), file=f, end='')
f.close()
!scp acttable root@192.168.1.100:/tmp/
!rm -f acttable


f = open('acttable', 'w')
print('{} {} 0'.format(0, 0xFFFFFFFF), file=f)
print('{} {} 1'.format(1e9, 0), file=f)
print('{} {} 0'.format(2e9, 0xFFFFFFFF), file=f)
f.close()

!scp acttable root@192.168.1.106:/tmp/
!rm -f acttable

'''
    #print('{} 13 0'.format(repr(int(t))), file=f)
    #t += dt*3
    #print('{} 13 1'.format(repr(int(t))), file=f)
    #t += dt*3
'''
