from __future__ import print_function

dt = 70000#ns
f = open('acttable', 'w')

t = 0
for l in range(10000):
    print('{} 13 0'.format(t), file=f)
    t += dt
    print('{} 13 1'.format(t), file=f)
    t += dt

!scp acttable root@192.168.1.100:/tmp/
!rm -f acttable
